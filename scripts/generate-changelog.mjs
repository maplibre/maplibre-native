#!/usr/bin/env node

import { simpleGit } from "simple-git";
import * as fs from "node:fs";

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
      .filter((line) => line.startsWith("## "))[1]
      .slice(3);
    return `${platform}-v${lastVersion}`;
  }
  usage();
}

const tagLastVersion = getTagLastVersion();

const git = simpleGit();

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

const logs = await git.log({ from: commitLastVersion });
for await (const logEntry of logs.all.toReversed()) {
  console.log(`- ${formatMessageToMarkdownLink(logEntry.message)}.`);
}
