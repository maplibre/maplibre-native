use std::collections::HashSet;
use std::path::{Path, PathBuf};

/// Given the contents of mbgl-core-deps.txt and the base directory where static libraries are found,
/// parse the whitespace-separated tokens and return a vector of Cargo linker instructions.
pub fn parse_deps(deps_contents: &str, static_lib_base: &Path) -> Vec<String> {
    let mut instructions = Vec::new();
    let mut added_search_paths = HashSet::new();
    let tokens: Vec<&str> = deps_contents.split_whitespace().collect();
    let mut token_iter = tokens.iter().peekable();

    while let Some(&token) = token_iter.next() {
        if token == "-framework" {
            if let Some(&framework) = token_iter.next() {
                instructions.push(format!("cargo:rustc-link-lib=framework={}", framework));
            } else {
                panic!("Expected a framework name after '-framework'");
            }
        } else if token.starts_with("-l") {
            let libname = &token[2..];
            instructions.push(format!("cargo:rustc-link-lib={}", libname));
        } else if token.ends_with(".a") {
            let lib_path = Path::new(token);
            let file_stem = lib_path.file_stem().expect("Library file has no stem");
            let file_stem = file_stem.to_str().expect("Library file stem is not valid UTF-8");
            let lib_name = file_stem.strip_prefix("lib").unwrap_or(file_stem);

            let search_dir = match lib_path.parent() {
                Some(parent) if !parent.as_os_str().is_empty() => static_lib_base.join(parent),
                _ => static_lib_base.to_path_buf(),
            };
            if added_search_paths.insert(search_dir.clone()) {
                instructions.push(format!("cargo:rustc-link-search=native={}", search_dir.display()));
            }
            instructions.push(format!("cargo:rustc-link-lib=static={}", lib_name));
        } else {
            instructions.push(format!("cargo:rustc-link-arg={}", token));
        }
    }
    instructions
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::PathBuf;

    #[test]
    fn test_parse_deps() {
        // This sample mimics a deps file with:
        // - a dynamic library flag (-lsqlite3)
        // - a static library file (libmbgl-core.a)
        // - a framework flag (-framework AppKit)
        // - an extra linker argument ("some_arg")
        let deps_content = "-lsqlite3 libmbgl-core.a -framework AppKit some_arg";
        // Assume that the static libraries are found in "/build_dir/build"
        let base_dir = PathBuf::from("/build_dir/build");
        let instructions = parse_deps(deps_content, &base_dir);
        let expected = vec![
            "cargo:rustc-link-lib=sqlite3".to_string(),
            format!("cargo:rustc-link-search=native={}", base_dir.display()),
            "cargo:rustc-link-lib=static=mbgl-core".to_string(),
            "cargo:rustc-link-lib=framework=AppKit".to_string(),
            "cargo:rustc-link-arg=some_arg".to_string(),
        ];
        assert_eq!(instructions, expected);
    }
}
