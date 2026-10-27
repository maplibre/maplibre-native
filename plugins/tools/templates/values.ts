// What generate.ts passes to the templates.

/** A plugin in plugins/plugins. */
export interface Plugin {
  id: string;
  backends: string[];
  /** The files in shared/cpp that are built. */
  sources: string[];
  /** The directories of the plugin that are on the include path. */
  includeDirectories: string[];
}

// A template module in templates/<platform>: where its file goes relative to plugins/<platform>, and what is in it.
// The file is a wrapper of each plugin, or with `scope = 'plugins'` one file for all the plugins of the platform.
interface PluginTemplate {
  scope?: 'plugin';
  path: (plugin: Plugin) => string;
  default: (plugin: Plugin) => string;
}

interface PluginsTemplate {
  scope: 'plugins';
  path: (plugins: Plugin[]) => string;
  default: (plugins: Plugin[]) => string;
}

export type Template = PluginTemplate | PluginsTemplate;
