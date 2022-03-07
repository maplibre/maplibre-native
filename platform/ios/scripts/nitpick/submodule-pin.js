#!/usr/bin/env node
const nitpick = require('.');
const child_process = require('child_process');

// Make sure that the maplibre-gl-js submodule pin is up to date
const head = child_process.execSync('git -C maplibre-gl-js rev-parse HEAD').toString().trim();
const revs = child_process.execSync(`git -C maplibre-gl-js branch -a --contains ${head}`).toString().split('\n');

if (revs.indexOf('  remotes/origin/main') >= 0) {
    nitpick.ok(`maplibre-gl-js submodule pin is merged to main`);
} else {
    nitpick.fail(`maplibre-gl-js submodule is pinned to ${head}, which isn't merged to main`);
}
