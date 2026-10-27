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
import type { Plugin, Template } from './templates/values.ts';

const root = join(import.meta.dirname, '..');
const repository = join(root, '..');
const backendNames = ['opengl', 'vulkan', 'metal'];
const manifestKeys = ['description', 'backends'];
// Build outputs in the generated directories, which are neither generated nor stale.
const ignored = new Set(['build', '.cxx', '.gradle']);

// The platforms. plugins/<name> gets the files of the templates in templates/<name> for the plugins that have a backend
// of the platform.
const platforms = [
  { name: 'android', backends: ['opengl', 'vulkan'] },
  { name: 'ios', backends: ['opengl', 'metal'] },
];

type Files = Map<string, string>;

interface Manifest {
  description?: string;
  backends?: string[];
}

function fail(message: string): never {
  console.error(`error: ${message}`);
  process.exit(1);
}

function readPlugin(id: string, directory: string): Plugin {
  const where = `plugins/plugins/${id}/plugin.json`;
  if (!/^[a-z][a-z0-9]*(-[a-z0-9]+)*$/.test(id)) fail(`${where}: the directory must be a kebab-case id`);
  const manifest: Manifest = JSON.parse(readFileSync(join(directory, 'plugin.json'), 'utf8'));
  for (const key of Object.keys(manifest)) {
    if (!manifestKeys.includes(key)) fail(`${where}: unknown key ${key}`);
  }
  const backends = manifest.backends ?? backendNames;
  if (!backends.length || !backends.every((backend) => backendNames.includes(backend))) {
    fail(`${where}: backends must be a non-empty subset of ${backendNames.join(', ')}`);
  }
  // The rest follows from the directory layout.
  return {
    id,
    backends,
    sources: readdirSync(join(directory, 'shared/cpp')).filter((name) => name.endsWith('.cpp')).sort(),
    includeDirectories: ['shared/include', 'shared/cpp', 'vendor'].filter((name) => existsSync(join(directory, name))),
  };
}

function readPlugins(): Plugin[] {
  const directory = join(root, 'plugins');
  return readdirSync(directory, { withFileTypes: true })
    .filter((entry) => entry.isDirectory() && existsSync(join(directory, entry.name, 'plugin.json')))
    .map((entry) => readPlugin(entry.name, join(directory, entry.name)))
    .sort((a, b) => a.id.localeCompare(b.id));
}

async function loadTemplates(platform: string): Promise<Template[]> {
  const directory = join(import.meta.dirname, 'templates', platform);
  const names = (await Array.fromAsync(glob('*.ts', { cwd: directory }))).sort();
  return Promise.all(names.map((name) => import(pathToFileURL(join(directory, name)).href)));
}

// The files of a platform, by their path in plugins/<name>.
async function render(platform: (typeof platforms)[number], plugins: Plugin[]): Promise<Files> {
  const supported = plugins.filter((plugin) => plugin.backends.some((backend) => platform.backends.includes(backend)));
  const files: Files = new Map();
  for (const template of await loadTemplates(platform.name)) {
    if (template.scope === 'plugins') {
      files.set(template.path(supported), template.default(supported));
    } else {
      for (const plugin of supported) files.set(template.path(plugin), template.default(plugin));
    }
  }
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
const stale: string[] = [];
for (const platform of platforms) {
  const directory = `plugins/${platform.name}`;
  const changed = sync(join(repository, directory), await render(platform, plugins), !!values.check);
  stale.push(...changed.map((path) => `  ${directory}/${path}`));
}
if (values.check) {
  if (stale.length) fail(`The generated plugin wrappers are out of date. Run node plugins/tools/generate.ts and commit the result:\n${stale.join('\n')}`);
  console.log('The generated plugin wrappers are up to date.');
} else {
  console.log(`Generated the wrappers of ${plugins.length} plugins (${stale.length} files changed).`);
  for (const line of stale) console.log(line);
}
