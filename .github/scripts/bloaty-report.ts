import * as core from "@actions/core";
import { Octokit } from "@octokit/rest";
import { execFileSync } from "node:child_process";
import { mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { resolve } from "node:path";
import { pathToFileURL } from "node:url";

const platforms = [
  { name: "iOS", workflow: "ios-ci.yml", artifact: "ios-size-test-files", files: ["MapLibre_dynamic", "MapLibre_DWARF"] },
  { name: "Linux", workflow: "linux-ci.yml", artifact: "mbgl-render", files: ["mbgl-render"] },
] as const;

export interface Run {
  id: number;
  head_sha: string;
  head_branch: string | null;
  head_repository: { id: number } | null;
  event: string;
  status: string | null;
  conclusion: string | null;
  html_url: string;
  created_at: string;
}

export interface Result {
  platform: string;
  status: string;
  run?: Run;
  baseline?: Run;
  before?: Sizes;
  after?: Sizes;
  diff?: string;
}

interface Sizes { vm: number; file: number }
interface Report { sha: string; results: Result[] }
type Platform = typeof platforms[number];
type Artifact = { id: number; expired: boolean; name: string };
type Repo = { owner: string; repo: string };

export function selectRun(runs: Run[], trigger: Run): Run | undefined {
  return runs.filter(run =>
    run.head_sha === trigger.head_sha && run.head_branch === trigger.head_branch &&
    run.head_repository?.id === trigger.head_repository?.id && run.event === "pull_request"
  ).sort((a, b) => b.id - a.id)[0];
}

export function parseSizes(tsv: string): Sizes {
  const lines = tsv.trim().split(/\r?\n/);
  if (lines.shift() !== "sections\tvmsize\tfilesize") throw new Error("Unexpected Bloaty TSV header");
  if (!lines.length) throw new Error("Bloaty returned no section sizes");
  return lines.reduce((total, line) => {
    const fields = line.split("\t");
    const vm = Number(fields[1]);
    const file = Number(fields[2]);
    if (fields.length !== 3 || !Number.isSafeInteger(vm) || !Number.isSafeInteger(file) || vm < 0 || file < 0) {
      throw new Error("Invalid Bloaty section size");
    }
    return { vm: total.vm + vm, file: total.file + file };
  }, { vm: 0, file: 0 });
}

function size(bytes: number): string {
  const magnitude = Math.abs(bytes);
  if (magnitude < 1024) return `${magnitude} B`;
  if (magnitude < 1024 ** 2) return `${(magnitude / 1024).toFixed(2)} KiB`;
  return `${(magnitude / 1024 ** 2).toFixed(2)} MiB`;
}

export function change(before: number, after: number): string {
  const delta = after - before;
  if (delta === 0) return "0 B (0.00%)";
  const sign = delta > 0 ? "+" : "−";
  const percent = before === 0 ? "new" : `${sign}${(Math.abs(delta) / before * 100).toFixed(2)}%`;
  return `${sign}${size(delta)} (${percent})`;
}

function runLink(run: Run): string {
  return `[\`${run.head_sha.slice(0, 7)}\`](${run.html_url})`;
}

function code(text: string): string {
  // Compile-unit names originate in PR debug information, so they can contain
  // Markdown fences. Use a fence longer than any backtick sequence in the data.
  const longest = Math.max(2, ...(text.match(/`+/g) ?? []).map(match => match.length));
  const fence = "`".repeat(longest + 1);
  return `${fence}text\n${text.trimEnd()}\n${fence}`;
}

export function formatReport(report: Report, reportUrl: string, full = false): string {
  const lines = [
    "## 🐋 Binary size report", "",
    `Revision: \`${report.sha.slice(0, 7)}\`. Compared with the latest available main build for each platform.`, "",
    "| Platform | Main build | VM size change | File size change | Status |",
    "| :--- | :--- | ---: | ---: | :--- |",
  ];
  for (const result of report.results) {
    lines.push(`| ${result.platform} | ${result.baseline ? runLink(result.baseline) : "—"} | ${result.before && result.after ? change(result.before.vm, result.after.vm) : "—"} | ${result.before && result.after ? change(result.before.file, result.after.file) : "—"} | ${result.status} |`);
  }
  lines.push("", "VM size is the space mapped into memory. File size includes embedded debug information on Linux. Positive changes mean growth.");
  if (!full) lines.push("", `[Full iOS and Linux report](${reportUrl})`);
  for (const result of report.results) {
    lines.push("", full ? `### ${result.platform}` : `<details>\n<summary>${result.platform} details</summary>`, "");
    if (result.run) lines.push(`PR build: ${runLink(result.run)} (${result.run.created_at}).`, "");
    if (result.baseline) lines.push(`Main build: ${runLink(result.baseline)} (${result.baseline.created_at}).`, "");
    if (result.before && result.after) {
      lines.push(`VM size: ${size(result.before.vm)} → ${size(result.after.vm)}. File size: ${size(result.before.file)} → ${size(result.after.file)}.`, "");
    }
    if (result.diff !== undefined) {
      const diffLines = result.diff.trimEnd().split("\n");
      const excerpt = diffLines.length <= 14 ? result.diff : [...diffLines.slice(0, 12), "…", diffLines.at(-1)].join("\n");
      lines.push(code(full ? result.diff : excerpt), "");
      if (!full && diffLines.length > 14) lines.push("First rows shown; the full report contains all compile units.", "");
    } else lines.push(result.status, "");
    if (!full) lines.push("</details>");
  }
  return lines.join("\n") + "\n";
}

async function artifacts(octokit: Octokit, repo: Repo, runId: number): Promise<Artifact[]> {
  return octokit.paginate(octokit.rest.actions.listWorkflowRunArtifacts, { ...repo, run_id: runId, per_page: 100 });
}

export async function findBaseline(octokit: Octokit, repo: Repo, platform: Platform) {
  // Artifacts identify the revision and keep the iOS binary/dSYM together. The
  // old mutable S3 *-main objects can be stale or come from different builds.
  for await (const page of octokit.paginate.iterator(octokit.rest.actions.listArtifactsForRepo, {
    ...repo, name: platform.artifact, per_page: 100,
  })) {
    for (const artifact of page.data) {
      if (artifact.expired || !artifact.workflow_run?.id || artifact.workflow_run.head_branch !== "main" ||
          artifact.workflow_run.head_repository_id !== artifact.workflow_run.repository_id) continue;
      const { data: run } = await octokit.rest.actions.getWorkflowRun({ ...repo, run_id: artifact.workflow_run.id });
      if (run.status === "completed" && ["push", "workflow_dispatch"].includes(run.event) &&
          run.path === `.github/workflows/${platform.workflow}`) return { run, artifact };
    }
  }
  return undefined;
}

async function downloadInputs(octokit: Octokit, repo: Repo, artifact: Artifact, platform: Platform, directory: string) {
  mkdirSync(directory, { recursive: true });
  const { data } = await octokit.rest.actions.downloadArtifact({ ...repo, artifact_id: artifact.id, archive_format: "zip" });
  const archive = resolve(directory, "artifact.zip");
  writeFileSync(archive, Buffer.from(data as ArrayBuffer));
  // Extract only expected files; never unpack arbitrary paths from a PR ZIP.
  for (const file of platform.files) {
    const contents = execFileSync("unzip", ["-p", archive, file], { maxBuffer: 512 * 1024 ** 2 });
    if (!contents.length) throw new Error(`Empty artifact member: ${file}`);
    writeFileSync(resolve(directory, file), contents);
  }
}

function bloaty(args: string[]): string {
  return execFileSync(resolve("bloaty/build/bloaty"), args, {
    encoding: "utf8", maxBuffer: 64 * 1024 ** 2, timeout: 120_000,
  });
}

export function compareBinaries(platform: Platform["name"], currentDirectory: string, baselineDirectory: string) {
  const filename = platform === "iOS" ? "MapLibre_dynamic" : "mbgl-render";
  const current = resolve(currentDirectory, filename);
  const base = resolve(baselineDirectory, filename);
  const before = parseSizes(bloaty(["--tsv", "-n", "0", "-d", "sections", base]));
  const after = parseSizes(bloaty(["--tsv", "-n", "0", "-d", "sections", current]));
  const debug = platform === "iOS" ? [
    "--debug-file", resolve(currentDirectory, "MapLibre_DWARF"),
    "--debug-file", resolve(baselineDirectory, "MapLibre_DWARF"),
  ] : [];
  const diff = bloaty([...debug, "-n", "0", "-s", "vm", "-d", "compileunits", current, "--", base]);
  return { before, after, diff };
}

export async function analyze(octokit: Octokit, repo: Repo, trigger: Run, platform: Platform): Promise<Result> {
  const result: Result = { platform: platform.name, status: "Waiting for CI" };
  try {
    const runs = await octokit.paginate(octokit.rest.actions.listWorkflowRuns, {
      ...repo, workflow_id: platform.workflow, head_sha: trigger.head_sha, event: "pull_request", per_page: 100,
    });
    const run = selectRun(runs, trigger);
    if (!run) return result;
    result.run = run;
    if (run.status !== "completed") return result;
    const artifact = (await artifacts(octokit, repo, run.id)).find(item => item.name === platform.artifact && !item.expired);
    if (!artifact) {
      result.status = `No size artifact (CI: ${run.conclusion ?? run.status})`;
      return result;
    }
    const baseline = await findBaseline(octokit, repo, platform);
    if (!baseline) {
      result.status = "No main size artifact available";
      return result;
    }
    result.baseline = baseline.run;
    const directory = resolve("bloaty-inputs", platform.name.toLowerCase());
    await downloadInputs(octokit, repo, artifact, platform, resolve(directory, "pr"));
    await downloadInputs(octokit, repo, baseline.artifact, platform, resolve(directory, "main"));
    Object.assign(result, compareBinaries(platform.name, resolve(directory, "pr"), resolve(directory, "main")));
    result.status = "Ready";
  } catch (error) {
    core.warning(`${platform.name} Bloaty analysis failed: ${error instanceof Error ? error.message : error}`);
    result.status = "Analysis failed (see workflow logs)";
    // Avoid publishing totals as a successful comparison if debug analysis failed.
    delete result.before;
    delete result.after;
  }
  return result;
}

export async function findPullRequest(octokit: Octokit, repo: Repo, trigger: Run) {
  // Resolve PRs through the API, not the late pr-number artifact: failed builds
  // often never upload it, and fork workflow_run payloads can have an empty PR list.
  const pulls = await octokit.paginate(octokit.rest.pulls.list, {
    ...repo, state: "open", per_page: 100,
  });
  return pulls.find(pr => pr.head.sha === trigger.head_sha && pr.head.ref === trigger.head_branch &&
    pr.head.repo?.id === trigger.head_repository?.id);
}

async function main() {
  const command = process.argv[2];
  if (command === "format") {
    const report = JSON.parse(readFileSync("bloaty-report.json", "utf8")) as Report;
    const message = formatReport(report, process.env.REPORT_URL ?? "");
    writeFileSync("message.md", message);
    if (process.env.GITHUB_STEP_SUMMARY) writeFileSync(process.env.GITHUB_STEP_SUMMARY, message);
    return;
  }
  const event = JSON.parse(readFileSync(process.env.GITHUB_EVENT_PATH!, "utf8")) as { workflow_run: Run };
  const trigger = event.workflow_run;
  const [owner, repoName] = process.env.GITHUB_REPOSITORY!.split("/");
  const repo = { owner, repo: repoName };
  const octokit = new Octokit({ auth: process.env.GITHUB_TOKEN });
  if (command === "find-pr") {
    const pr = await findPullRequest(octokit, repo, trigger);
    core.setOutput("number", pr?.number ?? "");
    if (!pr) core.info("No open PR matches this revision; ignoring obsolete or cancelled run.");
  } else if (command === "check-current") {
    const { data: pr } = await octokit.rest.pulls.get({ ...repo, pull_number: Number(process.env.PR_NUMBER) });
    core.setOutput("current", pr.state === "open" && pr.head.sha === trigger.head_sha &&
      pr.head.repo?.id === trigger.head_repository?.id);
  } else if (command === "prepare") {
    const results: Result[] = [];
    for (const platform of platforms) results.push(await analyze(octokit, repo, trigger, platform));
    const report = { sha: trigger.head_sha, results };
    writeFileSync("bloaty-report.json", JSON.stringify(report));
    writeFileSync("bloaty-report.md", formatReport(report, "", true));
    const message = formatReport(report, process.env.REPORT_URL ?? "");
    writeFileSync("message.md", message);
    core.setOutput("has-errors", results.some(result => result.status.startsWith("Analysis failed")));
  } else throw new Error(`Unknown command: ${command}`);
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  main().catch(error => core.setFailed(error instanceof Error ? error.message : String(error)));
}
