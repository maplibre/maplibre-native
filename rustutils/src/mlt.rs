//! MapLibre Tile (MLT) decoding for `src/mln/tile/vector_mlt_tile_data.rs.cpp`.
//!
//! A tile is decoded in one go into flat per-layer buffers, which C++ reads through
//! borrowed slices without copying them again.

use std::collections::HashMap;
use std::ops::Range;

use mlt_core::geo_types::{Coord, Geometry, LineString, Polygon};
use mlt_core::{
    Decoder, GeometryValues, Layer, LendingIterator, ParsedLayer01, Parser, PropValueRef,
};

#[cxx::bridge(namespace = "rustutils")]
mod ffi {
    /// Same values as `mln::FeatureType`.
    #[derive(Clone, Copy, Debug, PartialEq, Eq)]
    enum MltFeatureType {
        Unknown = 0,
        Point = 1,
        LineString = 2,
        Polygon = 3,
    }

    #[derive(Clone, Copy, Debug, PartialEq, Eq)]
    enum MltValueType {
        /// The feature has no value for this property.
        Null,
        Bool,
        Int,
        UInt,
        Double,
        String,
    }

    /// A property value. Only the field that matches `kind` is set.
    #[derive(Clone, Debug, PartialEq)]
    struct MltValue {
        kind: MltValueType,
        bool_value: bool,
        int_value: i64,
        uint_value: u64,
        double_value: f64,
        string_value: String,
    }

    #[derive(Clone, Copy, Debug, PartialEq, Eq)]
    struct MltFeatureId {
        has_id: bool,
        value: u64,
    }

    #[derive(Clone, Copy, Debug, PartialEq, Eq)]
    struct MltCoordinate {
        x: i32,
        y: i32,
    }

    extern "Rust" {
        type MltTile;

        fn decode_mlt_tile(data: &[u8]) -> Result<Box<MltTile>>;

        fn layer_count(&self) -> usize;
        /// Index of the first layer called `name`, or -1.
        fn layer_index(&self, name: &[u8]) -> i64;
        fn layer_name(&self, layer: usize) -> &str;
        fn layer_extent(&self, layer: usize) -> u32;
        fn feature_count(&self, layer: usize) -> usize;
        fn property_count(&self, layer: usize) -> usize;
        fn property_name(&self, layer: usize, property: usize) -> &str;
        /// Index of the first property called `name`, or -1.
        fn property_index(&self, layer: usize, name: &[u8]) -> i64;
        /// The coordinates of all features in the layer, see `feature_part_offsets`.
        fn layer_coordinates(&self, layer: usize) -> &[MltCoordinate];

        fn feature_id(&self, layer: usize, feature: usize) -> MltFeatureId;
        fn feature_type(&self, layer: usize, feature: usize) -> MltFeatureType;
        /// One value per property of the layer, in property order.
        fn feature_properties(&self, layer: usize, feature: usize) -> &[MltValue];
        /// Part `i` of the feature spans `layer_coordinates()[offsets[i]..offsets[i + 1]]`.
        /// Polygon rings are closed.
        fn feature_part_offsets(&self, layer: usize, feature: usize) -> &[u32];
        /// Pre-tessellated triangles of a polygon feature, as indices into the
        /// concatenation of its rings. Empty if the tile has none.
        fn feature_triangles(&self, layer: usize, feature: usize) -> &[u32];
    }
}

/// mlt-core limits parsing and decoding to 20 MiB by default, but decoded tiles take roughly 5-20
/// times their encoded size, so dense tiles of a few MB would fail to load. Keep a limit against
/// malicious tiles, but a higher one.
const MEMORY_BUDGET: u32 = 128 * 1024 * 1024;

pub struct MltTile {
    layers: Vec<MltLayer>,
}

