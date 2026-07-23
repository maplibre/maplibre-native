use mlt_core::{
    Decoder, GeometryType, GeometryValues, Layer, LendingIterator, ParsedLayer01, Parser,
    PropValueRef,
};

#[cxx::bridge(namespace = "rustutils")]
mod ffi {
    #[derive(Clone, Copy, PartialEq, Eq)]
    enum MltGeometryType {
        Unknown = 0,
        Point = 1,
        LineString = 2,
        Polygon = 3,
        MultiPoint = 4,
        MultiLineString = 5,
        MultiPolygon = 6,
    }

    #[derive(Clone, Copy, PartialEq, Eq)]
    enum MltValueType {
        Missing = 0,
        Null = 1,
        Bool = 2,
        I64 = 3,
        U64 = 4,
        F32 = 5,
        F64 = 6,
        String = 7,
    }

    #[derive(Clone, Copy)]
    struct MltValue {
        typ: MltValueType,
        bool_value: bool,
        i64_value: i64,
        u64_value: u64,
        f32_value: f32,
        f64_value: f64,
    }

    #[derive(Clone, Copy)]
    struct MltFeatureId {
        has_id: bool,
        value: u64,
    }

    #[derive(Clone, Copy)]
    struct MltCoordinate {
        x: i32,
        y: i32,
    }

    extern "Rust" {
        type MltTile;

        fn parse_mlt_tile_metadata(data: &[u8]) -> Result<Box<MltTile>>;
        fn decode_mlt_tile(data: &[u8]) -> Result<Box<MltTile>>;

        fn is_decoded(&self) -> bool;
        fn layer_count(&self) -> usize;
        fn layer_index_by_name(&self, name: &str) -> i64;
        fn layer_name(&self, layer_index: usize) -> String;
        fn layer_extent(&self, layer_index: usize) -> u32;

        fn feature_count(&self, layer_index: usize) -> usize;
        fn feature_id(&self, layer_index: usize, feature_index: usize) -> MltFeatureId;
        fn feature_type(&self, layer_index: usize, feature_index: usize) -> MltGeometryType;

        fn property_count(&self, layer_index: usize) -> usize;
        fn property_index_by_name(&self, layer_index: usize, name: &str) -> i64;
        fn property_name(&self, layer_index: usize, property_index: usize) -> String;
        fn property_value(
            &self,
            layer_index: usize,
            feature_index: usize,
            property_index: usize,
        ) -> MltValue;
        fn property_value_by_name(
            &self,
            layer_index: usize,
            feature_index: usize,
            name: &str,
        ) -> MltValue;
        fn property_values(
            &self,
            layer_index: usize,
            feature_index: usize,
        ) -> &[MltValue];
        fn property_string_value(
            &self,
            layer_index: usize,
            feature_index: usize,
            property_index: usize,
        ) -> String;
        fn property_string_value_by_name(
            &self,
            layer_index: usize,
            feature_index: usize,
            name: &str,
        ) -> String;

        fn geometry_part_count(&self, layer_index: usize, feature_index: usize) -> usize;
        fn geometry_part_coordinate_count(
            &self,
            layer_index: usize,
            feature_index: usize,
            part_index: usize,
        ) -> usize;
        fn geometry_coordinate(
            &self,
            layer_index: usize,
            feature_index: usize,
            part_index: usize,
            coordinate_index: usize,
        ) -> MltCoordinate;
        fn geometry_part_offsets(&self, layer_index: usize, feature_index: usize) -> &[u32];
        fn geometry_coordinates(
            &self,
            layer_index: usize,
            feature_index: usize,
        ) -> &[MltCoordinate];

        fn geometry_triangle_count(&self, layer_index: usize, feature_index: usize) -> usize;
        fn geometry_triangle(
            &self,
            layer_index: usize,
            feature_index: usize,
            triangle_index: usize,
        ) -> u32;
        fn geometry_triangles(&self, layer_index: usize, feature_index: usize) -> &[u32];
    }
}

pub struct MltTile {
    layers: Vec<MltLayer>,
    decoded: bool,
}

