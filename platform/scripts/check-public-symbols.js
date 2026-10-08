import fs from "node:fs";
import { execFileSync } from "node:child_process";
import _ from "lodash";

const keyword = /\bMLN_EXPORT\b/;

let scanned = [];

function hasMissingSymbols() {
  let missing = false;
  let sysroot = execFileSync('xcrun', ['--show-sdk-path', '--sdk', 'iphonesimulator']).toString().trim();
  let umbrellaPath = 'platform/ios/MapLibre_umbrella.h';
  let docArgs = ['doc', '--objc', umbrellaPath, '--',
                 '-x', 'objective-c', '-I', 'platform/ios/src/',
                 '-I', 'platform/darwin/src/', '-isysroot', sysroot];
  let docStr = execFileSync('sourcekitten', docArgs, { maxBuffer: Infinity }).toString().trim();
  let docJson = JSON.parse(docStr);
  _.forEach(docJson, function (result) {
    _.forEach(result, function (structure, path) {
      // Prevent multiple scans of the same file.
      if (scanned.indexOf(path) >= 0) return;
      scanned.push(path);

      const src = fs.readFileSync(path, 'utf8').split('\n');
      _.forEach(structure['key.substructure'], function (substructure) {
        switch (substructure['key.kind']) {
          case 'sourcekitten.source.lang.objc.decl.class':
            if (!keyword.test(src[substructure['key.doc.line'] - 1]) && !keyword.test(src[substructure['key.doc.line'] - 2]) && !keyword.test(src[substructure['key.doc.line'] - 3]) && !keyword.test(src[substructure['key.doc.line'] - 4])) {
              console.warn(`- missing symbol export for class ${substructure['key.name']} in ${path}:${substructure['key.doc.line']}:${substructure['key.doc.column']}`);
              missing = true;
            }
            break;
          case 'sourcekitten.source.lang.objc.decl.constant':
            if (!keyword.test(src[substructure['key.doc.line'] - 1]) && !keyword.test(src[substructure['key.doc.line'] - 2])) {
              console.warn(`- missing symbol export for constant ${substructure['key.name']} in ${path}:${substructure['key.doc.line']}:${substructure['key.doc.column']}`);
              missing = true;
            }
            break;
        }
      });
    });
  });

  return missing;
}

function ensureSourceKittenIsInstalled() {
  try {
    execFileSync('which', ['sourcekitten']);
  } catch (e) {
    console.log(`Installing SourceKitten via Homebrew…`);
    execFileSync('brew', ['install', 'sourcekitten']);
  }
}

ensureSourceKittenIsInstalled();

if (hasMissingSymbols()) {
  process.exit(1);
} else {
  console.warn(`All symbols are correctly exported.`);
}