#[derive(Default)]
struct MltLayer {
    name: String,
    extent: u32,
    property_names: Vec<String>,
    property_indices: HashMap<String, usize>,
    /// `property_names.len()` values per feature.
    values: Vec<ffi::MltValue>,
    /// Part boundaries for all features. Starts with 0, and each part ends where the next starts.
    part_offsets: Vec<u32>,
    coordinates: Vec<ffi::MltCoordinate>,
    triangles: Vec<u32>,
    features: Vec<MltFeature>,
}

struct MltFeature {
    id: Option<u64>,
    feature_type: ffi::MltFeatureType,
    /// Range of `MltLayer::part_offsets`, including the end of the last part.
    part_offsets: Range<usize>,
    /// Range of `MltLayer::triangles`.
    triangles: Range<usize>,
}

pub fn decode_mlt_tile(data: &[u8]) -> Result<Box<MltTile>, String> {
    let layers = Parser::with_max_size(MEMORY_BUDGET)
        .parse_layers(data)
        .map_err(|err| err.to_string())?;
    let mut decoder = Decoder::with_max_size(MEMORY_BUDGET);
    let mut result = Vec::with_capacity(layers.len());
    for layer in layers {
        // Layers with an unknown tag are skipped, as the MLT spec requires.
        if let Layer::Tag01(layer) = layer
            .decode_all(&mut decoder)
            .map_err(|err| err.to_string())?
        {
            result.push(MltLayer::decode(&layer)?);
        }
    }
    Ok(Box::new(MltTile { layers: result }))
}

impl MltLayer {
    fn decode(layer: &ParsedLayer01<'_>) -> Result<Self, String> {
        let property_names: Vec<String> = layer
            .iterate_prop_names()
            .map(|name| name.to_string())
            .collect();
        let mut property_indices = HashMap::with_capacity(property_names.len());
        for (index, name) in property_names.iter().enumerate() {
            property_indices.entry(name.clone()).or_insert(index);
        }

        let feature_count = layer.feature_count();
        let mut result = Self {
            name: layer.name().to_string(),
            extent: layer.extent().get(),
            values: Vec::with_capacity(feature_count.saturating_mul(property_names.len())),
            property_names,
            property_indices,
            part_offsets: vec![0],
            features: Vec::with_capacity(feature_count),
            ..Self::default()
        };

        let mut triangles = Triangles::new(layer.geometry_values());
        let mut scratch = Vec::new();
        let mut features = layer.iter_features();
        while let Some(feature) = features.next() {
            let feature = feature.map_err(|err| err.to_string())?;
            let geometry = feature.geometry();

            // One value per property name, see `ParsedLayer01::iterate_prop_names`.
            result
                .values
                .extend(feature.iter_all_properties().map(ffi::MltValue::from));

            let part_offsets_start = result.part_offsets.len() - 1;
            let feature_type = result.push_geometry(geometry)?;
            let part_offsets = part_offsets_start..result.part_offsets.len();

            let triangles_start = result.triangles.len();
            if feature_type == ffi::MltFeatureType::Polygon {
                if let Some(indices) = triangles.next_polygon() {
                    result.push_triangles(geometry, indices, &mut scratch);
                }
            }

            result.features.push(MltFeature {
                id: feature.id(),
                feature_type,
                part_offsets,
                triangles: triangles_start..result.triangles.len(),
            });
        }

        Ok(result)
    }

    fn push_geometry(&mut self, geometry: &Geometry<i32>) -> Result<ffi::MltFeatureType, String> {
        use ffi::MltFeatureType as Type;
        Ok(match geometry {
            Geometry::Point(point) => {
                self.push_part([point.0])?;
                Type::Point
            }
            // Like MVT, every point of a multi-point is its own part.
            Geometry::MultiPoint(points) => {
                for point in points {
                    self.push_part([point.0])?;
                }
                Type::Point
            }
            Geometry::LineString(line) => {
                self.push_part(line.0.iter().copied())?;
                Type::LineString
            }
            Geometry::MultiLineString(lines) => {
                for line in lines {
                    self.push_part(line.0.iter().copied())?;
                }
                Type::LineString
            }
            Geometry::Polygon(_) | Geometry::MultiPolygon(_) => {
                for ring in polygon_rings(geometry) {
                    self.push_part(ring.0.iter().copied())?;
                }
                Type::Polygon
            }
            // mlt-core does not decode into any other kind of geometry.
            _ => Type::Unknown,
        })
    }