struct MltLayer {
    name: String,
    extent: u32,
    property_names: Vec<String>,
    property_values: Vec<MltPropertyValue>,
    ffi_property_values: Vec<ffi::MltValue>,
    part_offsets: Vec<u32>,
    coordinates: Vec<ffi::MltCoordinate>,
    triangle_indices: Vec<u32>,
    features: Vec<MltFeature>,
}

struct MltFeature {
    id: Option<u64>,
    geometry_type: ffi::MltGeometryType,
    part_offset_start: usize,
    part_offset_end: usize,
    triangle_start: usize,
    triangle_end: usize,
}

struct MltGeometry {
    geometry_type: ffi::MltGeometryType,
    part_offsets: Vec<u32>,
    coordinates: Vec<ffi::MltCoordinate>,
}

enum MltPropertyValue {
    Null,
    Bool(bool),
    I64(i64),
    U64(u64),
    F32(f32),
    F64(f64),
    String(String),
}

pub fn decode_mlt_tile(data: &[u8]) -> Result<Box<MltTile>, String> {
    let layers = parse_layers(data)?;
    let mut decoder = Decoder::default();
    let mut result = Vec::with_capacity(layers.len());
    for layer in layers {
        match layer.decode_all(&mut decoder).map_err(|err| err.to_string())? {
            Layer::Tag01(layer) => {
                let triangles = feature_triangles(layer.geometry_values());
                result.push(convert_layer(&layer, triangles)?);
            }
            Layer::Unknown(_) => {}
            _ => {}
        }
    }

    Ok(Box::new(MltTile {
        layers: result,
        decoded: true,
    }))
}

pub fn parse_mlt_tile_metadata(data: &[u8]) -> Result<Box<MltTile>, String> {
    let layers = parse_layers(data)?;
    let mut result = Vec::with_capacity(layers.len());

    for layer in layers {
        if let Layer::Tag01(layer) = layer {
            result.push(MltLayer {
                name: layer.name().to_string(),
                extent: layer.extent().get(),
                property_names: layer
                    .iterate_prop_names()
                    .map(|name| name.to_string())
                    .collect(),
                property_values: Vec::new(),
                ffi_property_values: Vec::new(),
                part_offsets: vec![0],
                coordinates: Vec::new(),
                triangle_indices: Vec::new(),
                features: Vec::new(),
            });
        }
    }

    Ok(Box::new(MltTile {
        layers: result,
        decoded: false,
    }))
}

fn parse_layers(data: &[u8]) -> Result<Vec<Layer<'_>>, String> {
    let mut parser = Parser::default();
    parser.parse_layers(data).map_err(|err| err.to_string())
}

fn feature_triangles(geometry: &mlt_core::GeometryValues) -> Vec<Vec<u32>> {
    let mut result = vec![Vec::new(); geometry.feature_count()];
    let (Some(triangle_counts), Some(index_buffer)) = (geometry.triangles(), geometry.index_buffer())
    else {
        return result;
    };

    let mut triangle_count_index = 0usize;
    let mut index_buffer_offset = 0usize;
    for (feature_index, geometry_type) in geometry.vector_types().iter().copied().enumerate() {
        if matches!(
            geometry_type,
            mlt_core::GeometryType::Polygon | mlt_core::GeometryType::MultiPolygon
        ) {
            let Some(&num_triangles) = triangle_counts.get(triangle_count_index) else {
                break;
            };
            triangle_count_index += 1;

            let index_count = 3usize.saturating_mul(num_triangles as usize);
            let end = index_buffer_offset.saturating_add(index_count);
            if let Some(indices) = index_buffer.get(index_buffer_offset..end) {
                result[feature_index] = indices.to_vec();
            }
            index_buffer_offset = end;
        }
    }

    result
}

