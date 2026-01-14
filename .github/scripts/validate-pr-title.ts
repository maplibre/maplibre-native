#!/usr/bin/env node

// This script validates that a PR title also contains its labels
// in the title. This is a requirement, because this makes it easier
// to parse commit messages. For example, for generating a changelog
// for a particular platform.
// Usage:
// PR_TITLE="Make some changes to Android" PR_LABELS="android" node validate-pr-title.ts

export function validateTitle({
  title,
  labels
}: {
  title: string,
  labels: Set<string>
}): { ok: true } | { ok: false, mismatches: string[] } {
  // these labels are the ones we care about
  const keywords = new Set(['node', 'core', 'ios', 'android', 'qt']);

  // when labels includes core, only 'core' is required
  const requiredKeyWords = labels.has('core')
    ? new Set(['core'])
    : labels.intersection(keywords);

  title = title.toLocaleLowerCase();

  const actualKeywords = new Set();
  for (const keyword of requiredKeyWords) {
    if (title.includes(keyword)) actualKeywords.add(keyword);
  }

  const mismatches = requiredKeyWords.difference(actualKeywords);

  if (mismatches.size === 0) return { ok: true };
  return { ok: false, mismatches: [...mismatches] };
}

if (import.meta.main) {
  if (!process.env.PR_TITLE) {
    console.log('::error::PR_TITLE environment variable is not set');
    process.exit(1);
  }

  if (!process.env.PR_LABELS) {
    console.log('::error::PR_LABELS environment variable is not set');
    process.exit(1);
  }

  const title = process.env.PR_TITLE;
  const labelsString = process.env.PR_LABELS.toLowerCase();
  const labels = new Set(labelsString.split(',').map(l => l.trim()).filter(l => l));

  const result = validateTitle({
    title,
    labels
  });
  if (result.ok) {
    console.log('✅ PR labels match the title');
    process.exit(0);
  }

  if (!result.ok) {
    if (result.mismatches.length > 0) {
      console.log(`::error::PR title does not match PR labels. Mismatched: ${result.mismatches.join(', ')}. Title: "${process.env.PR_TITLE}". Labels: "${process.env.PR_LABELS}"`);
      process.exit(1);
    }
  }
}
