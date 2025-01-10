#!/usr/bin/env node

import { simpleGit } from "simple-git";
import * as fs from "node:fs";
import { Octokit } from "@octokit/rest";

/**
 * @returns {never}
 */
function usage() {
  process.exit(1);
}

const platform = process.argv.at(2);

function getTagLastVersion() {
  if (platform === "ios" || platform === "android") {
    const lastVersion = fs
      .readFileSync(`platform/${platform}/CHANGELOG.md`, "utf-8")
      .split("\n")
      .filter((line) => line.startsWith("## "))[0]
      .slice(3);
    return `${platform}-v${lastVersion}`;
  }
  usage();
}

const tagLastVersion = getTagLastVersion();

if (!process.env.GITHUB_ACCESS_TOKEN) throw new Error("!process.env.GITHUB_ACCESS_TOKEN");

const git = simpleGit();
const octokit = new Octokit({
  auth: process.env.GITHUB_ACCESS_TOKEN
});

/**
 *
 * @param {string} tag
 * @returns
 */
async function getCommit(tag) {
  const result = await git.show(tag);
  const match = result.match(/^commit\s([0-9a-f]{40})/m);

  if (match) return match[1];
  else throw new Error("Failed to find commit");
}

/**
 *
 * @param {string} message
 * @returns
 */
function formatMessageToMarkdownLink(message) {
  // Regular expression to match the issue number pattern
  const regex = /#(\d+)/;
  const match = message.match(regex);

  // Check if a match is found
  if (match) {
    const issueNumber = match[1];
    const link = `https://github.com/maplibre/maplibre-native/pull/${issueNumber}`;
    // Replace the issue number in the message with the Markdown link
    return message.replace(regex, `[#${issueNumber}](${link})`);
  }

  // Return the original message if no issue number is found
  return message;
}

const commitLastVersion = await getCommit(tagLastVersion);

// Load all closed pull requests
const pulls = await octokit.paginate(octokit.pulls.list, {
  owner: "maplibre",
  repo: "maplibre-native",
  state: "closed",
  per_page: 100,
});

/**
 * @type {Map<string, typeof pulls[0]>}[] }>}
 */
const pullRequestsMap = new Map();
pulls.forEach(pr => {
  if (pr.merge_commit_sha) {
    pullRequestsMap.set(pr.merge_commit_sha, pr);
  }
});

const logs = await git.log({ from: commitLastVersion });

for await (const logEntry of logs.all.toReversed()) {
  const pr = pullRequestsMap.get(logEntry.hash);

  const log = () => console.log(`- ${formatMessageToMarkdownLink(logEntry.message)}.`);


  if (!pr) {
    log();
    continue;
  }

  // only log changelog entry when PR has corresponding platform label (or none)
  const hasLabel = (/** @type {string} **/ label) =>  pr.labels.some(({name}) => name === label);

  if (!hasLabel("ios") && !hasLabel("android") && !hasLabel("node")) log();
  else if (platform === "android" && hasLabel("android")) log();
  else if (platform === "ios" && hasLabel("ios")) log();
}
