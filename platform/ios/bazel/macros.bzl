load("//platform/ios/bazel:config.bzl", "API_KEY", "SEM_VER")

def info_plist(name, base_info_plist, out, **kwargs):
    native.genrule(
        name = name,
        srcs = [
            base_info_plist,
            "//:git_hash",
        ],
        outs = [
            out,
        ],
        cmd = ("""
        cp $(location {}) $@
        plutil -replace MLNCommitHash -string $$(cat $(location //:git_hash)) $@
        
        sem_version=\"""" + SEM_VER + """\"
        token=\"""" + API_KEY + """\"
        plutil -replace MLNSemanticVersionString -string $$sem_version $@
        plutil -replace MLNApiKey -string $$token $@
    """).format(base_info_plist),
        **kwargs
    )
