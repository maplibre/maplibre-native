// This script parses logcat output from AWS Device Farm to extract benchmark results.
// It also uploads the results to S3.
// android-benchmark-render/{gitRevision}/{deviceModel}.json

import {
  DeviceFarmClient,
  ListJobsCommand,
  ListArtifactsCommand,
} from "@aws-sdk/client-device-farm";
import { S3Client, PutObjectCommand } from "@aws-sdk/client-s3";

import https from "node:https";
import readline from "node:readline";

const deviceFarmClient = new DeviceFarmClient({ region: "us-west-2" });

function usage() {
  console.error("Downloads benchmark results from AWS Device Farm");
  console.error(`Usage: node ${process.argv.at(1)} RUN_ARN`);
  process.exit(1);
}

if (process.argv.length !== 3) usage();

const arn = process.argv.at(2);

const listJobsCommand = new ListJobsCommand({ arn });
const { jobs } = await deviceFarmClient.send(listJobsCommand);

const passedJobs = jobs.filter((job) => job.result === "PASSED" && job.arn);
const failedJobs = jobs.filter((job) => job.result !== "PASSED" || !job.arn);

failedJobs.forEach((job) => {
  console.error(
    `job.result not PASSED. Skipping.\n$${JSON.stringify(job, null, 0)}`
  );
});

const artifacts = (
  await Promise.all(
    passedJobs.map((job) =>
      deviceFarmClient.send(
        new ListArtifactsCommand({
          arn: job.arn,
          type: "FILE",
        })
      )
    )
  )
).flatMap((listArtifactsOutput) =>
  listArtifactsOutput.artifacts.filter(
    (artifact) => artifact.type === "DEVICE_LOG" && artifact.name === "Logcat"
  )
);

/**
 * @param {string} artifactUrl 
 * @returns {Promise<string>}
 */
function getBenchmarkResult(artifactUrl) {
  return new Promise((resolve, reject) => {
    https.get(artifactUrl, res => {
      const rl = readline.createInterface({
        input: res
      });
  
      rl.on("line", line => {
        if (line.includes('Benchmark {"resultsPerStyle"')) {
          resolve(line);
          rl.close();
        }
      });

      rl.on("close", () => {
        resolve("");
      })
    });
  });
}

const benchmarkResultLogLines = await Promise.all(artifacts.map(artifact => getBenchmarkResult(artifact.url)));

/** @type {{gitRevision?: string, model?: string}[]} */
const benchmarkResults = benchmarkResultLogLines.map(line => {
  const firstCurly = line.indexOf("{");
  if (firstCurly === -1) return "";
  return line.substring(firstCurly);
}).filter(line => !!line).map(jsonVal => JSON.parse(jsonVal));

console.log(benchmarkResults);

const s3Client = new S3Client({
  region: "eu-central-1"
});

await Promise.all(benchmarkResults.flatMap(result => {
  if (typeof result.model !== 'string') {
    console.error("Result missing model");
    return [];
  }
  if (typeof result.gitRevision !== 'string') {
    console.error("Result missing gitRevision");
    return [];
  }

  const command = new PutObjectCommand({
    Bucket: 'maplibre-native', // use environment variable
    Key: `android-benchmark-render/${result.gitRevision}/${result.model}.json`,
    ContentType: 'application/json',
    Body: JSON.stringify(result, null, 2),
  });
  return [s3Client.send(command)];
}));
