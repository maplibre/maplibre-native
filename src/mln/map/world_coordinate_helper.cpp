#include <mln/map/world_coordinate_helper.hpp>
#include <mln/util/constants.hpp>
#include <mln/math/clamp.hpp>
#include <mln/util/projection.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace mln {

namespace {

using Pair = std::array<double, 2>;

constexpr Pair noAnswer{std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};

bool isAnswer(const Pair& pair) {
    return std::isfinite(pair[0]) && std::isfinite(pair[1]);
}

class MercatorWorldCoordinateHelper final : public WorldCoordinateHelper {
public:
    const std::string& name() const override { return name_; }

    void worldFromLngLats(std::span<const Pair> lngLats, std::span<Pair> world) const override {
        for (std::size_t i = 0; i < lngLats.size(); ++i) {
            const Point<double> point = Projection::project({lngLats[i][1], lngLats[i][0]}, 1.0 / util::tileSize_D);
            world[i] = {point.x, point.y};
        }
    }

    void lngLatsFromWorld(std::span<const Pair> world, std::span<Pair> lngLats) const override {
        for (std::size_t i = 0; i < world.size(); ++i) {
            const LatLng latLng = Projection::unproject({world[i][0], world[i][1]}, 1.0 / util::tileSize_D);
            lngLats[i] = {latLng.longitude(), latLng.latitude()};
        }
    }

    double metersPerWorldUnit(const Point<double>& point) const override {
        return Projection::getMetersPerPixelAtLatitude(latLngFromWorld(point).latitude(), 0) * util::tileSize_D;
    }

    bool wraps() const override { return true; }

    const TileMatrixSet& tileMatrixSet() const override { return mercatorTileMatrixSet; }

private:
    const std::string name_ = "mercator";
};

// How far `forward` may take an inverse from the position it came from, as a fraction of the zoom 0 extent, for the
// inverse to count as that position's answer.
constexpr double roundTripTolerance = 1e-3;

// How many times the line to the center of the world square is halved in the search for the last position with an
// answer.
constexpr int edgeSearchSteps = 30;

// How many times as much as at the center of its tile matrix set a projection may stretch distances along the meridian.
constexpr double maxMeridianStretch = 10;

// How many parts a lat/lng box is cut into along each axis when it is sampled.
constexpr int boxSampleSteps = 16;

double boxLongitudeAt(const LatLngBounds& box, int step) {
    return step == boxSampleSteps ? box.east() : box.west() + (box.east() - box.west()) * step / boxSampleSteps;
}

double boxLatitudeAt(const LatLngBounds& box, int step) {
    return step == boxSampleSteps ? box.south() : box.north() + (box.south() - box.north()) * step / boxSampleSteps;
}

// Pairs for one converter call, on the stack for a single one so a scalar call allocates nothing.
class Pairs {
public:
    explicit Pairs(std::size_t size)
        : heap(size > 1 ? size : 0),
          pairs(size > 1 ? std::span<Pair>(heap) : std::span<Pair>(local).first(size)) {}
    Pairs(const Pairs&) = delete;
    Pairs& operator=(const Pairs&) = delete;

    std::span<Pair> span() const { return pairs; }

private:
    std::array<Pair, 1> local{};
    std::vector<Pair> heap;
    std::span<Pair> pairs;
};

void copyPairs(std::span<const Pair> in, std::span<Pair> out) {
    std::ranges::copy(in, out.begin());
}

std::shared_ptr<const CRSWorldCoordinateHelper> identityHelper() {
    return std::make_shared<const CRSWorldCoordinateHelper>(
        std::make_shared<const CRS>(CRS{.name = "identity",
                                        .forward = copyPairs,
                                        .inverse = copyPairs,
                                        .tileMatrixSet = {.origin = {-90, 90}, .extentAtZoom0 = 180}}));
}

std::mutex registryMutex;

std::unordered_map<std::string, std::shared_ptr<const CRSWorldCoordinateHelper>>& registry() {
    static std::unordered_map<std::string, std::shared_ptr<const CRSWorldCoordinateHelper>> projections{
        {"identity", identityHelper()}};
    return projections;
}

} // namespace

Point<double> WorldCoordinateHelper::worldFromLatLng(const LatLng& latLng) const {
    const std::array<Pair, 1> lngLat{{{latLng.longitude(), latLng.latitude()}}};
    std::array<Pair, 1> world{};
    worldFromLngLats(lngLat, world);
    return {world[0][0], world[0][1]};
}

LatLng WorldCoordinateHelper::latLngFromWorld(const Point<double>& point) const {
    const std::array<Pair, 1> world{{{point.x, point.y}}};
    std::array<Pair, 1> lngLat{};
    lngLatsFromWorld(world, lngLat);
    return {lngLat[0][1], lngLat[0][0], LatLng::Unwrapped};
}

