// A target that needs a build with plugins and the Metal renderer.
const only = (condition: string) => `select({
        "${condition}": [],
        "//conditions:default": ["@platforms//:incompatible"],
    })`;

export const metalWithPlugins = `    target_compatible_with = ${only('//:plugins_enabled')} + ${only('//:metal_renderer')},`;