fn convert_layer(layer: &ParsedLayer01<'_>, triangles: Vec<Vec<u32>>) -> Result<MltLayer, String> {
    let property_names: Vec<String> = layer
        .iterate_prop_names()
        .map(|name| name.to_string())
        .collect();

    let feature_count = layer.feature_count();
    let property_count = property_names.len();
    let mut features = Vec::with_capacity(feature_count);
    let mut property_values = Vec::with_capacity(feature_count.saturating_mul(property_count));
    let mut ffi_property_values = Vec::with_capacity(feature_count.saturating_mul(property_count));
    let mut part_offsets = Vec::new();
    let mut coordinates = Vec::new();
    let mut triangle_indices = Vec::new();
    part_offsets.push(0);

    let geometry_values = layer.geometry_values();
    let mut feature_iter = layer.iter_features();
    while let Some(feature_ref) = feature_iter.next() {
        let feature_ref = feature_ref.map_err(|err| err.to_string())?;
        let index = features.len();
        let geometry = convert_geometry(geometry_values, index);
        let triangle_slice = triangles.get(index).map_or(&[][..], Vec::as_slice);
        let feature = convert_feature(
            feature_ref.id(),
            geometry,
            triangle_slice,
            &mut part_offsets,
            &mut coordinates,
            &mut triangle_indices,
        );
        for property in feature_ref.iter_all_properties().map(convert_property) {
            ffi_property_values.push(value_for_property_value(&property));
            property_values.push(property);
        }
        features.push(feature);
    }

    Ok(MltLayer {
        name: layer.name().to_string(),
        extent: layer.extent().get(),
        property_names,
        property_values,
        ffi_property_values,
        part_offsets,
        coordinates,
        triangle_indices,
        features,
    })
}

fn convert_feature(
    id: Option<u64>,
    geometry: MltGeometry,
    triangles: &[u32],
    layer_part_offsets: &mut Vec<u32>,
    layer_coordinates: &mut Vec<ffi::MltCoordinate>,
    layer_triangle_indices: &mut Vec<u32>,
) -> MltFeature {
    let part_offset_start = layer_part_offsets.len().saturating_sub(1);
    let coordinate_base = layer_coordinates.len() as u32;
    layer_part_offsets.extend(
        geometry
            .part_offsets
            .iter()
            .copied()
            .skip(1)
            .map(|offset| coordinate_base.saturating_add(offset)),
    );
    let part_offset_end = layer_part_offsets.len();
    layer_coordinates.extend(geometry.coordinates);

    let triangle_start = layer_triangle_indices.len();
    layer_triangle_indices.extend_from_slice(triangles);
    let triangle_end = layer_triangle_indices.len();

    MltFeature {
        id,
        geometry_type: geometry.geometry_type,
        part_offset_start,
        part_offset_end,
        triangle_start,
        triangle_end,
    }
}

fn convert_property(value: Option<PropValueRef<'_>>) -> MltPropertyValue {
    match value {
        Some(PropValueRef::Bool(value)) => MltPropertyValue::Bool(value),
        Some(PropValueRef::I8(value)) => MltPropertyValue::I64(i64::from(value)),
        Some(PropValueRef::U8(value)) => MltPropertyValue::U64(u64::from(value)),
        Some(PropValueRef::I32(value)) => MltPropertyValue::I64(i64::from(value)),
        Some(PropValueRef::U32(value)) => MltPropertyValue::U64(u64::from(value)),
        Some(PropValueRef::I64(value)) => MltPropertyValue::I64(value),
        Some(PropValueRef::U64(value)) => MltPropertyValue::U64(value),
        Some(PropValueRef::F32(value)) => MltPropertyValue::F32(value),
        Some(PropValueRef::F64(value)) => MltPropertyValue::F64(value),
        Some(PropValueRef::Str(value)) => MltPropertyValue::String(value.to_string()),
        None => MltPropertyValue::Null,
    }
}

fn convert_geometry(geometry: &GeometryValues, feature_index: usize) -> MltGeometry {
    let geometry_type = geometry
        .vector_types()
        .get(feature_index)
        .copied()
        .map(geometry_type)
        .unwrap_or(ffi::MltGeometryType::Unknown);

    let mut part_offsets = Vec::new();
    let mut coordinates = Vec::new();
    part_offsets.push(0);

    append_geometry_parts(geometry, feature_index, &mut coordinates, &mut part_offsets);

    MltGeometry {
        geometry_type,
        part_offsets,
        coordinates,
    }
}