const std::shared_ptr<const WorldCoordinateHelper>& mercatorWorldCoordinateHelper() {
    static const std::shared_ptr<const WorldCoordinateHelper> helper =
        std::make_shared<const MercatorWorldCoordinateHelper>();
    return helper;
}

CRSWorldCoordinateHelper::CRSWorldCoordinateHelper(std::shared_ptr<const CRS> crs_)
    : crs(std::move(crs_)),
      originX(crs->tileMatrixSet.origin[0]),
      originY(crs->tileMatrixSet.origin[1]),
      extent(crs->tileMatrixSet.extentAtZoom0) {}

void CRSWorldCoordinateHelper::worldFromLngLats(std::span<const Pair> lngLats, std::span<Pair> world) const {
    if (lngLats.empty()) {
        return;
    }
    crs->forward(lngLats, world);
    for (Pair& position : world) {
        position = {(position[0] - originX) / extent, (originY - position[1]) / extent};
    }
}

void CRSWorldCoordinateHelper::lngLatsFromWorld(std::span<const Pair> world, std::span<Pair> lngLats) const {
    ownAnswers(world, lngLats);
    // The positions still without an answer, by index, and their nearest places inside the world square.
    std::vector<std::size_t> rest;
    std::vector<Pair> inside;
    for (std::size_t i = 0; i < world.size(); ++i) {
        if (!isAnswer(lngLats[i])) {
            rest.push_back(i);
            inside.push_back({util::clamp(world[i][0], 0.0, 1.0), util::clamp(world[i][1], 0.0, 1.0)});
        }
    }
    std::vector<Pair> found(rest.size());
    // Takes the answers found for the rest, and leaves the rest those still without one.
    const auto keepFound = [&] {
        std::size_t kept = 0;
        for (std::size_t k = 0; k < rest.size(); ++k) {
            if (isAnswer(found[k])) {
                lngLats[rest[k]] = found[k];
            } else {
                rest[kept] = rest[k];
                inside[kept] = inside[k];
                ++kept;
            }
        }
        rest.resize(kept);
        inside.resize(kept);
        found.resize(kept);
    };
    if (!rest.empty()) {
        ownAnswers(inside, found);
        keepFound();
    }
    if (!rest.empty()) {
        answersTowardCenter(inside, found);
        keepFound();
    }
    if (!rest.empty()) {
        std::vector<Pair> positions(inside.size());
        std::ranges::transform(inside, positions.begin(), [&](const Pair& point) { return crsPosition(point); });
        finiteInverses(positions, found);
        keepFound();
    }
    if (!rest.empty()) {
        throw std::runtime_error("The inverse of the projection \"" + crs->name +
                                 "\" has no answer inside its tile matrix set.");
    }
    for (Pair& lngLat : lngLats) {
        lngLat[1] = util::clamp(lngLat[1], -90.0, 90.0);
    }
}

Pair CRSWorldCoordinateHelper::crsPosition(const Pair& world) const {
    return {originX + world[0] * extent, originY - world[1] * extent};
}

void CRSWorldCoordinateHelper::finiteInverses(std::span<const Pair> positions, std::span<Pair> lngLats) const {
    crs->inverse(positions, lngLats);
    for (Pair& lngLat : lngLats) {
        if (!isAnswer(lngLat)) {
            lngLat = noAnswer;
        }
    }
}

void CRSWorldCoordinateHelper::ownAnswers(std::span<const Pair> world, std::span<Pair> lngLats) const {
    if (world.empty()) {
        return;
    }
    const Pairs positions(world.size());
    std::ranges::transform(world, positions.span().begin(), [&](const Pair& point) { return crsPosition(point); });
    finiteInverses(positions.span(), lngLats);
    const auto answered = std::ranges::find_if(lngLats, isAnswer);
    if (answered == lngLats.end()) {
        return;
    }
    // An answer counts when forward takes it back to its position. A position without one sends the first answer in
    // its place, so the converter sees only answers, and its own position becomes NaN so the comparison fails.
    const Pair first = *answered;
    for (std::size_t i = 0; i < lngLats.size(); ++i) {
        if (!isAnswer(lngLats[i])) {
            lngLats[i] = first;
            positions.span()[i] = noAnswer;
        }
    }
    const Pairs back(world.size());
    crs->forward(lngLats, back.span());
    for (std::size_t i = 0; i < lngLats.size(); ++i) {
        const Pair& forward = back.span()[i];
        const Pair& position = positions.span()[i];
        const double distance = std::hypot(forward[0] - position[0], forward[1] - position[1]);
        if (!(distance <= roundTripTolerance * extent)) {
            lngLats[i] = noAnswer;
        }
    }
}

