import fs from 'node:fs';
import { execSync } from 'node:child_process';

const changelogPath = 'CHANGELOG.md';
const changelog = fs.readFileSync(changelogPath, 'utf8');

function usage() {
  console.error(`Usage ${process.argv[0]} ${process.argv[1]} current-version`);
  process.exit(1);
}

if (process.argv.length !== 3) {
  usage();
}

let currentVersion = process.argv[2];
if (typeof currentVersion !== 'string') {
  usage();
}

/*
  Parse the raw changelog text and split it into individual releases.
*/
const regex = /^## (\d+\.\d+\.\d+(?:-[^\s]+)?).*?\n(.+?)(?=\n^## \d+\.\d+\.\d+(?:-[^\s]+)?.*?\n)/gms;

let releaseNotes = {};
let match;
while (match = regex.exec(changelog)) {
  const version = match[1];
  const changelog = match[2].trim();
  releaseNotes[version] = changelog;
}

if (releaseNotes[currentVersion]) {
  process.stdout.write(releaseNotes[currentVersion]);
} else if (currentVersion.includes("pre")) {
  console.error(`No release notes found for version ${currentVersion}, ignoring since pre release.`);
  process.exit(0);
} else {
  console.error(`No release notes found for version ${currentVersion}, please update the changelog.`);
  process.exit(1);
}
