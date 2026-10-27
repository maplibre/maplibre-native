// What generate.ts passes to the templates of each kind.

export interface AndroidValues {
  id: string;
  displayName: string;
  namespace: string;
  product: string;
  className: string;
  qualifiedClassName: string;
  ndkVersion: string;
  pluginRoot: string;
  sources: string;
  includeDirectories: string;
  header: string;
  register: string;
  jniClass: string;
  backends: string;
  opengl: string;
  vulkan: string;
}

export interface IosValues {
  id: string;
  product: string;
  className: string;
  swiftName: string;
  errorDomain: string;
  displayName: string;
  header: string;
  register: string;
  compatible: string;
  demo: string;
}

export interface RegistryValues {
  imports: string;
  identifiers: string;
  cases: string;
  deps: string;
  demos: string;
}

// A template module in templates/: the path of its file relative to plugins/<platform>, and its content.
export interface Template<Values> {
  path: (values: Values) => string;
  default: (values: Values) => string;
}
