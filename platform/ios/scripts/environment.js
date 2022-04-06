#!/usr/bin/env node

// Output some `export` commands with a few extra environment variables. They'll be evaluated
// into the build environment and available for use in later steps.

const github = require('@octokit/rest')();
const {execSync} = require('child_process');

const pr = process.env['GITHUB_PULL_REQUEST'];
if (pr) {
    const number = +pr.match(/\/(\d+)\/?$/)[1];
    return github.pullRequests.get({
        owner: 'maplibre',
        repo: 'maplibre-gl-native',
        number
    }).then(({data}) => {
        const base = data.base.ref;
        const head = process.env['GITHUB_SHA'];
        const mergeBase = execSync(`git merge-base origin/${base} ${head}`).toString().trim();

        console.log(`export GITHUB_TARGET_BRANCH=${base}`);
        console.log(`export GITHUB_MERGE_BASE=${mergeBase}`);
    });
} else {
    const head = process.env['GITHUB_SHA'];
    for (const sha of execSync(`git rev-list --max-count=500 ${head}`).toString().trim().split('\n')) {
        const base = execSync(`git branch -r --contains ${sha} origin/main origin/release-*`).toString().split('\n')[0].trim().replace(/^origin\//, '');
        if (base) {
            const mergeBase = execSync(`git merge-base origin/${base} ${head}`).toString().trim();
            console.log(`export GITHUB_TARGET_BRANCH=${base}`);
            console.log(`export GITHUB_MERGE_BASE=${mergeBase}`);
            break;
        }
    }
}
