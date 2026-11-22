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

// A template module in templates/<platform>: where the file of a plugin's wrapper goes relative to plugins/<platform>,
// and what is in it.
export interface Template {
  path: (plugin: Plugin) => string;
  default: (plugin: Plugin) => string;
}
