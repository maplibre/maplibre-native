#pragma once

#include <mbgl/map/camera.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/size.hpp>

#include <memory>
#include <string>
#include <vector>

class TestOperationSerializer;

class TestWriter final {
public:
    TestWriter();
    ~TestWriter();

    TestWriter& withCameraOptions(const mln::CameraOptions&);
    TestWriter& withStyle(const mln::style::Style&);
    TestWriter& withInitialSize(const mln::Size&);

    bool write(const std::string& dir) const;

private:
    std::string serialize() const;

    std::vector<std::unique_ptr<TestOperationSerializer>> operations;
    std::unique_ptr<TestOperationSerializer> initialSize;
};
