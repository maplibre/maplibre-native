// Checks that every Objective-C class and constant declared in the public
// headers of MapLibre is marked MLN_EXPORT. Classes and constants implemented in
// Objective-C++ are compiled with -fvisibility=hidden, so without MLN_EXPORT they
// are missing from the dynamic framework.
//
// Usage: check-public-symbols.js <sourcekitten> <iOS|macOS> <headers.zip>

import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { execFileSync } from "node:child_process";

const [sourcekitten, platform, headersZip] = process.argv.slice(2);
const sdk = { iOS: "iphoneos", macOS: "macosx" }[platform];
if (!headersZip || !sdk) {
  console.error("Usage: check-public-symbols.js <sourcekitten> <iOS|macOS> <headers.zip>");
  process.exit(1);
}

const headersDir = fs.mkdtempSync(path.join(process.env.TEST_TMPDIR ?? os.tmpdir(), "headers-"));
execFileSync("unzip", ["-q", headersZip, "-d", headersDir]);
const umbrellaHeader = path.join(headersDir, "MapLibre.h");
const sysroot = execFileSync("xcrun", ["--show-sdk-path", "--sdk", sdk]).toString().trim();

const docs = JSON.parse(execFileSync(sourcekitten, [
  "doc", "--objc", umbrellaHeader, "--",
  "-x", "objective-c", "-isysroot", sysroot, "-I", headersDir,
], { maxBuffer: Infinity }).toString());

// Number of lines, up to and including the declaration, that may hold MLN_EXPORT.
const exportLookback = {
  "sourcekitten.source.lang.objc.decl.class": 4,
  "sourcekitten.source.lang.objc.decl.constant": 2,
};

let missing = 0;
let scanned = 0;
for (const result of docs) {
  for (const [file, structure] of Object.entries(result)) {
    const lines = fs.readFileSync(file, "utf8").split("\n");
    scanned++;
    for (const decl of structure["key.substructure"] ?? []) {
      const lookback = exportLookback[decl["key.kind"]];
      if (!lookback) continue;
      const line = decl["key.doc.line"];
      const preceding = lines.slice(Math.max(0, line - lookback), line);
      if (!preceding.some((l) => /\bMLN_EXPORT\b/.test(l))) {
        const kind = decl["key.kind"].split(".").pop();
        console.error(`- missing MLN_EXPORT for ${kind} ${decl["key.name"]} in ${path.basename(file)}:${line}`);
        missing++;
      }
    }
  }
}

if (scanned === 0) {
  console.error(`SourceKitten did not return any headers for ${umbrellaHeader}`);
  process.exit(1);
}
if (missing > 0) {
  process.exit(1);
}
console.log(`All public ${platform} symbols in ${scanned} headers are exported.`);
