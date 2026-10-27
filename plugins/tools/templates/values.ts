// What generate.ts passes to the templates of each kind.

/** A plugin in plugins/plugins. */
export interface Plugin {
  id: string;
  backends: string[];
}

export interface AndroidValues {
  plugin: Plugin;
  ndkVersion: string;
  /** The files in shared/cpp that are built. */
  sources: string[];
  /** The directories of the plugin that are on the include path. */
  includeDirectories: string[];
}

export interface RegistryValues {
  plugins: Plugin[];
}

// A template module in templates/: the path of its file relative to plugins/<platform>, and its content.
export interface Template<Values> {
  path: (values: Values) => string;
  default: (values: Values) => string;
}
