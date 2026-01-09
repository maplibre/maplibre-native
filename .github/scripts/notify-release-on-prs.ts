/**
 * This script should be run as node notify-release-on-prs.ts --tag <release_tag>.
 * It will post a comment on each of the PRs included in the changelog of the specified release.
 * The script can be ran multiple times for the same release_tag, it will not post a
 * new comment but instead update the existing comment.
 */

import { Octokit } from '@octokit/rest';
import { parseArgs } from 'node:util';
import * as core from "@actions/core";

const { values } = parseArgs({
  options: {
    tag: { type: 'string', short: 't' }
  },
  allowPositionals: false,
  strict: true
});

function errMessage(err: unknown) {
  return err instanceof Error ? err.message : `${err}`;
}

const owner = 'maplibre';
const repo = 'maplibre-native';

function extractPrNumbers(params: {
  releaseBody: string;
  owner: string;
  repo: string;
}): Set<number> {
  const { releaseBody, owner, repo } = params;
  const prs = new Set<number>();

  const esc = (s: string) => s.replace(/[-/\\^$*+?.()|[\]{}]/g, '\\$&');
  const urlRegex = new RegExp(
    `https?:\\/\\/github\\.com\\/${esc(owner)}\\/${esc(repo)}\\/pull\\/(\\d+)`,
    'g'
  );

  for (const match of releaseBody.matchAll(urlRegex)) {
    prs.add(Number(match[1]));
  }

  return prs;
}

async function createOrUpdateReleaseNotification({
  octokit,
  prNumber,
  tag,
  owner,
  repo
}: {
  owner: string,
  repo: string,
  octokit: Octokit,
  prNumber: number,
  tag: string
}) {

  const m = tag.match(/^([A-Za-z0-9]+)-v(.+)$/i);
  const platform = m?.[1]?.toLowerCase();
  const version = m?.[2] ?? '';

  if (!platform || !version) {
    throw new Error(`Failed to parse platform or version from ${tag}`);
  }

  const MARKER = `<!-- release-notification-${platform} -->`;

  function linkText() {
    if (platform === 'ios') {
      return `MapLibre iOS ${version}`;
    } else if (platform === 'android') {
      return `MapLibre Android ${version}`;
    }
    return tag;
  }

  const { data: comments } = await octokit.issues.listComments({
    owner,
    repo,
    issue_number: prNumber
  });

  const existing = comments.find((({ body }) => body?.includes(MARKER)));

  const releaseUrl = `https://github.com/${owner}/${repo}/releases/tag/${tag}`;
  releaseUrl
  const body = `
${MARKER}
🚀 This PR has been included in the [${linkText()}](${releaseUrl}) release.
`.trim();

  if (existing) {
    await octokit.issues.updateComment({
      owner,
      repo,
      comment_id: existing.id,
      body
    });
    console.log(`Updated comment on PR #${prNumber}`);
  } else {
    await octokit.issues.createComment({
      owner,
      repo,
      issue_number: prNumber,
      body
    });
    console.log(`Created comment on PR #${prNumber}`);
  }
}

try {
  const { tag } = values;

  if (!tag) {
    throw new Error("pass tag with --tag");
  }

  const token = process.env.GITHUB_TOKEN;
  if (!token) {
    throw new Error('GITHUB_TOKEN environment variable is required');
  }

  const octokit = new Octokit({ auth: token });

  const release = await octokit.repos.getReleaseByTag({ owner, repo, tag });
  const releaseBody = release.data.body || '';

  if (!releaseBody) {
    throw new Error(`No release notes found for tag '${tag}'.`);
  }

  const prs = extractPrNumbers({
    owner,
    releaseBody,
    repo
  });

  if (prs.size === 0) {
    console.log(`No PR references found in release notes for tag '${tag}'.`);
    process.exit(0);
  }

  for (const prNumber of prs) {
    try {
      await createOrUpdateReleaseNotification({
        octokit,
        owner,
        prNumber,
        repo,
        tag
      });
    } catch (err) {
      console.error(`Failed to comment on PR #${prNumber}: ${errMessage(err)}`);
    }
  }
} catch (err) {
  core.setFailed(errMessage(err));
}
