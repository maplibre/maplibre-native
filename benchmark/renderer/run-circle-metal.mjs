// Run from the repository root. Results are generated artifacts, not baselines.
import {execFileSync} from 'node:child_process';
import {createHash} from 'node:crypto';
import {mkdirSync, readFileSync, writeFileSync} from 'node:fs';
import {resolve} from 'node:path';
import {parseArgs} from 'node:util';

const {values} = parseArgs({options: {
  off: {type: 'string', default: 'build-circle-metal-off'},
  on: {type: 'string', default: 'build-circle-metal-on'},
  output: {type: 'string', default: 'benchmark/results/circle-metal'},
  frames: {type: 'string', default: '300'},
  warmups: {type: 'string', default: '30'},
  repeats: {type: 'string', default: '7'},
  smoke: {type: 'boolean', default: false},
}});
const run = (cmd, args) => execFileSync(cmd, args, {encoding: 'utf8', maxBuffer: 128 * 1024 * 1024}).trim();
const output = resolve(values.output);
mkdirSync(output, {recursive: true});
const builds = [values.off, values.on].map(resolvePath => resolve(resolvePath));
for (const [index, build] of builds.entries()) {
  const cache = readFileSync(`${build}/CMakeCache.txt`, 'utf8');
  for (const setting of ['CMAKE_BUILD_TYPE:STRING=Release', 'MLN_WITH_METAL:BOOL=ON', 'MLN_CREATE_AUTORELEASEPOOL:BOOL=ON']) {
    if (!cache.includes(setting)) throw new Error(`${build} must contain ${setting}`);
  }
  const pluginSetting = `MLN_WITH_PLUGINS:BOOL=${index ? 'ON' : 'OFF'}`;
  if (!cache.includes(pluginSetting)) throw new Error(`${build} must contain ${pluginSetting}`);
}
const displays = JSON.parse(run('system_profiler', ['SPDisplaysDataType', '-json']));
const repeats = values.smoke ? 1 : Number(values.repeats);
const frames = values.smoke ? 3 : Number(values.frames);
const warmups = values.smoke ? 2 : Number(values.warmups);
if (![repeats, frames].every(n => Number.isInteger(n) && n > 0) || !Number.isInteger(warmups) || warmups < 0)
  throw new Error('invalid sample counts');
const metadata = {
  revision: run('git', ['rev-parse', 'HEAD']),
  diff_sha256: createHash('sha256').update(run('git', ['diff', 'HEAD'])).digest('hex'),
  timestamp: new Date().toISOString(),
  cpu: run('sysctl', ['-n', 'machdep.cpu.brand_string']),
  os: run('sw_vers', ['-productVersion']),
  gpu: displays.SPDisplaysDataType.map(d => d.sppci_model),
  builds, frames, warmups, repeats,
  artifact_sha256: Object.fromEntries([
    `${builds[0]}/benchmark/mln-circle-benchmark`,
    `${builds[1]}/benchmark/mln-circle-benchmark`,
    `${builds[1]}/plugins/libmln-circle-layer.dylib`,
  ].map(path => [path, createHash('sha256').update(readFileSync(path)).digest('hex')])),
  background_workers: 4,
  size: [1024, 768], pixel_ratio: 1,
  timing: 'steady-clock wall time including Metal headless GPU wait; encoding_ms is the host CPU encoding counter',
};
writeFileSync(`${output}/metadata.json`, JSON.stringify(metadata, null, 2) + '\n');
const variants = [
  [builds[0], 'native', 'disabled-native'],
  [builds[1], 'native', 'enabled-native'],
  [builds[1], `${builds[1]}/plugins/libmln-circle-layer.dylib`, 'enabled-plugin'],
];
const workloads = values.smoke ? [[1000, 1, 'state', 'spread']] : [
  [1000, 1, 'constant', 'spread'], [10000, 1, 'constant', 'spread'], [100000, 1, 'constant', 'spread'],
  [10000, 1, 'camera', 'spread'], [10000, 1, 'feature', 'spread'], [10000, 1, 'composite', 'spread'],
  [10000, 1, 'state', 'spread'], [10000, 1, 'constant', 'dense'], [10000, 8, 'feature', 'spread'],
];
const measurements = new Map();
for (const [count, layers, paint, density] of workloads) {
  const workload = `${count}-${layers}-${paint}-${density}`;
  for (let repeat = 0; repeat < repeats; ++repeat) {
    // Rotate the first variant; reverse alternate rounds to balance pair ordering.
    let order = variants.map((_, i) => variants[(i + repeat) % variants.length]);
    if (repeat % 2) order.reverse();
    for (const [build, library, mode] of order) {
      process.stderr.write(`${workload} repeat ${repeat + 1}/${repeats}: ${mode}\n`);
      const raw = run(`${build}/benchmark/mln-circle-benchmark`,
        [library, String(count), String(layers), paint, String(frames), String(warmups), density]);
      writeFileSync(`${output}/${workload}-${repeat}-${mode}.jsonl`, raw + '\n');
      const grouped = new Map();
      for (const line of raw.split('\n')) {
        if (!line.startsWith('{')) continue;
        const row = JSON.parse(line);
        if (row.mode !== mode) throw new Error(`Expected ${mode}, got ${row.mode}`);
        if (!row.phase) continue;
        for (const field of ['wall_ms', 'encoding_ms']) {
          if (field === 'encoding_ms' && ['registration', 'startup_readback'].includes(row.phase)) continue;
          const key = `${row.phase}/${field}`;
          if (!grouped.has(key)) grouped.set(key, []);
          grouped.get(key).push(row[field]);
        }
      }
      for (const [phase, samples] of grouped) {
        const key = `${workload}/${phase}`;
        if (!measurements.has(key)) measurements.set(key, {});
        const byMode = measurements.get(key);
        (byMode[mode] ??= [])[repeat] = samples.reduce((a, b) => a + b, 0) / samples.length;
      }
    }
  }
}
const mean = a => a.reduce((x, y) => x + y, 0) / a.length;
// Process repeats, not individual frames, are the independent samples.
const interval = samples => {
  const avg = mean(samples);
  if (samples.length < 2) return [avg, null];
  const sd = Math.sqrt(samples.reduce((s, x) => s + (x - avg) ** 2, 0) / (samples.length - 1));
  const t = [0, 12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262][samples.length - 1];
  if (!t) return [avg, null]; // Do not claim a confidence interval for unsupported degrees of freedom.
  return [avg, t * sd / Math.sqrt(samples.length)];
};
const summary = [...measurements].map(([workload_phase, modes]) => ({
  workload_phase,
  modes: Object.fromEntries(Object.entries(modes).map(([mode, samples]) => [mode, {
    process_means_ms: samples, mean_ms: interval(samples)[0], ci95_half_width_ms: interval(samples)[1],
  }])),
  plugin_to_native: interval(modes['enabled-plugin'].map((x, i) => x / modes['enabled-native'][i])),
  enabled_to_disabled: interval(modes['enabled-native'].map((x, i) => x / modes['disabled-native'][i])),
}));
writeFileSync(`${output}/summary.json`, JSON.stringify(summary, null, 2) + '\n');
console.log(`Results: ${output}/summary.json`);