fn append_geometry_parts(
    geometry: &GeometryValues,
    feature_index: usize,
    coordinates: &mut Vec<ffi::MltCoordinate>,
    part_offsets: &mut Vec<u32>,
) {
    let vertices = geometry.vertices().unwrap_or(&[]);
    let geometry_offsets = geometry.geometry_offsets();
    let part_offsets_in = geometry.part_offsets();
    let ring_offsets = geometry.ring_offsets();
    let Some(geometry_type) = geometry.vector_types().get(feature_index).copied() else {
        return;
    };

    match geometry_type {
        GeometryType::Point => {
            let Some(mut vertex_index) = geometry_offsets.map_or(Some(feature_index), |offsets| {
                offset(offsets, feature_index)
            }) else {
                return;
            };
            if let Some(offsets) = part_offsets_in {
                let Some(offset) = offset(offsets, vertex_index) else {
                    return;
                };
                vertex_index = offset;
            }
            if let Some(offsets) = ring_offsets {
                let Some(offset) = offset(offsets, vertex_index) else {
                    return;
                };
                vertex_index = offset;
            }
            push_vertex(vertices, vertex_index, coordinates);
            push_part_offset(coordinates, part_offsets);
        }
        GeometryType::LineString => {
            let Some(parts) = part_offsets_in else {
                return;
            };
            let Some(part_index) = geometry_offsets.map_or(Some(feature_index), |offsets| {
                offset(offsets, feature_index)
            }) else {
                return;
            };
            if let Some(rings) = ring_offsets {
                let Some(ring_index) = offset(parts, part_index) else {
                    return;
                };
                push_vertex_range(vertices, offset_range(rings, ring_index), false, coordinates);
            } else {
                push_vertex_range(vertices, offset_range(parts, part_index), false, coordinates);
            }
            push_part_offset(coordinates, part_offsets);
        }
        GeometryType::Polygon => {
            let (Some(parts), Some(rings)) = (part_offsets_in, ring_offsets) else {
                return;
            };
            let Some(part_index) = geometry_offsets.map_or(Some(feature_index), |offsets| {
                offset(offsets, feature_index)
            }) else {
                return;
            };
            append_polygon_parts(vertices, parts, rings, part_index, coordinates, part_offsets);
        }
        GeometryType::MultiPoint => {
            let Some(geometry_range) = geometry_offsets.and_then(|offsets| offset_range(offsets, feature_index)) else {
                return;
            };
            for index in geometry_range {
                let vertex_index = match (part_offsets_in, ring_offsets) {
                    (Some(parts), Some(rings)) => {
                        let Some(part_index) = offset(parts, index) else {
                            continue;
                        };
                        let Some(ring_index) = offset(rings, part_index) else {
                            continue;
                        };
                        ring_index
                    }
                    (Some(parts), None) => {
                        let Some(part_index) = offset(parts, index) else {
                            continue;
                        };
                        part_index
                    }
                    (None, _) => index,
                };
                push_vertex(vertices, vertex_index, coordinates);
            }
            push_part_offset(coordinates, part_offsets);
        }
        GeometryType::MultiLineString => {
            let (Some(geometry_offsets), Some(parts)) = (geometry_offsets, part_offsets_in) else {
                return;
            };
            let Some(geometry_range) = offset_range(geometry_offsets, feature_index) else {
                return;
            };
            for index in geometry_range {
                if let Some(rings) = ring_offsets {
                    let Some(ring_index) = offset(parts, index) else {
                        continue;
                    };
                    push_vertex_range(vertices, offset_range(rings, ring_index), false, coordinates);
                } else {
                    push_vertex_range(vertices, offset_range(parts, index), false, coordinates);
                }
                push_part_offset(coordinates, part_offsets);
            }
        }
        GeometryType::MultiPolygon => {
            let (Some(geometry_offsets), Some(parts), Some(rings)) =
                (geometry_offsets, part_offsets_in, ring_offsets)
            else {
                return;
            };
            let Some(geometry_range) = offset_range(geometry_offsets, feature_index) else {
                return;
            };
            for part_index in geometry_range {
                append_polygon_parts(vertices, parts, rings, part_index, coordinates, part_offsets);
            }
        }
    }
}