    fn push_part(
        &mut self,
        coordinates: impl IntoIterator<Item = Coord<i32>>,
    ) -> Result<(), String> {
        self.coordinates.extend(
            coordinates
                .into_iter()
                .map(|coordinate| ffi::MltCoordinate {
                    x: coordinate.x,
                    y: coordinate.y,
                }),
        );
        let end = u32::try_from(self.coordinates.len())
            .map_err(|_| format!("MLT layer '{}' has too many coordinates", self.name))?;
        self.part_offsets.push(end);
        Ok(())
    }

    /// MLT stores rings without their closing vertex and indexes triangles into that
    /// layout, but the decoded rings repeat their first vertex at the end, so the
    /// indices have to be shifted. If an index is out of range, the triangles are
    /// dropped and the renderer tessellates the polygon itself.
    fn push_triangles(
        &mut self,
        geometry: &Geometry<i32>,
        indices: &[u32],
        scratch: &mut Vec<u32>,
    ) {
        // `scratch[i]` is the closed-ring index of vertex `i` of the unclosed rings.
        scratch.clear();
        let mut closed_index = 0;
        for ring in polygon_rings(geometry) {
            // Fits in u32 because `push_part` checked the coordinate count.
            let len = ring.0.len() as u32;
            scratch.extend(closed_index..closed_index + len - 1);
            closed_index += len;
        }

        let start = self.triangles.len();
        for &index in indices {
            let Some(&index) = scratch.get(index as usize) else {
                self.triangles.truncate(start);
                return;
            };
            self.triangles.push(index);
        }
    }
}

/// The non-empty rings of a polygon or multi-polygon, in MLT order.
fn polygon_rings(geometry: &Geometry<i32>) -> impl Iterator<Item = &LineString<i32>> {
    let polygons: &[Polygon<i32>] = match geometry {
        Geometry::Polygon(polygon) => std::slice::from_ref(polygon),
        Geometry::MultiPolygon(polygons) => &polygons.0,
        _ => &[],
    };
    polygons
        .iter()
        .flat_map(|polygon| std::iter::once(polygon.exterior()).chain(polygon.interiors()))
        .filter(|ring| !ring.0.is_empty())
}

/// Hands out the pre-tessellated triangles of a layer's polygon features in order.
struct Triangles<'a> {
    counts: std::slice::Iter<'a, u32>,
    indices: &'a [u32],
}

impl<'a> Triangles<'a> {
    fn new(geometry: &'a GeometryValues) -> Self {
        Self {
            counts: geometry.triangles().unwrap_or_default().iter(),
            indices: geometry.index_buffer().unwrap_or_default(),
        }
    }

    fn next_polygon(&mut self) -> Option<&'a [u32]> {
        let len = (*self.counts.next()? as usize).checked_mul(3)?;
        if len == 0 || len > self.indices.len() {
            return None;
        }
        let (indices, rest) = self.indices.split_at(len);
        self.indices = rest;
        Some(indices)
    }
}

impl Default for ffi::MltValue {
    fn default() -> Self {
        Self {
            kind: ffi::MltValueType::Null,
            bool_value: false,
            int_value: 0,
            uint_value: 0,
            double_value: 0.0,
            string_value: String::new(),
        }
    }
}

