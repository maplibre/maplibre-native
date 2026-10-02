import assert from "node:assert/strict";
import test from "node:test";
import { Octokit } from "@octokit/rest";
import { analyze, change, findBaseline, findPullRequest, formatReport, parseSizes, selectRun, type Run } from "./bloaty-report.ts";
import { jobSkipState } from "./check-job-skipped.ts";

const trigger: Run = {
  id: 10, head_sha: "123456789abcdef", head_branch: "feature", head_repository: { id: 2 },
  event: "pull_request", status: "completed", conclusion: "success",
  html_url: "https://github.com/maplibre/maplibre-native/actions/runs/10", created_at: "2026-10-02T12:00:00Z",
};
const repo = { owner: "maplibre", repo: "maplibre-native" };
const linux = { name: "Linux", workflow: "linux-ci.yml", artifact: "mbgl-render", files: ["mbgl-render"] } as const;

test("only the latest run from the same revision, branch and fork is eligible", () => {
  const runs = [
    trigger,
    { ...trigger, id: 11, status: "in_progress", conclusion: null },
    { ...trigger, id: 12, head_repository: { id: 3 } },
    { ...trigger, id: 13, head_sha: "old" },
    { ...trigger, id: 14, head_branch: "other" },
    { ...trigger, id: 15, event: "push" },
  ];
  assert.equal(selectRun(runs, trigger)?.id, 11);
  assert.equal(selectRun(runs.slice(2), trigger), undefined);
});

test("missing build jobs are skipped instead of failing the reporter", () => {
  assert.deepEqual(jobSkipState([], "ios-build"), { was_skipped: true, was_skipped_or_cancelled: true });
  assert.deepEqual(jobSkipState([{ name: "ios-build-cmake", conclusion: "success" }], "ios-build"), {
    was_skipped: true, was_skipped_or_cancelled: true,
  });
});

test("a cancelled matrix sibling does not hide an available size artifact", () => {
  const jobs = [
    { name: "linux-build-and-test (vulkan, false)", conclusion: "cancelled" },
    { name: "linux-build-and-test (opengl, false)", conclusion: "success" },
  ];
  assert.deepEqual(jobSkipState(jobs, "linux-build-and-test"), { was_skipped: false, was_skipped_or_cancelled: false });
  jobs[1].conclusion = "skipped";
  assert.deepEqual(jobSkipState(jobs, "linux-build-and-test"), { was_skipped: false, was_skipped_or_cancelled: true });
});

test("TSV totals keep VM and file size separate, including debug-only sections", () => {
  assert.deepEqual(parseSizes("sections\tvmsize\tfilesize\n.text\t2048\t2048\n.debug_info\t0\t4096\n.bss\t1024\t0\n"), {
    vm: 3072, file: 6144,
  });
  assert.throws(() => parseSizes("sections\tvmsize\tfilesize\n.text\tnan\t12"));
  assert.throws(() => parseSizes("sections\tvmsize\tfilesize\n.text\t-1\t12"));
  assert.throws(() => parseSizes("sections\tvmsize\tfilesize\n"));
});

test("changes format growth, shrinkage, zero and a zero baseline", () => {
  assert.equal(change(1024, 2048), "+1.00 KiB (+100.00%)");
  assert.equal(change(2048, 1024), "−1.00 KiB (−50.00%)");
  assert.equal(change(1024, 1024), "0 B (0.00%)");
  assert.equal(change(0, 1024), "+1.00 KiB (new)");
});

