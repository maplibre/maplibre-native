import * as core from "@actions/core";
import { Octokit } from "@octokit/rest";
import { resolve } from "node:path";
import { pathToFileURL } from "node:url";

export function jobSkipState(jobs: { name: string; conclusion: string | null }[], name: string) {
  const matching = jobs.filter(job => job.name === name || job.name.startsWith(`${name} (`));
  // Cancelled/invalid runs may never create any build jobs. Check all matrix
  // siblings rather than whichever job the API happens to return first.
  return {
    was_skipped: matching.length === 0 || matching.every(job => job.conclusion === "skipped"),
    was_skipped_or_cancelled: matching.length === 0 || matching.every(job =>
      job.conclusion === "skipped" || job.conclusion === "cancelled"
    ),
  };
}

async function run() {
  const octokit = new Octokit({ auth: process.env.GITHUB_TOKEN });

  const run_id = process.env.TEST_RUN_ID;
  if (!run_id) throw new Error("TEST_RUN_ID not set");

  const [owner, repo] = (process.env.GITHUB_REPOSITORY ?? "maplibre/maplibre-native").split("/");
  const jobs = await octokit.paginate(octokit.rest.actions.listJobsForWorkflowRun, {
    owner,
    repo,
    run_id: parseInt(run_id),
    per_page: 100,
  });

  const jobName = process.env.JOB_NAME;
  if (!jobName) throw new Error("JOB_NAME not set");

  const state = jobSkipState(jobs, jobName);
  core.setOutput('was_skipped', state.was_skipped);
  core.setOutput('was_skipped_or_cancelled', state.was_skipped_or_cancelled);
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  run().catch(err => core.setFailed(err instanceof Error ? err.message : String(err)));
}