fn geometry_type(geometry_type: GeometryType) -> ffi::MltGeometryType {
    match geometry_type {
        GeometryType::Point => ffi::MltGeometryType::Point,
        GeometryType::LineString => ffi::MltGeometryType::LineString,
        GeometryType::Polygon => ffi::MltGeometryType::Polygon,
        GeometryType::MultiPoint => ffi::MltGeometryType::MultiPoint,
        GeometryType::MultiLineString => ffi::MltGeometryType::MultiLineString,
        GeometryType::MultiPolygon => ffi::MltGeometryType::MultiPolygon,
    }
}

fn offset(offsets: &[u32], index: usize) -> Option<usize> {
    offsets.get(index).map(|offset| *offset as usize)
}

fn offset_range(offsets: &[u32], index: usize) -> Option<std::ops::Range<usize>> {
    Some(offset(offsets, index)?..offset(offsets, index + 1)?)
}

fn vertex(vertices: &[i32], index: usize) -> Option<ffi::MltCoordinate> {
    let coordinate_index = index.checked_mul(2)?;
    let pair = vertices.get(coordinate_index..coordinate_index + 2)?;
    Some(ffi::MltCoordinate {
        x: pair[0],
        y: pair[1],
    })
}

fn push_vertex(
    vertices: &[i32],
    index: usize,
    coordinates: &mut Vec<ffi::MltCoordinate>,
) {
    if let Some(coordinate) = vertex(vertices, index) {
        coordinates.push(coordinate);
    }
}

fn push_vertex_range(
    vertices: &[i32],
    range: Option<std::ops::Range<usize>>,
    close_ring: bool,
    coordinates: &mut Vec<ffi::MltCoordinate>,
) {
    let Some(range) = range else {
        return;
    };
    let first = range.start;
    for index in range {
        push_vertex(vertices, index, coordinates);
    }
    if close_ring {
        push_vertex(vertices, first, coordinates);
    }
}

fn append_polygon_parts(
    vertices: &[i32],
    parts: &[u32],
    rings: &[u32],
    part_index: usize,
    coordinates: &mut Vec<ffi::MltCoordinate>,
    part_offsets: &mut Vec<u32>,
) {
    let Some(ring_range) = offset_range(parts, part_index) else {
        return;
    };
    for ring_index in ring_range {
        push_vertex_range(vertices, offset_range(rings, ring_index), true, coordinates);
        push_part_offset(coordinates, part_offsets);
    }
}

fn push_part_offset(coordinates: &[ffi::MltCoordinate], part_offsets: &mut Vec<u32>) {
    part_offsets.push(coordinates.len() as u32);
}

fn missing_value() -> ffi::MltValue {
    value(ffi::MltValueType::Missing)
}

fn null_value() -> ffi::MltValue {
    value(ffi::MltValueType::Null)
}

fn value(typ: ffi::MltValueType) -> ffi::MltValue {
    ffi::MltValue {
        typ,
        bool_value: false,
        i64_value: 0,
        u64_value: 0,
        f32_value: 0.0,
        f64_value: 0.0,
    }
}

fn value_for_property(property: Option<&MltPropertyValue>) -> ffi::MltValue {
    match property {
        Some(property) => value_for_property_value(property),
        None => missing_value(),
    }
}

fn value_for_property_value(property: &MltPropertyValue) -> ffi::MltValue {
    match property {
        MltPropertyValue::Null => null_value(),
        MltPropertyValue::Bool(bool_value) => ffi::MltValue {
            typ: ffi::MltValueType::Bool,
            bool_value: *bool_value,
            ..value(ffi::MltValueType::Bool)
        },
        MltPropertyValue::I64(i64_value) => ffi::MltValue {
            typ: ffi::MltValueType::I64,
            i64_value: *i64_value,
            ..value(ffi::MltValueType::I64)
        },
        MltPropertyValue::U64(u64_value) => ffi::MltValue {
            typ: ffi::MltValueType::U64,
            u64_value: *u64_value,
            ..value(ffi::MltValueType::U64)
        },
        MltPropertyValue::F32(f32_value) => ffi::MltValue {
            typ: ffi::MltValueType::F32,
            f32_value: *f32_value,
            ..value(ffi::MltValueType::F32)
        },
        MltPropertyValue::F64(f64_value) => ffi::MltValue {
            typ: ffi::MltValueType::F64,
            f64_value: *f64_value,
            ..value(ffi::MltValueType::F64)
        },
        MltPropertyValue::String(_) => value(ffi::MltValueType::String),
    }
}

