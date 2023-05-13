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
        cmd = """
        cp $(location {}) $@
        plutil -replace MLNCommitHash -string $$(cat $(location //:git_hash)) $@
        
        # TODO: move to file
        sem_version=0.0.0
        plutil -replace MLNSemanticVersionString -string $$sem_version $@
    """.format(base_info_plist),
        **kwargs
    )
