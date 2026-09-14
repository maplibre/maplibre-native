// Retain reproducible per-process aggregates without committing frame-by-frame logs.
import {mkdirSync, readFileSync, readdirSync, writeFileSync} from 'node:fs';
import {resolve} from 'node:path';

const [input, output] = process.argv.slice(2);
if (!input || !output) throw new Error('Usage: node export-circle-results.mjs INPUT OUTPUT');
if (resolve(input) === resolve(output)) throw new Error('Input and output must differ');
const read = name => JSON.parse(readFileSync(`${input}/${name}`, 'utf8'));
const metadata = read('metadata.json');
const summary = read('summary.json');
const mean = values => values.reduce((sum, value) => sum + value, 0) / values.length;
const processes = readdirSync(input).filter(name => name.endsWith('.jsonl')).sort().map(file => {
  const rows = readFileSync(`${input}/${file}`, 'utf8').trim().split('\n')
    .filter(line => line.startsWith('{')).map(line => JSON.parse(line));
  const phases = {};
  for (const phase of [...new Set(rows.map(row => row.phase).filter(Boolean))]) {
    const samples = rows.filter(row => row.phase === phase);
    const fields = Object.keys(samples[0]).filter(key => !['count', 'layers', 'dense', 'frame'].includes(key)
      && samples.every(row => typeof row[key] === 'number' && Number.isFinite(row[key])));
    phases[phase] = {samples: samples.length, metrics: Object.fromEntries(fields.map(field => {
      const values = samples.map(row => row[field]).sort((a, b) => a - b);
      const percentile = fraction => values[Math.max(0, Math.ceil(fraction * values.length) - 1)];
      return [field, {mean: mean(values), p50: percentile(0.5), p95: percentile(0.95)}];
    }))};
  }
  return {file, mode: rows[0].mode, peak_rss_bytes: rows.find(row => 'peak_rss_native_units' in row)?.peak_rss_native_units,
    phases};
});
if (processes.length !== metadata.repeats * 3 * 9) throw new Error('Expected a complete nine-workload run');
mkdirSync(output, {recursive: true});
for (const [file, value] of Object.entries({'metadata.json': metadata, 'summary.json': summary, 'processes.json': processes})) {
  writeFileSync(`${output}/${file}`, JSON.stringify(value, null, 2) + '\n');
}
console.log(`Exported ${processes.length} processes to ${output}`);
