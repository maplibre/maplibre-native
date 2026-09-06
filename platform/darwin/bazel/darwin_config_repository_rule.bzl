def _impl(ctx):
    config_dir = ctx.workspace_root.get_child("platform", "darwin", "bazel")
    config = config_dir.get_child("config.bzl")
    example_config = config_dir.get_child("example_config.bzl")

    ctx.watch(config)
    ctx.watch(example_config)

    ctx.file("BUILD.bazel", "")
    if config.exists:
        ctx.file("config.bzl", ctx.read(config))
    elif example_config.exists:
        ctx.file("config.bzl", ctx.read(example_config))
    else:
        # When MapLibre is a local_path_override dependency, workspace_root is
        # the consuming application. Resolve the fallback in MapLibre's own
        # repository instead of assuming MapLibre is the root module.
        ctx.file("config.bzl", ctx.read(ctx.attr.example_config))

darwin_config = repository_rule(
    implementation = _impl,
    local = True,
    attrs = {
        "example_config": attr.label(
            default = Label("//platform/darwin:bazel/example_config.bzl"),
            allow_single_file = True,
        ),
    },
)
