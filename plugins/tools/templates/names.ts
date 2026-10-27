// The names in the code of a plugin follow from its id, the name of its directory in plugins/plugins.

export const snake = (id: string) => id.replaceAll('-', '_');

export const pascal = (id: string) => id.split('-').map((part) => part[0].toUpperCase() + part.slice(1)).join('');

export const displayName = (id: string) => id[0].toUpperCase() + id.slice(1).replaceAll('-', ' ');

/** The header of the plugin, as included, and the function in it that registers the plugin. */
export const header = (id: string) => `${snake(id)}.hpp`;
export const registerFunction = (id: string) => `mln_${snake(id)}_register`;

export const androidNamespace = (id: string) => `org.maplibre.plugins.${id.replaceAll('-', '')}`;
export const androidClass = (id: string) => `${pascal(id)}Plugin`;

export const iosClass = (id: string) => `MLN${pascal(id)}Plugin`;
