use crate::ffi::mbgl::util::{RunLoop, RunLoop_Type};
use crate::ffi::mbgl::Map;
use autocxx::prelude::*;

include_cpp! {
    #include "mbgl/util/run_loop.hpp"
    #include "mbgl/map/map_options.hpp"
    #include "mbgl/util/image.hpp"
    #include "mbgl/util/run_loop.hpp"

    #include "mbgl/gfx/backend.hpp"
    #include "mbgl/gfx/headless_frontend.hpp"
    #include "mbgl/style/style.hpp"

    safety!(unsafe)
    generate!("mbgl::util::RunLoop")
    // generate!("mbgl::HeadlessFrontend")
    generate!("mbgl::Map")
}

fn main() {
    let _ = RunLoop::new(RunLoop_Type::Default);
    // Map::foo()
}
