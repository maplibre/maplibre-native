load("@darwin_config//:config.bzl", "API_KEY")

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

        token=\"""" + API_KEY + """\"
        plutil -replace MLNApiKey -string $$token $@
    """).format(base_info_plist),
        **kwargs
    )
