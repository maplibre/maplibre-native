import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { test } from 'node:test';
import { inflateSync } from 'node:zlib';
import { packSources, generateCode } from './generate_metal_shader_code.ts';

test('every Metal source fragment survives compression byte for byte', () => {
    const directory = path.join(import.meta.dirname, '../shaders/mtl');
    const packed = packSources(directory);
    const source = inflateSync(packed.compressed);
    assert.deepEqual(source, packed.source);
    for (const [file, { offset, length }] of packed.fragments) {
        assert.deepEqual(source.subarray(offset, offset + length), fs.readFileSync(path.join(directory, file)), file);
    }
    assert.equal(packed.manifest.shaders.length, new Set(packed.manifest.shaders.map(s => s.shader)).size);
    assert.ok(packed.compressed.length < packed.source.length);
    assert.equal(generateCode(packed), generateCode(packSources(directory)));
});

test('shared fragments occupy one range; offsets and lengths count UTF-8 bytes', () => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'mln-shaders-'));
    try {
        fs.writeFileSync(path.join(directory, 'manifest.json'), JSON.stringify({
            prelude: 'shared.metal',
            shaders: [
                { prelude: 'shared.metal', source: 'a.metal' },
                { prelude: 'shared.metal', source: 'b.metal' },
            ],
        }));
        fs.writeFileSync(path.join(directory, 'shared.metal'), '// café 🌍\n');
        fs.writeFileSync(path.join(directory, 'a.metal'), 'a\0b\n');
        fs.writeFileSync(path.join(directory, 'b.metal'), '');
        const { fragments, compressed } = packSources(directory);
        assert.equal(fragments.size, 3);
        const decoded = inflateSync(compressed);
        assert.equal(decoded.toString(), '// café 🌍\na\0b\n');
        assert.equal(fragments.get('a.metal')!.offset, Buffer.byteLength('// café 🌍\n'));
        assert.equal(fragments.get('b.metal')!.offset, decoded.length);
        assert.equal(fragments.get('b.metal')!.length, 0);
    } finally {
        fs.rmSync(directory, { recursive: true, force: true });
    }
});
