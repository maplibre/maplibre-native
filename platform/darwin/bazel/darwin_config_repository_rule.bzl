def _impl(ctx):
    print("Reading Darwin config")
    config_dir = ctx.workspace_root.get_child("platform", "darwin", "bazel")
    config = config_dir.get_child("confib.bzl")
    example_config = config_dir.get_child("example_config.bzl")

    ctx.watch(config)
    ctx.watch(example_config)

    ctx.file("BUILD.bazel", "")
    if config.exists:
        ctx.file("config.bzl", ctx.read(config))
    else:
        ctx.file("config.bzl", ctx.read(example_config))

darwin_config = repository_rule(
    implementation = _impl,
    local = True,
)
