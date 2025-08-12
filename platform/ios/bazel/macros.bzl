load("@darwin_config//:config.bzl", "API_KEY")

def info_plist(name, base_info_plist, out, **kwargs):
    native.genrule(
        name = name,
        srcs = [
            base_info_plist,
        ],
        outs = [
            out,
        ],
        cmd = ("""
        cp $(location {}) $@

        token=\"""" + API_KEY + """\"
        plutil -replace MLNApiKey -string $$token $@
    """).format(base_info_plist),
        **kwargs
    )

def _env_info_plist_impl(ctx):
    in_file = ctx.file.input
    out_file = ctx.outputs.output

    ctx.actions.run_shell(
        inputs = [in_file],
        outputs = [out_file],
        arguments = [in_file.path, out_file.path, ctx.attr.var],
        use_default_shell_env = True,
        command = """
            cp $1 $2

            if plutil -p $2 | grep -q "<key>$3<key>"; then
                # skip if there is a key present (let the user manually set a value)
                #plutil -replace $3 -string ${!3} $2
            else
                plutil -insert $3 -string ${!3} $2
            fi
        """,
    )

env_info_plist = rule(
    implementation = _env_info_plist_impl,
    attrs = {
        "input": attr.label(
            mandatory = True,
            allow_single_file = True,
        ),
        "output": attr.output(mandatory = True),
        "var": attr.string(mandatory = True),
    },
)