test("one report contains both platforms, provenance, and explicit missing results", () => {
  const report = { sha: trigger.head_sha, results: [
    { platform: "iOS", status: "Ready", run: trigger, baseline: trigger,
      before: { vm: 1024, file: 2048 }, after: { vm: 2048, file: 2048 }, diff: "header\n+1 Ki TOTAL\n" },
    { platform: "Linux", status: "No size artifact (CI: failure)" },
  ] };
  const comment = formatReport(report, "https://example.com/report.md");
  assert.match(comment, /\| iOS \|.*\+1.00 KiB.*0 B/);
  assert.match(comment, /\| Linux \| — \| — \| — \| No size artifact/);
  assert.match(comment, /<details>\n<summary>iOS details<\/summary>\n\n/);
  assert.match(comment, /\[Full iOS and Linux report\]\(https:\/\/example.com\/report.md\)/);
  const full = formatReport(report, "", true);
  assert.match(full, /### iOS/);
  assert.match(full, /### Linux/);
  assert.match(full, /2026-10-02T12:00:00Z/);
  assert.doesNotMatch(full, /<details>|Full iOS and Linux report/);
});

test("comment excerpts preserve totals while the full report retains every compile unit", () => {
  const diff = Array.from({ length: 30 }, (_, i) => `source-${i}.cpp`).join("\n") + "\n+1Ki TOTAL\n";
  const report = { sha: trigger.head_sha, results: [{ platform: "Linux", status: "Ready", diff }] };
  assert.match(formatReport(report, "report"), /…\n\+1Ki TOTAL/);
  assert.doesNotMatch(formatReport(report, "report"), /source-25/);
  assert.match(formatReport(report, "", true), /source-25/);
});

test("compile-unit backticks cannot close the report code fence", () => {
  const report = { sha: trigger.head_sha, results: [{ platform: "iOS", status: "Ready", diff: "````\nsource.cpp" }] };
  assert.match(formatReport(report, "report"), /`````text\n````\nsource.cpp\n`````/);
});

test("PR discovery ignores stale revisions and PRs from other forks", async () => {
  const octokit = {
    rest: { pulls: { list: {} } },
    paginate: async () => [
      { number: 1, head: { sha: "old", ref: "feature", repo: { id: 2 } } },
      { number: 2, head: { sha: trigger.head_sha, ref: "feature", repo: { id: 3 } } },
      { number: 3, head: { sha: trigger.head_sha, ref: "feature", repo: { id: 2 } } },
    ],
  } as unknown as Octokit;
  assert.equal((await findPullRequest(octokit, repo, trigger))?.number, 3);
  assert.equal(await findPullRequest(octokit, repo, { ...trigger, head_sha: "obsolete" }), undefined);
});

test("baseline discovery skips fork, PR, expired and unrelated workflow artifacts across pages", async () => {
  const workflowRun = { id: 20, head_branch: "main", head_repository_id: 1, repository_id: 1 };
  const artifact = { id: 1, name: "mbgl-render", expired: false, workflow_run: workflowRun };
  const requested: number[] = [];
  const octokit = {
    rest: { actions: {
      listArtifactsForRepo: {},
      getWorkflowRun: async ({ run_id }: { run_id: number }) => {
        requested.push(run_id);
        return { data: { ...trigger, id: run_id, event: run_id === 20 ? "pull_request" : "push",
          path: run_id === 21 ? ".github/workflows/other.yml" : ".github/workflows/linux-ci.yml" } };
      },
    } },
    paginate: { iterator: async function* () {
      yield { data: [
        { ...artifact, expired: true },
        { ...artifact, workflow_run: { ...workflowRun, head_repository_id: 2 } },
        artifact,
        { ...artifact, workflow_run: { ...workflowRun, id: 21 } },
      ] };
      yield { data: [{ ...artifact, id: 2, workflow_run: { ...workflowRun, id: 22 } }] };
    } },
  } as unknown as Octokit;
  const baseline = await findBaseline(octokit, repo, linux);
  assert.deepEqual(requested, [20, 21, 22]);
  assert.equal(baseline?.run.id, 22);
  assert.equal(baseline?.artifact.id, 2);
});

test("a failed parent with no artifact yields an explicit status", async () => {
  const listRuns = {};
  const listArtifacts = {};
  const octokit = {
    rest: { actions: { listWorkflowRuns: listRuns, listWorkflowRunArtifacts: listArtifacts } },
    paginate: async (method: unknown) => method === listRuns ? [{ ...trigger, conclusion: "failure" }] : [],
  } as unknown as Octokit;
  assert.equal((await analyze(octokit, repo, trigger, linux)).status, "No size artifact (CI: failure)");
});

test("an artifact from an older successful run is not used while a newer run is pending", async () => {
  const octokit = {
    rest: { actions: { listWorkflowRuns: {} } },
    paginate: async () => [trigger, { ...trigger, id: 11, status: "in_progress", conclusion: null }],
  } as unknown as Octokit;
  assert.equal((await analyze(octokit, repo, trigger, linux)).status, "Waiting for CI");
});

test("a platform API error is returned as a result so the other platform can still report", async () => {
  const octokit = {
    rest: { actions: { listWorkflowRuns: {} } },
    paginate: async () => { throw new Error("artifact API unavailable"); },
  } as unknown as Octokit;
  assert.equal((await analyze(octokit, repo, trigger, linux)).status, "Analysis failed (see workflow logs)");
});
