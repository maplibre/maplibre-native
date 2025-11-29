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
    args = ctx.actions.args()
    args.add(ctx.file.input)
    args.add(ctx.outputs.output)
    args.add_all(ctx.attr.vars)

    ctx.actions.run_shell(
        inputs = [ctx.file.input],
        outputs = [ctx.outputs.output],
        arguments = [args],
        use_default_shell_env = True,
        command = """
            in=$1
            out=$2
            shift 2
            vars=(${@})

            cp $in $out

            for var in "${vars[@]}"; do
                if [ -n "${!var}" ]; then
                    if plutil -p $out | grep -q "<key>$var<key>"; then
                        # skip if there is a key present (let the user manually override)
                        #plutil -replace $var -string ${!var} $out
                        echo "User $var key detected. Update ignored"
                    else
                        plutil -insert $var -string ${!var} $out
                    fi
                fi
            done
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
        "vars": attr.string_list(mandatory = True),
    },
    doc = "Pull environment variables and add them to the provided plist",
)
