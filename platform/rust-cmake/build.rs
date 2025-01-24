fn main() {
    // Compile maplibre native using cmake
    // cmake \
    //   -B build \
    //   -GNinja \
    //   -DCMAKE_C_COMPILER=clang \
    //   -DCMAKE_CXX_COMPILER=clang++ \
    //   -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    //   -DMLN_WITH_CLANG_TIDY=OFF \
    //   -DMLN_WITH_COVERAGE=OFF \
    //   -DMLN_DRAWABLE_RENDERER=ON \
    //   -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
    //   -DMLN_USE_RUST=ON
    let mut config = cmake::Config::new("../..");
    config.define("CMAKE_C_COMPILER", "clang");
    config.define("CMAKE_CXX_COMPILER", "clang++");
    config.define("CMAKE_BUILD_TYPE", "RelWithDebInfo");
    config.define("MLN_WITH_CLANG_TIDY", "OFF");
    config.define("MLN_WITH_COVERAGE", "OFF");
    config.define("MLN_DRAWABLE_RENDERER", "ON");
    config.define("CMAKE_BUILD_WITH_INSTALL_RPATH", "ON");
    config.define("MLN_USE_RUST", "ON");
    config.build();
}
