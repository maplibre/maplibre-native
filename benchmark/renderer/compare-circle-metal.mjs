// Focused before/after experiment. Run after builds/tests, never concurrently.
import {execFileSync} from 'node:child_process';
import {createHash} from 'node:crypto';
import {mkdirSync, readFileSync, writeFileSync} from 'node:fs';
import {resolve} from 'node:path';
import {parseArgs} from 'node:util';

const {values} = parseArgs({options: {
  'before-bin': {type: 'string'}, 'before-plugin': {type: 'string'},
  'after-bin': {type: 'string'}, 'after-plugin': {type: 'string'},
  output: {type: 'string'}, cases: {type: 'string', default: 'dense,layers,large,state'},
  repeats: {type: 'string', default: '7'}, frames: {type: 'string', default: '300'},
  warmups: {type: 'string', default: '100'},
}});
for (const key of ['before-bin', 'before-plugin', 'after-bin', 'after-plugin', 'output']) {
  if (!values[key]) throw new Error(`--${key} is required`);
}
const repeats = Number(values.repeats), frames = Number(values.frames), warmups = Number(values.warmups);
if (!Number.isInteger(repeats) || repeats < 2 || repeats > 10 ||
    !Number.isInteger(frames) || frames < 1 || !Number.isInteger(warmups) || warmups < 0) {
  throw new Error('Require 2–10 repeats, positive frames, non-negative warmups');
}
const cases = {dense: [10000, 1, 'constant', 'dense'], layers: [10000, 8, 'feature', 'spread'],
  large: [100000, 1, 'constant', 'spread'], state: [10000, 1, 'state', 'spread']};
const selected = values.cases.split(',');
if (new Set(selected).size !== selected.length || selected.some(name => !Object.hasOwn(cases, name))) {
  throw new Error('Unknown or duplicate case');
}
const variants = ['before', 'after'].flatMap(version => [
  [version + '-native', resolve(values[version + '-bin']), 'native'],
  [version + '-plugin', resolve(values[version + '-bin']), resolve(values[version + '-plugin'])],
]);
const run = (cmd, args) => execFileSync(cmd, args, {encoding: 'utf8', maxBuffer: 128 * 1024 * 1024}).trim();
const output = resolve(values.output);
mkdirSync(output); // Refuse to overwrite an existing experiment.
const write = (name, data) => writeFileSync(`${output}/${name}.json`, JSON.stringify(data, null, 2) + '\n');
write('metadata', {
  revision: run('git', ['rev-parse', 'HEAD']), timestamp: new Date().toISOString(),
  diff_sha256: createHash('sha256').update(run('git', ['diff', 'HEAD'])).digest('hex'),
  cpu: run('sysctl', ['-n', 'machdep.cpu.brand_string']), os: run('sw_vers', ['-productVersion']),
  repeats, frames, warmups, cases: selected,
  artifacts: Object.fromEntries(['before-bin', 'before-plugin', 'after-bin', 'after-plugin'].map(key =>
    [key, {path: resolve(values[key]), sha256: createHash('sha256').update(readFileSync(values[key])).digest('hex')}]))});
const mean = a => a.reduce((sum, v) => sum + v, 0) / a.length;
const processes = [];
for (const name of selected) for (let repeat = 0; repeat < repeats; ++repeat) {
  const [count, layers, paint, density] = cases[name];
  let order = variants.map((_, i) => variants[(i + repeat) % variants.length]);
  if (repeat % 2) order.reverse();
  for (const [variant, binary, plugin] of order) {
    process.stderr.write(`${name} ${repeat + 1}/${repeats}: ${variant}\n`);
    const raw = run(binary, [plugin, String(count), String(layers), paint, String(frames), String(warmups), density]);
    writeFileSync(`${output}/${name}-${repeat}-${variant}.jsonl`, raw + '\n');
    const rows = raw.split('\n').filter(line => line.startsWith('{')).map(line => JSON.parse(line));
    const phases = Object.fromEntries([...new Set(rows.map(row => row.phase).filter(Boolean))].map(phase => {
      const samples = rows.filter(row => row.phase === phase);
      const metrics = Object.keys(samples[0]).filter(key => !['frame', 'count', 'layers', 'dense'].includes(key) &&
        samples.every(row => typeof row[key] === 'number' && Number.isFinite(row[key])));
      return [phase, {samples: samples.length, metrics: Object.fromEntries(metrics.map(key =>
        [key, mean(samples.map(row => row[key]))]))}];
    }));
    if (phases.warm_no_readback.samples !== frames) throw new Error('Incomplete measured phase');
    processes.push({case: name, repeat, variant, phases,
      peak_rss_bytes: rows.find(row => 'peak_rss_native_units' in row)?.peak_rss_native_units});
  }
}
const interval = a => {
  const m = mean(a), sd = Math.sqrt(a.reduce((s, v) => s + (v - m) ** 2, 0) / (a.length - 1));
  return {mean: m, ci95_half_width: [0, 12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262][a.length - 1] * sd / Math.sqrt(a.length)};
};
const comparisons = [];
for (const name of selected) for (const phase of Object.keys(processes.find(p => p.case === name).phases)) {
  for (const metric of ['wall_ms', 'encoding_ms']) {
    const groups = Object.fromEntries(variants.map(([variant]) => [variant,
      processes.filter(p => p.case === name && p.variant === variant).map(p => p.phases[phase].metrics[metric])]));
    const ratio = (a, b) => interval(groups[a].map((v, i) => v / groups[b][i]));
    if (metric === 'encoding_ms' && ['registration', 'startup_readback'].includes(phase)) continue;
    comparisons.push({case: name, phase, metric, modes: Object.fromEntries(Object.entries(groups).map(([k, v]) => [k, interval(v)])),
      after_to_before_plugin: ratio('after-plugin', 'before-plugin'),
      after_to_before_native: ratio('after-native', 'before-native'),
      after_plugin_to_native: ratio('after-plugin', 'after-native')});
  }
}
write('processes', processes);
write('comparisons', comparisons);
console.log(`Completed ${processes.length} processes: ${output}`);
