#pragma once

#include <mln/util/client_options.hpp>
#include <mln/util/image.hpp>
#include <mln/util/geo.hpp>
#include <mln/annotation/annotation.hpp>
#include <mln/style/image.hpp>

#include <exception>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace mln {

struct CameraOptions;
class ClientOptions;
class LatLngBounds;
class ResourceOptions;

namespace style {
class Style;
} // namespace style

class MapSnapshotterObserver {
public:
    virtual ~MapSnapshotterObserver() = default;

    static MapSnapshotterObserver& nullObserver();
    virtual void onDidFailLoadingStyle(const std::string&) {}
    virtual void onDidFinishLoadingStyle() {}
    virtual void onStyleImageMissing(const std::string&) {}
};

class MapSnapshotter {
public:
    MapSnapshotter(Size size,
                   float pixelRatio,
                   const ResourceOptions&,
                   const ClientOptions&,
                   MapSnapshotterObserver&,
                   std::optional<std::string> localFontFamily = std::nullopt);

    MapSnapshotter(Size size, float pixelRatio, const ResourceOptions&, const ClientOptions& = ClientOptions());

    ~MapSnapshotter();

    void setStyleURL(const std::string& styleURL);
    std::string getStyleURL() const;

    void setStyleJSON(const std::string& styleJSON);
    std::string getStyleJSON() const;

    void setSize(const Size&);
    Size getSize() const;

    void setCameraOptions(const CameraOptions&);
    CameraOptions getCameraOptions() const;

    void setRegion(const LatLngBounds&);
    LatLngBounds getRegion() const;

    void setPadding(const mln::EdgeInsets&);
    mln::EdgeInsets getPadding() const;

    void addAnnotationImage(std::unique_ptr<style::Image>);
    void addAnnotation(const Annotation&);

    style::Style& getStyle();
    const style::Style& getStyle() const;

    using PointForFn = std::function<ScreenCoordinate(const LatLng&)>;
    using LatLngForFn = std::function<LatLng(const ScreenCoordinate&)>;
    using Attributions = std::vector<std::string>;
    using Callback = std::function<void(std::exception_ptr, PremultipliedImage, Attributions, PointForFn, LatLngForFn)>;
    void snapshot(Callback);
    void cancel();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mln