impl From<Option<PropValueRef<'_>>> for ffi::MltValue {
    fn from(value: Option<PropValueRef<'_>>) -> Self {
        use ffi::MltValueType as Kind;
        let int = |int_value: i64| Self {
            kind: Kind::Int,
            int_value,
            ..Self::default()
        };
        let uint = |uint_value: u64| Self {
            kind: Kind::UInt,
            uint_value,
            ..Self::default()
        };
        let double = |double_value: f64| Self {
            kind: Kind::Double,
            double_value,
            ..Self::default()
        };
        match value {
            None => Self::default(),
            Some(PropValueRef::Bool(bool_value)) => Self {
                kind: Kind::Bool,
                bool_value,
                ..Self::default()
            },
            Some(PropValueRef::I8(value)) => int(value.into()),
            Some(PropValueRef::I32(value)) => int(value.into()),
            Some(PropValueRef::I64(value)) => int(value),
            Some(PropValueRef::U8(value)) => uint(value.into()),
            Some(PropValueRef::U32(value)) => uint(value.into()),
            Some(PropValueRef::U64(value)) => uint(value),
            Some(PropValueRef::F32(value)) => double(value.into()),
            Some(PropValueRef::F64(value)) => double(value),
            Some(PropValueRef::Str(value)) => Self {
                kind: Kind::String,
                string_value: value.to_string(),
                ..Self::default()
            },
        }
    }
}

fn index_or_negative(index: Option<usize>) -> i64 {
    index
        .and_then(|index| i64::try_from(index).ok())
        .unwrap_or(-1)
}

impl MltTile {
    fn layer(&self, layer: usize) -> Option<&MltLayer> {
        self.layers.get(layer)
    }

    fn feature(&self, layer: usize, feature: usize) -> Option<(&MltLayer, &MltFeature)> {
        let layer = self.layer(layer)?;
        Some((layer, layer.features.get(feature)?))
    }

    fn layer_count(&self) -> usize {
        self.layers.len()
    }

    fn layer_index(&self, name: &[u8]) -> i64 {
        index_or_negative(
            self.layers
                .iter()
                .position(|layer| layer.name.as_bytes() == name),
        )
    }

    fn layer_name(&self, layer: usize) -> &str {
        self.layer(layer).map_or("", |layer| &layer.name)
    }

    fn layer_extent(&self, layer: usize) -> u32 {
        self.layer(layer).map_or(0, |layer| layer.extent)
    }

    fn feature_count(&self, layer: usize) -> usize {
        self.layer(layer).map_or(0, |layer| layer.features.len())
    }

    fn property_count(&self, layer: usize) -> usize {
        self.layer(layer)
            .map_or(0, |layer| layer.property_names.len())
    }

    fn property_name(&self, layer: usize, property: usize) -> &str {
        self.layer(layer)
            .and_then(|layer| layer.property_names.get(property))
            .map_or("", String::as_str)
    }

    fn property_index(&self, layer: usize, name: &[u8]) -> i64 {
        let Some(layer) = self.layer(layer) else {
            return -1;
        };
        // Property names are strings, so a key that is not UTF-8 cannot match.
        let index = std::str::from_utf8(name)
            .ok()
            .and_then(|name| layer.property_indices.get(name).copied());
        index_or_negative(index)
    }

    fn layer_coordinates(&self, layer: usize) -> &[ffi::MltCoordinate] {
        self.layer(layer).map_or(&[], |layer| &layer.coordinates)
    }

    fn feature_id(&self, layer: usize, feature: usize) -> ffi::MltFeatureId {
        match self
            .feature(layer, feature)
            .and_then(|(_, feature)| feature.id)
        {
            Some(value) => ffi::MltFeatureId {
                has_id: true,
                value,
            },
            None => ffi::MltFeatureId {
                has_id: false,
                value: 0,
            },
        }
    }

    fn feature_type(&self, layer: usize, feature: usize) -> ffi::MltFeatureType {
        self.feature(layer, feature)
            .map_or(ffi::MltFeatureType::Unknown, |(_, feature)| {
                feature.feature_type
            })
    }