impl MltTile {
    fn is_decoded(&self) -> bool {
        self.decoded
    }

    fn layer(&self, layer_index: usize) -> Option<&MltLayer> {
        self.layers.get(layer_index)
    }

    fn feature(&self, layer_index: usize, feature_index: usize) -> Option<&MltFeature> {
        self.layer(layer_index)?.features.get(feature_index)
    }

    fn property<'a>(
        &'a self,
        layer_index: usize,
        feature_index: usize,
        property_index: usize,
    ) -> Option<&'a MltPropertyValue> {
        let layer = self.layer(layer_index)?;
        let offset = feature_index
            .checked_mul(layer.property_names.len())?
            .checked_add(property_index)?;
        layer.property_values.get(offset)
    }

    fn layer_count(&self) -> usize {
        self.layers.len()
    }

    fn layer_index_by_name(&self, name: &str) -> i64 {
        self.layers
            .iter()
            .position(|layer| layer.name == name)
            .map_or(-1, |index| index as i64)
    }

    fn layer_name(&self, layer_index: usize) -> String {
        self.layer(layer_index)
            .map_or_else(String::new, |layer| layer.name.clone())
    }

    fn layer_extent(&self, layer_index: usize) -> u32 {
        self.layer(layer_index).map_or(0, |layer| layer.extent)
    }

    fn feature_count(&self, layer_index: usize) -> usize {
        self.layer(layer_index)
            .map_or(0, |layer| layer.features.len())
    }

    fn feature_id(&self, layer_index: usize, feature_index: usize) -> ffi::MltFeatureId {
        match self.feature(layer_index, feature_index).and_then(|feature| feature.id) {
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

    fn feature_type(&self, layer_index: usize, feature_index: usize) -> ffi::MltGeometryType {
        self.feature(layer_index, feature_index)
            .map_or(ffi::MltGeometryType::Unknown, |feature| feature.geometry_type)
    }

    fn property_count(&self, layer_index: usize) -> usize {
        self.layer(layer_index)
            .map_or(0, |layer| layer.property_names.len())
    }

    fn property_index_by_name(&self, layer_index: usize, name: &str) -> i64 {
        self.layer(layer_index)
            .and_then(|layer| layer.property_names.iter().position(|property| property == name))
            .map_or(-1, |index| index as i64)
    }

    fn property_name(&self, layer_index: usize, property_index: usize) -> String {
        self.layer(layer_index)
            .and_then(|layer| layer.property_names.get(property_index))
            .cloned()
            .unwrap_or_default()
    }

    fn property_value(
        &self,
        layer_index: usize,
        feature_index: usize,
        property_index: usize,
    ) -> ffi::MltValue {
        value_for_property(self.property(layer_index, feature_index, property_index))
    }

    fn property_value_by_name(
        &self,
        layer_index: usize,
        feature_index: usize,
        name: &str,
    ) -> ffi::MltValue {
        let property_index = self.property_index_by_name(layer_index, name);
        if property_index < 0 {
            return missing_value();
        }
        self.property_value(layer_index, feature_index, property_index as usize)
    }

    fn property_values(&self, layer_index: usize, feature_index: usize) -> &[ffi::MltValue] {
        let Some(layer) = self.layer(layer_index) else {
            return &[];
        };
        let property_count = layer.property_names.len();
        let Some(start) = feature_index.checked_mul(property_count) else {
            return &[];
        };
        layer
            .ffi_property_values
            .get(start..start.saturating_add(property_count))
            .unwrap_or(&[])
    }

    fn property_string_value(
        &self,
        layer_index: usize,
        feature_index: usize,
        property_index: usize,
    ) -> String {
        match self.property(layer_index, feature_index, property_index) {
            Some(MltPropertyValue::String(value)) => value.clone(),
            _ => String::new(),
        }
    }

    fn property_string_value_by_name(
        &self,
        layer_index: usize,
        feature_index: usize,
        name: &str,
    ) -> String {
        let property_index = self.property_index_by_name(layer_index, name);
        if property_index < 0 {
            return String::new();
        }
        self.property_string_value(layer_index, feature_index, property_index as usize)
    }

    fn geometry_part_count(&self, layer_index: usize, feature_index: usize) -> usize {
        self.feature(layer_index, feature_index)
            .map_or(0, |feature| {
                feature
                    .part_offset_end
                    .saturating_sub(feature.part_offset_start)
                    .saturating_sub(1)
            })
    }

    fn geometry_part_coordinate_count(
        &self,
        layer_index: usize,
        feature_index: usize,
        part_index: usize,
    ) -> usize {
        let Some(layer) = self.layer(layer_index) else {
            return 0;
        };
        let Some(feature) = layer.features.get(feature_index) else {
            return 0;
        };
        let offset_index = feature.part_offset_start + part_index;
        let Some(start) = layer.part_offsets.get(offset_index) else {
            return 0;
        };
        let Some(end) = layer.part_offsets.get(offset_index + 1) else {
            return 0;
        };
        end.saturating_sub(*start) as usize
    }

    fn geometry_coordinate(
        &self,
        layer_index: usize,
        feature_index: usize,
        part_index: usize,
        coordinate_index: usize,
    ) -> ffi::MltCoordinate {
        let Some(layer) = self.layer(layer_index) else {
            return ffi::MltCoordinate { x: 0, y: 0 };
        };
        let Some(feature) = layer.features.get(feature_index) else {
            return ffi::MltCoordinate { x: 0, y: 0 };
        };
        let offset_index = feature.part_offset_start + part_index;
        let Some(start) = layer.part_offsets.get(offset_index) else {
            return ffi::MltCoordinate { x: 0, y: 0 };
        };
        layer
            .coordinates
            .get(*start as usize + coordinate_index)
            .copied()
            .unwrap_or(ffi::MltCoordinate { x: 0, y: 0 })
    }

    fn geometry_part_offsets(&self, layer_index: usize, feature_index: usize) -> &[u32] {
        let Some(layer) = self.layer(layer_index) else {
            return &[];
        };
        let Some(feature) = layer.features.get(feature_index) else {
            return &[];
        };
        layer
            .part_offsets
            .get(feature.part_offset_start..feature.part_offset_end)
            .unwrap_or(&[])
    }

    fn geometry_coordinates(
        &self,
        layer_index: usize,
        _feature_index: usize,
    ) -> &[ffi::MltCoordinate] {
        self.layer(layer_index)
            .map_or(&[], |layer| layer.coordinates.as_slice())
    }

    fn geometry_triangle_count(&self, layer_index: usize, feature_index: usize) -> usize {
        self.feature(layer_index, feature_index)
            .map_or(0, |feature| {
                feature.triangle_end.saturating_sub(feature.triangle_start)
            })
    }

    fn geometry_triangle(
        &self,
        layer_index: usize,
        feature_index: usize,
        triangle_index: usize,
    ) -> u32 {
        let Some(layer) = self.layer(layer_index) else {
            return 0;
        };
        let Some(feature) = layer.features.get(feature_index) else {
            return 0;
        };
        layer
            .triangle_indices
            .get(feature.triangle_start + triangle_index)
            .copied()
            .unwrap_or(0)
    }

    fn geometry_triangles(&self, layer_index: usize, feature_index: usize) -> &[u32] {
        let Some(layer) = self.layer(layer_index) else {
            return &[];
        };
        let Some(feature) = layer.features.get(feature_index) else {
            return &[];
        };
        layer
            .triangle_indices
            .get(feature.triangle_start..feature.triangle_end)
            .unwrap_or(&[])
    }
}
