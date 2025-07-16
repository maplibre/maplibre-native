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

  const job = data.jobs.find(({name}) => name === jobName);
  if (!job) throw new Error(`job with name ${jobName} not found in workflow run with id ${run_id}`);

  core.setOutput('was_skipped', job.conclusion === 'skipped');
}

try {
  await run();
} catch (err) {
  core.setFailed(err instanceof Error ? err.message: `${err}`);
}