    fn feature_properties(&self, layer: usize, feature: usize) -> &[ffi::MltValue] {
        let Some(layer) = self.layer(layer) else {
            return &[];
        };
        let count = layer.property_names.len();
        feature
            .checked_mul(count)
            .and_then(|start| layer.values.get(start..start.checked_add(count)?))
            .unwrap_or_default()
    }

    fn feature_part_offsets(&self, layer: usize, feature: usize) -> &[u32] {
        self.feature(layer, feature)
            .and_then(|(layer, feature)| layer.part_offsets.get(feature.part_offsets.clone()))
            .unwrap_or_default()
    }

    fn feature_triangles(&self, layer: usize, feature: usize) -> &[u32] {
        self.feature(layer, feature)
            .and_then(|(layer, feature)| layer.triangles.get(feature.triangles.clone()))
            .unwrap_or_default()
    }
}

#[cfg(test)]
mod tests {
    use mlt_core::encoder::EncoderConfig;
    use mlt_core::geo_types::{line_string, point, polygon, MultiPoint};
    use mlt_core::{PropKind, PropValue, TileLayer};

    use super::*;

    fn encode(layer: TileLayer) -> Vec<u8> {
        layer
            .encode(EncoderConfig::default().with_tessellation(true))
            .unwrap()
    }

    fn coordinates(tile: &MltTile, feature: usize) -> Vec<Vec<(i32, i32)>> {
        let all = tile.layer_coordinates(0);
        tile.feature_part_offsets(0, feature)
            .windows(2)
            .map(|part| {
                all[part[0] as usize..part[1] as usize]
                    .iter()
                    .map(|c| (c.x, c.y))
                    .collect()
            })
            .collect()
    }

    fn triangle_area(points: &[(i32, i32)], triangle: &[u32]) -> f64 {
        let [a, b, c] = [0, 1, 2].map(|i| points[triangle[i] as usize]);
        f64::from((b.0 - a.0) * (c.1 - a.1) - (c.0 - a.0) * (b.1 - a.1)).abs() / 2.0
    }

    #[test]
    fn polygon_with_hole_keeps_triangles_aligned_with_closed_rings() {
        let mut layer = TileLayer::builder("water", 4096).unwrap();
        layer
            .feature(Geometry::Polygon(polygon!(
                exterior: [(x: 0, y: 0), (x: 10, y: 0), (x: 10, y: 10), (x: 0, y: 10)],
                interiors: [[(x: 3, y: 3), (x: 3, y: 6), (x: 6, y: 6), (x: 6, y: 3)]],
            )))
            .finish()
            .unwrap();
        let tile = decode_mlt_tile(&encode(layer.finish())).unwrap();

        assert_eq!(tile.feature_type(0, 0), ffi::MltFeatureType::Polygon);
        let rings = coordinates(&tile, 0);
        assert_eq!(rings.len(), 2);
        assert!(rings
            .iter()
            .all(|ring| ring.len() == 5 && ring[0] == ring[4]));

        // The triangles cover the polygon minus its hole exactly.
        let points: Vec<_> = rings.concat();
        let triangles = tile.feature_triangles(0, 0);
        assert!(!triangles.is_empty());
        assert_eq!(triangles.len() % 3, 0);
        let area: f64 = triangles
            .chunks(3)
            .map(|triangle| triangle_area(&points, triangle))
            .sum();
        assert_eq!(area, 100.0 - 9.0);
    }