void CRSWorldCoordinateHelper::answersTowardCenter(std::span<const Pair> world, std::span<Pair> lngLats) const {
    // For each position, the fraction of the way from the center of the square that last had an answer, and the one
    // that last had none.
    std::vector<double> answered(world.size(), 0);
    std::vector<double> unanswered(world.size(), 1);
    std::vector<Pair> along(world.size());
    const auto alongAt = [&](const Pair& point, double fraction) {
        return Pair{0.5 + (point[0] - 0.5) * fraction, 0.5 + (point[1] - 0.5) * fraction};
    };
    for (int step = 0; step < edgeSearchSteps; ++step) {
        for (std::size_t k = 0; k < world.size(); ++k) {
            along[k] = alongAt(world[k], (answered[k] + unanswered[k]) / 2);
        }
        ownAnswers(along, lngLats);
        for (std::size_t k = 0; k < world.size(); ++k) {
            const double fraction = (answered[k] + unanswered[k]) / 2;
            (isAnswer(lngLats[k]) ? answered[k] : unanswered[k]) = fraction;
        }
    }
    for (std::size_t k = 0; k < world.size(); ++k) {
        along[k] = alongAt(world[k], answered[k]);
    }
    ownAnswers(along, lngLats);
}

const std::array<double, 2>& CRSWorldCoordinateHelper::latitudeRange() const {
    std::call_once(latitudeRangeFlag, [&] {
        const LatLng center = latLngFromWorld({0.5, 0.5});
        latitudeRange_ = {findLatitudeLimit(center, -90), findLatitudeLimit(center, 90)};
    });
    return latitudeRange_;
}

double CRSWorldCoordinateHelper::findLatitudeLimit(const LatLng& center, double pole) const {
    const double step = pole > center.latitude() ? 1 : -1;
    // Whole degrees from the center toward the pole, up to a degree short of it.
    const int steps = std::max(0, static_cast<int>(std::ceil(std::abs(pole - center.latitude()) - 1)));
    // How far a hundredth of a degree toward the equator moves a position on the center's meridian: at the center,
    // then a degree further toward the pole from each whole degree, all in one batch.
    std::vector<Pair> lngLats;
    lngLats.reserve(2 * static_cast<std::size_t>(steps + 1));
    const auto addStretch = [&](double latitude) {
        lngLats.push_back({center.longitude(), latitude});
        lngLats.push_back({center.longitude(), latitude >= 0 ? latitude - 0.01 : latitude + 0.01});
    };
    addStretch(center.latitude());
    for (int i = 0; i < steps; ++i) {
        addStretch(center.latitude() + (i * step) + step);
    }
    std::vector<Pair> positions(lngLats.size());
    crs->forward(lngLats, positions);
    const auto stretch = [&](int i) {
        const Pair& position = positions[2 * i];
        const Pair& towardEquator = positions[(2 * i) + 1];
        return std::hypot(towardEquator[0] - position[0], towardEquator[1] - position[1]);
    };
    const double maxStretch = maxMeridianStretch * stretch(0);
    if (!(maxStretch > 0)) {
        return pole;
    }
    for (int i = 0; i < steps; ++i) {
        if (!(stretch(i + 1) <= maxStretch)) {
            return center.latitude() + (i * step);
        }
    }
    return pole;
}

void PointBatcher::operator()(Point<double>& point) {
    in.push_back({point.x, point.y});
    points.push_back(&point);
    if (in.size() == maxConverterBatch) {
        flush();
    }
}

void PointBatcher::flush() {
    if (in.empty()) {
        return;
    }
    out.resize(in.size());
    convert(in, out);
    for (std::size_t i = 0; i < points.size(); ++i) {
        *points[i] = {out[i][0], out[i][1]};
    }
    in.clear();
    points.clear();
}

void convertPoints(const std::function<void(PointBatcher&)>& forEachPoint, const PointsConverter& convert) {
    PointBatcher batcher(convert);
    forEachPoint(batcher);
    batcher.flush();
}

std::vector<LatLng> latLngBoxSamples(const LatLngBounds& box) {
    std::vector<LatLng> samples;
    samples.reserve((boxSampleSteps + 1) * (boxSampleSteps + 1));
    for (int row = 0; row <= boxSampleSteps; ++row) {
        for (int column = 0; column <= boxSampleSteps; ++column) {
            samples.emplace_back(boxLatitudeAt(box, row), boxLongitudeAt(box, column));
        }
    }
    return samples;
}

