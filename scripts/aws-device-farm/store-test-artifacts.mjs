import * as fs from "node:fs";
import { Readable } from "node:stream";
import { finished } from "node:stream/promises";
import * as path from "node:path";
import * as crypto from "node:crypto";

import { ListArtifactsCommand, ListJobsCommand, ListSuitesCommand } from "@aws-sdk/client-device-farm";
import { getDeviceFarmClient } from "./device-farm-client.mjs";

/**
 * @returns {never}
 */
function usage() {
  console.error("Stores artifacts from AWS Device Farm run");
  console.error(`Usage: node ${process.argv.at(1)} RUN_ARN OUTPUT_DIR`);
  process.exit(1);
}

if (process.argv.length !== 4) usage();

const arn = process.argv.at(2);
const outputDirArg = process.argv.at(3);

if (typeof arn !== 'string') usage();
if (typeof outputDirArg !== 'string') usage();

const outputDir = outputDirArg;

if (!fs.existsSync(outputDir)) {
  console.error("Output dir does not exist");
  process.exit(1);
}

const deviceFarmClient = getDeviceFarmClient();

/**
 * Looks for the run with the provided ARN and returns the test spec output.
 * 
 * @param {string} arn 
 * @returns string
 */
async function getTestSpecOutput(arn) {
  const jobs = await deviceFarmClient.send(new ListJobsCommand({
    arn
  }));
  
  await Promise.all((jobs.jobs || []).map(async (job) => {
    const suites = await deviceFarmClient.send(new ListSuitesCommand({arn: job.arn}));
    await Promise.all((suites.suites || []).map(async (suite) => {
      const artifacts = await deviceFarmClient.send(new ListArtifactsCommand({
        arn: suite.arn,
        type: 'FILE'
      }));
      await Promise.all((artifacts.artifacts || []).map(async (artifact) => {
        if (!artifact.name || !artifact.url || !artifact.type) return;
        if (['TESTSPEC_OUTPUT', 'CUSTOMER_ARTIFACT', 'CUSTOMER_ARTIFACT_LOG', 'DEVICE_LOG'].includes(artifact.type)) {
          const filename = `${artifact.name.replaceAll(' ', '_')}-${crypto.randomBytes(10).toString('hex')}.${artifact.extension}`;
          const res = await fetch(artifact.url);
          if (!res.ok || !res.body) return;
          const destination = path.resolve(outputDir, filename);
          const fileStream = fs.createWriteStream(destination, { flags: 'wx' });
          await finished(Readable.fromWeb(/** @type {any} **/ (res.body)).pipe(fileStream));
        }
      }));
    }));
  }));
}

await getTestSpecOutput(arn);
