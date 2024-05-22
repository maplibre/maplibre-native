import { ListArtifactsCommand } from "@aws-sdk/client-device-farm";
import { getDeviceFarmClient } from "./device-farm-client.mjs";

/**
 * @returns {never}
 */
function usage() {
  console.error("Logs test spec output from AWS Device Farm");
  console.error(`Usage: node ${process.argv.at(1)} RUN_ARN`);
  process.exit(1);
}

if (process.argv.length !== 3) usage();

const arn = process.argv.at(2);

if (typeof arn !== 'string') usage();

const deviceFarmClient = getDeviceFarmClient();

/**
 * Looks for the run with the provided ARN and returns the test spec output.
 * 
 * @param {string} arn 
 * @returns string
 */
async function getTestSpecOutput(arn) {
  const logs = await deviceFarmClient.send(new ListArtifactsCommand({
    arn,
    type: 'FILE'
  }));
  for (const artifact of logs.artifacts || []) {
    if (artifact.type === 'TESTSPEC_OUTPUT' && artifact.url) {
      return await (await fetch(artifact.url)).text();
    }
  }
  return "";
}

const testSpecOutput = await getTestSpecOutput(arn);

if (!testSpecOutput) {
  console.error(`Test spec output for run '${arn}' not found`);
}

console.log(testSpecOutput);

