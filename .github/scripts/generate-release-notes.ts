/**
 * Generates the release notes of an Android or iOS release from the commits since the previous
 * release, so that they do not have to be written down in a CHANGELOG.md on every branch.
 *
 *   node generate-release-notes.ts --platform <android|ios> [--target <ref>]
 *     [--tag-prefix <prefix>] [--previous-tag <tag>]
 *
 * The Markdown goes to stdout. Every change links to its pull request.
 *
 * Needs the full history and the tags, e.g. `fetch-depth: 0` for actions/checkout.
 */

import { execFileSync } from 'node:child_process';
import { parseArgs } from 'node:util';

type Platform = 'android' | 'ios';

/** Paths of the code that every platform builds. */
const CORE_PATHS = ['src/', 'include/', 'shaders/', 'vendor/', 'platform/default/', 'plugins/'];

/** Paths of the code of one platform, next to the core. */
const PLATFORM_PATHS: Record<Platform, string[]> = {
  android: ['platform/android/'],
  ios: ['platform/ios/', 'platform/darwin/'],
};

/** Files that change in every release, without being a change to the SDK. */
const IGNORED_FILES = /(^|\/)VERSION$/;

/** The version of a release without a postfix, e.g. 13.7.1 but not 7.0.0-pre2 or 2026.10.19-globe. */
const PLAIN_VERSION = /^\d+\.\d+\.\d+$/;

/** Squash-merged pull requests end with the number of the pull request. */
const PULL_REQUEST_SUFFIX = /\s*\(#(\d+)\)$/;

/** GitHub rejects release descriptions of more than 125 000 characters. */
const MAX_LENGTH = 100_000;

const RECORD_SEPARATOR = '\x1e';
const FIELD_SEPARATOR = '\x1f';

/** `quiet` hides the error output of git. */
function git(args: string[], { quiet = false } = {}): string {
  return execFileSync('git', args, {
    encoding: 'utf8',
    maxBuffer: 256 * 1024 * 1024,
    stdio: ['ignore', 'pipe', quiet ? 'ignore' : 'inherit'],
  });
}

function fail(message: string): never {
  console.error(`Error: ${message}`);
  process.exit(1);
}

/**
 * The closest release before `target`. Only versions without a postfix count, so pre-releases
 * and other variants are never the previous release. This also skips the tag of the release
 * itself, which is usually created before the notes are.
 */
function findPreviousTag(prefix: string, target: string): string | undefined {
  let ref = `${target}^`;
  for (;;) {
    let tag: string;
    try {
      tag = git(['describe', '--tags', '--abbrev=0', '--match', `${prefix}*`, ref], { quiet: true }).trim();
    } catch {
      return undefined;
    }
    if (PLAIN_VERSION.test(tag.slice(prefix.length))) {
      return tag;
    }
    ref = `${tag}^`;
  }
}

const { values } = parseArgs({
  options: {
    platform: { type: 'string' },
    target: { type: 'string', default: 'HEAD' },
    'tag-prefix': { type: 'string' },
    'previous-tag': { type: 'string' },
  },
  allowPositionals: false,
  strict: true,
});

const platform = values.platform;
if (platform !== 'android' && platform !== 'ios') {
  fail('--platform must be android or ios');
}

const tagPrefix = values['tag-prefix'] ?? `${platform}-v`;
const repositoryUrl = `${process.env.GITHUB_SERVER_URL ?? 'https://github.com'}/${
  process.env.GITHUB_REPOSITORY ?? 'maplibre/maplibre-native'
}`;

let target: string;
try {
  target = git(['rev-parse', '--verify', '--quiet', `${values.target}^{commit}`], { quiet: true }).trim();
} catch {
  fail(`${values.target} is not a commit`);
}

const previousTag = values['previous-tag'] ?? findPreviousTag(tagPrefix, target);
if (!previousTag) {
  fail(`No release tag starting with ${tagPrefix} is reachable from ${target}, pass --previous-tag`);
}
try {
  git(['rev-parse', '--verify', '--quiet', `${previousTag}^{commit}`], { quiet: true });
} catch {
  fail(`${previousTag} is not a tag or commit, is the full history fetched?`);
}

const relevantPaths = [...PLATFORM_PATHS[platform], ...CORE_PATHS];

const log = git([
  'log',
  '--no-merges',
  '--no-renames',
  '--name-only',
  `--format=${RECORD_SEPARATOR}%H${FIELD_SEPARATOR}%s`,
  `${previousTag}..${target}`,
]);

const changes: string[] = [];
for (const record of log.split(RECORD_SEPARATOR).slice(1).reverse()) {
  const [header, ...files] = record.split('\n').filter((line) => line !== '');
  const [sha, subject] = header.split(FIELD_SEPARATOR);

  const changesSdk = files.some(
    (file) => !IGNORED_FILES.test(file) && relevantPaths.some((path) => file.startsWith(path)),
  );
  if (!changesSdk) {
    continue;
  }

  const pullRequest = PULL_REQUEST_SUFFIX.exec(subject);
  if (pullRequest) {
    const title = subject.slice(0, pullRequest.index);
    changes.push(`- ${title} ([#${pullRequest[1]}](${repositoryUrl}/pull/${pullRequest[1]})).`);
  } else {
    changes.push(`- ${subject} ([${sha.slice(0, 7)}](${repositoryUrl}/commit/${sha})).`);
  }
}

const footer = `**Full Changelog**: ${repositoryUrl}/compare/${previousTag}...${target}`;

const lines: string[] = [];
let length = footer.length;
for (const change of changes) {
  if (length + change.length > MAX_LENGTH) {
    break;
  }
  lines.push(change);
  length += change.length + 1;
}
if (lines.length < changes.length) {
  lines.push(`- And ${changes.length - lines.length} more changes, see the full changelog.`);
}
if (lines.length === 0) {
  lines.push(`No changes to the SDK since ${previousTag}.`);
}

console.log(`${lines.join('\n')}\n\n${footer}`);
