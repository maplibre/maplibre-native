// Generates the platform wrappers of the plugins in plugins/plugins from their plugin.json.
//
//   node plugins/tools/generate.ts [--check]
//
// plugins/android/<id>: a Gradle library module that builds the plugin and registers it from Kotlin.
// plugins/ios/<id>: a Bazel library with an Objective-C class that registers the plugin. plugins/ios also has a
// registry that registers any plugin by its id.
//
// The outputs are committed. `--check` writes nothing and fails when they are out of date.
import { existsSync, mkdirSync, readdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { glob } from 'node:fs/promises';
import { join, relative, sep } from 'node:path';
import { pathToFileURL } from 'node:url';
import { parseArgs } from 'node:util';
import type { AndroidValues, Plugin, RegistryValues, Template } from './templates/values.ts';

const root = join(import.meta.dirname, '..');
const repository = join(root, '..');
const backendNames = ['opengl', 'vulkan', 'metal'];
const ignored = new Set(['build', '.cxx', '.gradle']);

// The templates in templates/<directory>: one module per generated file, see templates/values.ts.
async function loadTemplates<Values>(directory: string): Promise<Template<Values>[]> {
  const base = join(import.meta.dirname, 'templates', directory);
  const names = (await Array.fromAsync(glob('*.ts', { cwd: base }))).sort();
  return Promise.all(names.map((name) => import(pathToFileURL(join(base, name)).href)));
}

const androidTemplates = await loadTemplates<AndroidValues>('android');
const iosTemplates = await loadTemplates<Plugin>('ios');
const registryTemplates = await loadTemplates<RegistryValues>('ios/registry');

interface Manifest {
  description?: string;
  backends?: string[];
}

interface PluginSource extends Plugin {
  directory: string;
}

type Files = Map<string, string>;

function fail(message: string): never {
  console.error(`error: ${message}`);
  process.exit(1);
}

function readPlugin(id: string, directory: string): PluginSource {
  const where = `plugins/plugins/${id}/plugin.json`;
  if (!/^[a-z][a-z0-9]*(-[a-z0-9]+)*$/.test(id)) fail(`${where}: the directory must be a kebab-case id`);
  const manifest: Manifest = JSON.parse(readFileSync(join(directory, 'plugin.json'), 'utf8'));
  for (const key of Object.keys(manifest)) {
    if (!['description', 'backends'].includes(key)) fail(`${where}: unknown key ${key}`);
  }
  const backends = manifest.backends ?? backendNames;
  if (!backends.length || !backends.every((backend) => backendNames.includes(backend))) {
    fail(`${where}: backends must be a non-empty subset of ${backendNames.join(', ')}`);
  }
  return { id, directory, backends };
}

function readPlugins(): PluginSource[] {
  const directory = join(root, 'plugins');
  return readdirSync(directory, { withFileTypes: true })
    .filter((entry) => entry.isDirectory() && existsSync(join(directory, entry.name, 'plugin.json')))
    .map((entry) => readPlugin(entry.name, join(directory, entry.name)))
    .sort((a, b) => a.id.localeCompare(b.id));
}

// The wrappers build a plugin from its directory layout: shared/cpp/*.cpp, shared/include/<id>.hpp and a register
// function named mln_<id>_register.
function sources(plugin: PluginSource): string[] {
  return readdirSync(join(plugin.directory, 'shared/cpp')).filter((name) => name.endsWith('.cpp')).sort();
}

function includeDirectories(plugin: PluginSource): string[] {
  return ['shared/include', 'shared/cpp', 'vendor'].filter((name) => existsSync(join(plugin.directory, name)));
}

function androidFiles(plugins: PluginSource[]): Files {
  const ndk = readFileSync(join(repository, 'platform/android/buildSrc/src/main/kotlin/Versions.kt'), 'utf8')
    .match(/ndkVersion\s*=\s*"([^"]+)"/)?.[1];
  if (!ndk) fail('Cannot find ndkVersion in platform/android/buildSrc/src/main/kotlin/Versions.kt');
  const files: Files = new Map([['.gitignore', '/*/build/\n/*/.cxx/\n/*/.gradle/\n']]);
  for (const plugin of plugins.filter((p) => p.backends.some((b) => b === 'opengl' || b === 'vulkan'))) {
    const values: AndroidValues = {
      plugin,
      ndkVersion: ndk,
      sources: sources(plugin),
      includeDirectories: includeDirectories(plugin),
    };
    for (const template of androidTemplates) files.set(template.path(values), template.default(values));
  }
  return files;
}

function iosFiles(plugins: PluginSource[]): Files {
  const files: Files = new Map();
  const supported = plugins.filter((p) => p.backends.some((b) => b === 'opengl' || b === 'metal'));
  for (const plugin of supported) {
    for (const template of iosTemplates) files.set(template.path(plugin), template.default(plugin));
  }
  const registry: RegistryValues = { plugins: supported };
  for (const template of registryTemplates) files.set(template.path(registry), template.default(registry));
  return files;
}

function filesUnder(directory: string, base = directory): string[] {
  if (!existsSync(directory)) return [];
  return readdirSync(directory, { withFileTypes: true }).flatMap((entry) => {
    const path = join(directory, entry.name);
    if (entry.isDirectory()) return ignored.has(entry.name) ? [] : filesUnder(path, base);
    return [relative(base, path).split(sep).join('/')];
  });
}

function removeEmptyDirectories(directory: string) {
  for (const entry of readdirSync(directory, { withFileTypes: true })) {
    if (!entry.isDirectory() || ignored.has(entry.name)) continue;
    const path = join(directory, entry.name);
    removeEmptyDirectories(path);
    if (!readdirSync(path).length) rmSync(path, { recursive: true });
  }
}

// Makes `directory` contain exactly `files` and returns what changed, or would change with `check`.
function sync(directory: string, files: Files, check: boolean): string[] {
  const changed: string[] = [];
  for (const [path, content] of [...files].sort(([a], [b]) => a.localeCompare(b))) {
    const target = join(directory, path);
    if (existsSync(target) && readFileSync(target, 'utf8') === content) continue;
    changed.push(path);
    if (!check) {
      mkdirSync(join(target, '..'), { recursive: true });
      writeFileSync(target, content);
    }
  }
  for (const path of filesUnder(directory).filter((path) => !files.has(path)).sort()) {
    changed.push(`${path} (not generated)`);
    if (!check) rmSync(join(directory, path));
  }
  if (!check && existsSync(directory)) removeEmptyDirectories(directory);
  return changed;
}

const { values } = parseArgs({ options: { check: { type: 'boolean' } } });
const plugins = readPlugins();
const outputs: [string, Files][] = [
  ['plugins/android', androidFiles(plugins)],
  ['plugins/ios', iosFiles(plugins)],
];

const stale = outputs.flatMap(([directory, files]) =>
  sync(join(repository, directory), files, !!values.check).map((path) => `  ${directory}/${path}`),
);
if (values.check) {
  if (stale.length) fail(`The generated plugin wrappers are out of date. Run node plugins/tools/generate.ts and commit the result:\n${stale.join('\n')}`);
  console.log('The generated plugin wrappers are up to date.');
} else {
  console.log(`Generated the wrappers of ${plugins.length} plugins (${stale.length} files changed).`);
  for (const line of stale) console.log(line);
}
