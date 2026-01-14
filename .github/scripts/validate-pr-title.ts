#!/usr/bin/env node

// This script validates that a PR title also contains its labels
// in the title. This is a requirement, because this makes it easier
// to parse commit messages. For example, for generating a changelog
// for a particular platform.
// Usage:
// PR_TITLE="Make some changes to Android" PR_LABELS="android" node validate-pr-title.ts

if (!process.env.PR_TITLE) {
  console.log('::error::PR_TITLE environment variable is not set');
  process.exit(1);
}

if (!process.env.PR_LABELS) {
  console.log('::error::PR_LABELS environment variable is not set');
  process.exit(1);
}

const prTitle = process.env.PR_TITLE.toLowerCase();
const labelsString = process.env.PR_LABELS.toLowerCase();
const labels = labelsString.split(',').map(l => l.trim()).filter(l => l);
// these labels are the ones we care about
const keywords = ['node', 'core', 'ios', 'android', 'qt'];

const mismatches = [];
for (const keyword of keywords) {
  if (labels.includes(keyword) && !prTitle.includes(keyword)) {
    mismatches.push(keyword);
  }
}

if (mismatches.length > 0) {
  console.log(`::error::PR title does not match PR labels. Mismatched: ${mismatches.join(', ')}. Title: "${process.env.PR_TITLE}". Labels: "${process.env.PR_LABELS}"`);
  process.exit(1);
}

console.log('✅ PR labels match the title');
