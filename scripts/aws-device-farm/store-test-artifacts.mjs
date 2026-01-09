import * as fs from "node:fs";
import { Readable } from "node:stream";
import { finished } from "node:stream/promises";
import * as path from "node:path";
import { parseArgs } from "node:util";

import { ArtifactType, ListArtifactsCommand, ListJobsCommand, ListSuitesCommand } from "@aws-sdk/client-device-farm";
import { getDeviceFarmClient } from "./device-farm-client.mjs";

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

function getArgs() {
  const {
    values
  } = parseArgs({
    options: {
      outputDir: {
        type: "string",
      },
      runArn: {
        type: "string",
        multiple: true
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

  if (!runArn || !runArn.length) usage();

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

if (!fs.existsSync(outputDir)) {
  console.error("Output dir does not exist");
  process.exit(1);
}

const deviceFarmClient = getDeviceFarmClient();
await storeRunArtifacts(runArn, outputDir);

///////////////////////////////////////////////////////////////////////////////

/**
 * Looks for the run with the provided ARN and returns the test spec output.
 *
 * @param {string[]} arnArr
 * @param {string} outputDir
 * @returns string
 */
async function storeRunArtifacts(arnArr, outputDir) {
  for (const arn of arnArr) {
    const jobs = await deviceFarmClient.send(new ListJobsCommand({
      arn
    }));

    await Promise.all((jobs.jobs || []).map(async (job) => {
      const suites = await deviceFarmClient.send(new ListSuitesCommand({ arn: job.arn }));
      await Promise.all((suites.suites || []).filter(suitesFilter).map(async (suite) => {
        const artifacts = await deviceFarmClient.send(new ListArtifactsCommand({
          arn: suite.arn,
          type: 'FILE'
        }));
        await Promise.all((artifacts.artifacts || []).map(async (artifact) => {
          if (!artifact.name || !artifact.url || !artifact.type) return;
          if (artifactsToDownload.includes(artifact.type)) {
            if (!artifact.arn) return;
            const destination = path.join(outputDir, ...[job.device?.name || ""].filter(x => x), `${Buffer.from(artifact.arn).toString('base64')}.${artifact.extension}`);
            await fs.promises.mkdir(path.dirname(destination), { recursive: true });
            try {
              await fs.promises.access(destination);
              return; // already exists
            } catch (err) {
            }
            const res = await fetch(artifact.url);
            if (!res.ok || !res.body) return;
            const fileStream = fs.createWriteStream(destination, { flags: 'wx' });
            await finished(Readable.fromWeb(/** @type {any} **/(res.body)).pipe(fileStream));
          }
        }));
      }));
    }));
  }
  console.log(`Wrote run artifacts to ${outputDir}`)
}
