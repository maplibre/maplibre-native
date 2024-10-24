import * as fs from "node:fs";
import { Readable } from "node:stream";
import { finished } from "node:stream/promises";
import * as path from "node:path";
import * as crypto from "node:crypto";
import { parseArgs } from "node:util";

import { ArtifactType, ListArtifactsCommand, ListJobsCommand, ListSuitesCommand } from "@aws-sdk/client-device-farm";
import { getDeviceFarmClient } from "./device-farm-client.mjs";

function getArgs() {
  const {
    values
  } = parseArgs({
    options: {
      outputDir: {
        type: "string",
      },
      runArn: {
        type: "string"
      },
      testsSuite: {
       type: "boolean" 
      },
      customerArtifacts: {
        type: "boolean"
      }
    },
  });
  const { outputDir, runArn } = values;
  if (typeof outputDir !== 'string') usage();

  if (typeof runArn !== 'string') usage();

  function suitesFilter() {
    const names = new Set();
    if (values.testsSuite) names.add("Tests Suite");

    if (names.size === 0) return () => true;

    return (/** @type {string} **/ name) => names.has(name);
  }

  /** @type {() => ArtifactType[]} **/
  function getArtifactsToDownload() {
    if (values.customerArtifacts) return ["CUSTOMER_ARTIFACT"];
    return ['TESTSPEC_OUTPUT', 'CUSTOMER_ARTIFACT', 'CUSTOMER_ARTIFACT_LOG', 'DEVICE_LOG'];
  }

  return {
    outputDir,
    runArn,
    suitesFilter,
    artifactsToDownload: getArtifactsToDownload()
  }
}

const { outputDir, runArn, suitesFilter, artifactsToDownload } = getArgs();

/**
 * @returns {never}
 */
function usage() {
  console.error("Stores artifacts from AWS Device Farm run");
  console.error("Usage: node store-test-artifacts.mjs --outputDir OUTPUT_DIR --runArn RUN_ARN");
  console.error("Arguments:")
  console.error("--customerArtifacts: only download customer artifacts");
  console.error("--testsSuite: only download stuff from Tests Suite");
  process.exit(1);
}

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
    await Promise.all((suites.suites || []).filter(suitesFilter).map(async (suite) => {
      const artifacts = await deviceFarmClient.send(new ListArtifactsCommand({
        arn: suite.arn,
        type: 'FILE'
      }));
      await Promise.all((artifacts.artifacts || []).map(async (artifact) => {
        if (!artifact.name || !artifact.url || !artifact.type) return;
        if (artifactsToDownload.includes(artifact.type)) {
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

await getTestSpecOutput(runArn);