    #[test]
    fn multi_points_and_lines() {
        let mut layer = TileLayer::builder("layer", 4096).unwrap();
        layer
            .feature(Geometry::MultiPoint(MultiPoint(vec![
                point!(x: 1, y: 2),
                point!(x: 3, y: 4),
            ])))
            .finish()
            .unwrap();
        layer
            .feature(Geometry::LineString(
                line_string![(x: 0, y: 0), (x: 5, y: 5)],
            ))
            .finish()
            .unwrap();
        let tile = decode_mlt_tile(&encode(layer.finish())).unwrap();

        assert_eq!(tile.feature_count(0), 2);
        assert_eq!(tile.feature_type(0, 0), ffi::MltFeatureType::Point);
        assert_eq!(coordinates(&tile, 0), vec![vec![(1, 2)], vec![(3, 4)]]);
        assert_eq!(tile.feature_type(0, 1), ffi::MltFeatureType::LineString);
        assert_eq!(coordinates(&tile, 1), vec![vec![(0, 0), (5, 5)]]);
        assert!(tile.feature_triangles(0, 1).is_empty());
    }

    #[test]
    fn properties_and_ids() {
        let mut layer = TileLayer::builder("places", 512).unwrap();
        let name = layer.add_property("name", PropKind::Str).unwrap();
        let rank = layer.add_property("rank", PropKind::U32).unwrap();
        let mut feature = layer.feature(Geometry::Point(point!(x: 1, y: 1)));
        feature
            .id(Some(7))
            .property(name, PropValue::Str(Some("Utrecht".to_string())))
            .unwrap()
            .property(rank, PropValue::U32(Some(3)))
            .unwrap();
        feature.finish().unwrap();
        // The second feature has no name.
        let mut feature = layer.feature(Geometry::Point(point!(x: 2, y: 2)));
        feature.property(rank, PropValue::U32(Some(5))).unwrap();
        feature.finish().unwrap();
        let tile = decode_mlt_tile(&encode(layer.finish())).unwrap();

        assert_eq!(tile.layer_count(), 1);
        assert_eq!(tile.layer_index(b"places"), 0);
        assert_eq!(tile.layer_index(b"roads"), -1);
        assert_eq!(tile.layer_name(0), "places");
        assert_eq!(tile.layer_extent(0), 512);

        assert_eq!(tile.property_count(0), 2);
        let name_index = tile.property_index(0, b"name");
        let rank_index = tile.property_index(0, b"rank");
        assert!(name_index >= 0 && rank_index >= 0);
        assert_eq!(tile.property_index(0, b"population"), -1);
        assert_eq!(tile.property_index(0, b"\xff"), -1);
        assert_eq!(tile.property_name(0, name_index as usize), "name");

        // Features may be reordered by the encoder, so find them by rank.
        let rank_of = |feature| tile.feature_properties(0, feature)[rank_index as usize].uint_value;
        let first = (0..2).find(|&feature| rank_of(feature) == 3).unwrap();
        let second = 1 - first;

        let values = tile.feature_properties(0, first);
        assert_eq!(values.len(), 2);
        assert_eq!(values[name_index as usize].kind, ffi::MltValueType::String);
        assert_eq!(values[name_index as usize].string_value, "Utrecht");
        assert_eq!(values[rank_index as usize].kind, ffi::MltValueType::UInt);
        assert_eq!(
            tile.feature_id(0, first),
            ffi::MltFeatureId {
                has_id: true,
                value: 7
            }
        );

        let values = tile.feature_properties(0, second);
        assert_eq!(values[name_index as usize].kind, ffi::MltValueType::Null);
        assert!(!tile.feature_id(0, second).has_id);
    }

    #[test]
    fn out_of_range_access_is_empty() {
        let tile = decode_mlt_tile(&[]).unwrap();
        assert_eq!(tile.layer_count(), 0);
        assert_eq!(tile.layer_name(3), "");
        assert_eq!(tile.feature_count(3), 0);
        assert!(tile.feature_properties(3, 0).is_empty());
        assert!(tile.feature_part_offsets(3, 0).is_empty());
        assert_eq!(tile.feature_type(3, 0), ffi::MltFeatureType::Unknown);
    }

    #[test]
    fn invalid_data_is_an_error() {
        assert!(decode_mlt_tile(&[0xff, 0xff, 0xff]).is_err());
    }
}
