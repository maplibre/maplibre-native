// Circular corner arcs matching the native geometry convention. Work at double
// precision, retain float points for tessellation, then quantize GPU positions
// to the native 1/128 tile unit grid. Holes follow the same winding rule.
Polygon rounded(Polygon polygon, double distance) {
    auto sub = [](Point a, Point b) {
        return Point{a[0] - b[0], a[1] - b[1]};
    };
    auto add = [](Point a, Point b) {
        return Point{a[0] + b[0], a[1] + b[1]};
    };
    auto mul = [](Point a, double b) {
        return Point{a[0] * b, a[1] * b};
    };
    auto cross = [](Point a, Point b) {
        return a[0] * b[1] - a[1] * b[0];
    };
    auto length = [](Point a) {
        return std::sqrt(a[0] * a[0] + a[1] * a[1]);
    };
    auto fp = [](Point a) {
        return Point{float(a[0]), float(a[1])};
    };
    for (auto& ring : polygon) {
        if (ring.size() < 4) continue;
        Ring output;
        const size_t count = ring.size() - 1;
        for (size_t i = 0; i < count; ++i) {
            const auto a = ring[(i + count - 1) % count], p = ring[i], b = ring[(i + 1) % count];
            const double len1 = length(sub(p, a)), len2 = length(sub(b, p));
            if (!len1 || !len2) {
                output.push_back(fp(p));
                continue;
            }
            const auto e1 = mul(sub(p, a), 1 / len1), e2 = mul(sub(b, p), 1 / len2);
            const double d = std::min({distance, len1 * 0.2, len2 * 0.2});
            const auto start = sub(p, mul(e1, d)), end = add(p, mul(e2, d));
            Point n1{-e1[1], e1[0]}, n2{-e2[1], e2[0]};
            if (cross(e1, e2) < 0) {
                n1 = mul(n1, -1);
                n2 = mul(n2, -1);
            }
            const double determinant = cross(n1, n2);
            if (std::abs(determinant) < std::sin(5.0 * 3.14159265358979323846 / 180.0)) {
                output.push_back(fp(p));
                continue;
            }
            const auto center = add(start, mul(n1, cross(sub(end, start), n2) / determinant));
            const auto va = sub(start, center), vb = sub(end, center);
            const double radius = length(va), angle = std::atan2(va[1], va[0]);
            const double arc = std::atan2(cross(va, vb), va[0] * vb[0] + va[1] * vb[1]);
            output.push_back(fp(start));
            for (int k = 1; k <= 3; ++k) {
                const double t = angle + arc * k / 4;
                output.push_back(fp({center[0] + std::cos(t) * radius, center[1] + std::sin(t) * radius}));
            }
            output.push_back(fp(end));
        }
        if (!output.empty()) output.push_back(output.front());
        ring = std::move(output);
    }
    return polygon;
}
Point quantize(Point p) {
    return {std::floor(p[0] * 128.0) / 128.0, std::floor(p[1] * 128.0) / 128.0};
}
