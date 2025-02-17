use std::collections::HashSet;
use std::path::Path;

/// Parses the contents of mbgl-core-deps.txt and returns Cargo linker instructions.
///
/// # Arguments
///
/// * `deps_contents` - The contents of the dependency file as a string.
/// * `static_lib_base` - The base directory where the static libraries reside.
pub fn parse_deps(deps_contents: &str, static_lib_base: &Path, include_args: bool) -> Vec<String> {
    let mut instructions = Vec::new();
    let mut added_search_paths = HashSet::new();
    let mut token_iter = deps_contents.split_whitespace().peekable();

    // FIXME: For debugging - need to figure out why tests do not compile
    // instructions.push(format!(
    //     "cargo::warning=debugging cmake string = {deps_contents}"
    // ));

    while let Some(token) = token_iter.next() {
        if token == "-framework" {
            if let Some(framework) = token_iter.next() {
                instructions.push(format!("cargo:rustc-link-lib=framework={framework}"));
            } else {
                panic!("Expected a framework name after '-framework'");
            }
        } else if let Some(libname) = token.strip_prefix("-l") {
            instructions.push(format!("cargo:rustc-link-lib={libname}"));
        } else if token.ends_with(".a") {
            let lib_path = Path::new(token);
            let file_stem = lib_path.file_stem().expect("Library file has no stem");
            let file_stem = file_stem
                .to_str()
                .expect("Library file stem is not valid UTF-8");
            let lib_name = file_stem.strip_prefix("lib").unwrap_or(file_stem);

            let search_dir = match lib_path.parent() {
                Some(parent) if !parent.as_os_str().is_empty() => static_lib_base.join(parent),
                _ => static_lib_base.to_path_buf(),
            };
            if added_search_paths.insert(search_dir.clone()) {
                instructions.push(format!(
                    "cargo:rustc-link-search=native={}",
                    search_dir.to_str().expect("Search path is not valid UTF-8")
                ));
            }
            instructions.push(format!("cargo:rustc-link-lib=static={lib_name}"));
        } else if include_args {
            // FIXME: should not use args by default, maybe with a feature flag?
            instructions.push(format!("cargo:rustc-link-arg={token}"));
        } else {
            instructions.push(format!("cargo::warning=Ignoring cmake token = {token}"));
        }
    }
    instructions
}

#[cfg(test)]
mod tests {
    use std::path::PathBuf;

    use super::*;

    #[test]
    fn test_parse_deps() {
        // Simulate a deps file with:
        //   - "-lsqlite3" (link sqlite3)
        //   - "libmbgl-core.a" (a static library with no parent directory)
        //   - "-framework AppKit"
        //   - "some_arg" (an extra linker argument)
        let deps_content = "-lsqlite3 libmbgl-core.a -framework AppKit some_arg";
        let base_dir = PathBuf::from("/build_dir/build");
        let instructions = parse_deps(deps_content, &base_dir, true);
        let expected = [
            "cargo:rustc-link-lib=sqlite3",
            "cargo:rustc-link-search=native=/build_dir/build",
            "cargo:rustc-link-lib=static=mbgl-core",
            "cargo:rustc-link-lib=framework=AppKit",
            "cargo:rustc-link-arg=some_arg",
        ];
        assert_eq!(instructions, expected);
    }

    #[test]
    fn long_parse() {
        let v = "-ffunction-sections -fdata-sections -fPIC -m64   libmbgl-core.a  libmbgl-vendor-parsedate.a  libmbgl-vendor-csscolorparser.a  vendor/glslang/glslang/libglslang.a  vendor/glslang/SPIRV/libSPIRV.a  vendor/glslang/glslang/libMachineIndependent.a  vendor/glslang/glslang/OSDependent/Unix/libOSDependent.a  vendor/glslang/glslang/libGenericCodeGen.a  vendor/glslang/glslang/libglslang-default-resource-limits.a  /usr/lib/x86_64-linux-gnu/libcurl.so  /usr/lib/x86_64-linux-gnu/libjpeg.so  -luv  -lpthread  -lrt  /usr/lib/x86_64-linux-gnu/libX11.so  /usr/lib/x86_64-linux-gnu/libXext.so  -lwebp  /usr/lib/x86_64-linux-gnu/libicui18n.so  /usr/lib/x86_64-linux-gnu/libicuuc.so  -ldl  /usr/lib/x86_64-linux-gnu/libpng.so  /usr/lib/x86_64-linux-gnu/libz.so  libmbgl-vendor-nunicode.a  libmbgl-vendor-sqlite.a  -lgcc  -lgcc_s  -lc  -lgcc  -lgcc_s  -lstdc++  -lm  -lgcc_s  -lgcc  -lc  -lgcc_s  -lgcc";
        let base_dir = PathBuf::from("/build_dir/build");
        let instructions = parse_deps(v, &base_dir, true);
        let expected = [
            "-ffunction-sections",
            "-fdata-sections",
            "-fPIC",
            "-m64",
            "cargo:rustc-link-search=native=mbgl-core",
            "cargo:rustc-link-search=native=mbgl-vendor-parsedate",
            "cargo:rustc-link-search=native=mbgl-vendor-csscolorparser",
            "cargo:rustc-link-search=native=glslang",
            "cargo:rustc-link-search=native=SPIRV",
            "cargo:rustc-link-search=native=MachineIndependent",
            "cargo:rustc-link-search=native=OSDependent",
            "cargo:rustc-link-search=native=GenericCodeGen",
            "cargo:rustc-link-search=native=glslang-default-resource-limits",
            "/usr/lib/x86_64-linux-gnu/libcurl.so",
            "/usr/lib/x86_64-linux-gnu/libjpeg.so",
            "cargo:rustc-link-lib=uv",
            "cargo:rustc-link-lib=pthread",
            "cargo:rustc-link-lib=rt",
            "/usr/lib/x86_64-linux-gnu/libX11.so",
            "/usr/lib/x86_64-linux-gnu/libXext.so",
            "cargo:rustc-link-lib=webp",
            "/usr/lib/x86_64-linux-gnu/libicui18n.so",
            "/usr/lib/x86_64-linux-gnu/libicuuc.so",
            "cargo:rustc-link-lib=dl",
            "/usr/lib/x86_64-linux-gnu/libpng.so",
            "/usr/lib/x86_64-linux-gnu/libz.so",
            "cargo:rustc-link-search=native=mbgl-vendor-nunicode",
            "cargo:rustc-link-search=native=mbgl-vendor-sqlite",
            "cargo:rustc-link-lib=gcc",
            "cargo:rustc-link-lib=gcc_s",
            "cargo:rustc-link-lib=c",
            "cargo:rustc-link-lib=gcc",
            "cargo:rustc-link-lib=gcc_s",
            "cargo:rustc-link-lib=stdc++",
            "cargo:rustc-link-lib=m",
            "cargo:rustc-link-lib=gcc_s",
            "cargo:rustc-link-lib=gcc",
            "cargo:rustc-link-lib=c",
            "cargo:rustc-link-lib=gcc_s",
            "cargo:rustc-link-lib=gcc",
        ];

        assert_eq!(instructions, expected);
    }
}