std::array<double, 4> latLngBoxToWorldBox(const WorldCoordinateHelper& helper, const LatLngBounds& box) {
    // The corners alone miss most of a box whose edges curve, as a parallel does around a pole. The grid goes in one
    // batch with the halfway points along its edges: north, south, west and east for each step.
    constexpr int side = boxSampleSteps + 1;
    std::vector<Pair> lngLats;
    lngLats.reserve((side * side) + (4 * boxSampleSteps));
    for (const LatLng& sample : latLngBoxSamples(box)) {
        lngLats.push_back({sample.longitude(), sample.latitude()});
    }
    for (int step = 0; step < boxSampleSteps; ++step) {
        const double longitudeHalfway = (boxLongitudeAt(box, step) + boxLongitudeAt(box, step + 1)) / 2;
        const double latitudeHalfway = (boxLatitudeAt(box, step) + boxLatitudeAt(box, step + 1)) / 2;
        lngLats.push_back({longitudeHalfway, box.north()});
        lngLats.push_back({longitudeHalfway, box.south()});
        lngLats.push_back({box.west(), latitudeHalfway});
        lngLats.push_back({box.east(), latitudeHalfway});
    }
    std::vector<Pair> world(lngLats.size());
    helper.worldFromLngLats(lngLats, world);
    const auto pointAt = [&](int index) {
        return Point<double>{world[index][0], world[index][1]};
    };
    const auto gridAt = [&](int row, int column) {
        return pointAt((row * side) + column);
    };
    double bulge = 0;
    for (int step = 0; step < boxSampleSteps; ++step) {
        const auto halfway = [&](int edge) {
            return pointAt((side * side) + (4 * step) + edge);
        };
        const std::array<std::array<Point<double>, 3>, 4> edges{{
            {gridAt(0, step), gridAt(0, step + 1), halfway(0)},
            {gridAt(boxSampleSteps, step), gridAt(boxSampleSteps, step + 1), halfway(1)},
            {gridAt(step, 0), gridAt(step + 1, 0), halfway(2)},
            {gridAt(step, boxSampleSteps), gridAt(step + 1, boxSampleSteps), halfway(3)},
        }};
        for (const auto& [start, end, middle] : edges) {
            const Point<double> chord = end - start;
            const Point<double> toHalfway = middle - start;
            const double chordLength = std::hypot(chord.x, chord.y);
            const double distance = chordLength > 0
                                        ? std::abs(chord.x * toHalfway.y - chord.y * toHalfway.x) / chordLength
                                        : std::hypot(toHalfway.x, toHalfway.y);
            bulge = std::max(bulge, distance);
        }
    }
    std::array<double, 4> rectangle{std::numeric_limits<double>::infinity(),
                                    std::numeric_limits<double>::infinity(),
                                    -std::numeric_limits<double>::infinity(),
                                    -std::numeric_limits<double>::infinity()};
    for (int index = 0; index < side * side; ++index) {
        const Point<double> point = pointAt(index);
        rectangle[0] = std::min(rectangle[0], point.x);
        rectangle[1] = std::min(rectangle[1], point.y);
        rectangle[2] = std::max(rectangle[2], point.x);
        rectangle[3] = std::max(rectangle[3], point.y);
    }
    return {rectangle[0] - bulge, rectangle[1] - bulge, rectangle[2] + bulge, rectangle[3] + bulge};
}

std::shared_ptr<const CRSWorldCoordinateHelper> getRegisteredProjection(const std::string& name) {
    std::scoped_lock lock(registryMutex);
    const auto it = registry().find(name);
    return it == registry().end() ? nullptr : it->second;
}

void addProjection(CRS crs) {
    if (crs.name.find(':') == std::string::npos) {
        throw std::invalid_argument("The projection name \"" + crs.name +
                                    "\" needs a colon, like \"EPSG:2193\" or \"custom:equal-earth\": names without one "
                                    "are reserved for built-in projections.");
    }
    std::scoped_lock lock(registryMutex);
    if (registry().contains(crs.name)) {
        throw std::invalid_argument("A projection called \"" + crs.name + "\" already exists.");
    }
    if (!(crs.tileMatrixSet.extentAtZoom0 > 0)) {
        throw std::invalid_argument("The projection \"" + crs.name +
                                    "\" needs a tileMatrixSet.extentAtZoom0 greater than zero.");
    }
    if (!crs.forward || !crs.inverse) {
        throw std::invalid_argument("The projection \"" + crs.name + "\" needs both forward and inverse.");
    }
    auto name = crs.name;
    registry().emplace(std::move(name),
                       std::make_shared<const CRSWorldCoordinateHelper>(std::make_shared<const CRS>(std::move(crs))));
}

void removeProjection(const std::string& name) {
    if (name.find(':') == std::string::npos) {
        throw std::invalid_argument("The projection name \"" + name +
                                    "\" has no colon, so it is reserved for built-in projections, which cannot be "
                                    "removed.");
    }
    std::scoped_lock lock(registryMutex);
    registry().erase(name);
}

} // namespace mln
