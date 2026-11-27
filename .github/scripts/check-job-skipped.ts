import * as core from "@actions/core";
import { Octokit } from "@octokit/rest";

async function run() {
  const octokit = new Octokit({ auth: process.env.GITHUB_TOKEN });

  const run_id = process.env.TEST_RUN_ID;
  if (!run_id) throw new Error("TEST_RUN_ID not set");

  const { data } = await octokit.rest.actions.listJobsForWorkflowRun({
    owner: 'maplibre',
    repo: 'maplibre-native',
    run_id: parseInt(run_id)
  });

  const jobName = process.env.JOB_NAME;
  if (!jobName) throw new Error("JOB_NAME not set");

  // matrix jobs are named `job (values)`; a plain prefix would also match e.g. ios-build-cmake
  const job = data.jobs.find(({name}) => name === jobName || name.startsWith(`${jobName} (`));
  if (!job) throw new Error(`job with name ${jobName} not found in workflow run with id ${run_id}`);

  core.setOutput('was_skipped', job.conclusion === 'skipped');
  core.setOutput('was_skipped_or_cancelled', job.conclusion === 'skipped' || job.conclusion === 'cancelled');
}

try {
  await run();
} catch (err) {
  core.setFailed(err instanceof Error ? err.message: `${err}`);
}
