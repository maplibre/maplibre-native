import * as fs from "node:fs/promises";
import { exec } from 'node:child_process';
import { parseArgs, promisify } from "node:util";
import * as path from "node:path";

const execPromise = promisify(exec);

/**
 * @returns {never}
 */
function usage() {
  console.error("Collects benchmark outputs from AWS Device Farm artifacts");
  console.error("Usage: node collect-benchmark-outputs.mjs --inputDir INPUT_DIR --outputDir OUTPUT_DIR");
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
      inputDir: {
        type: "string"
      },
    },
  });
  const { outputDir, inputDir } = values;
  if (typeof outputDir !== 'string') usage();

  if (typeof inputDir !== 'string') usage();

  return {
    outputDir,
    inputDir
  }
}

const { outputDir, inputDir } = getArgs();

await unzipFilesInDirectory(inputDir);
await writeResultsToOutputDir();

///////////////////////////////////////////////////////////////////////////////

/**
 *
 * @param {string} directory
 */
async function unzipFilesInDirectory(directory) {
  try {
    const files = await fs.readdir(directory);
    const zipFiles = files.filter(file => file.endsWith('.zip'));

    for (const zipFile of zipFiles) {
      const zipFilePath = path.join(directory, zipFile);
      const { name } = path.parse(zipFile);
      const extractDir = path.join(directory, name);

      await fs.mkdir(extractDir, { recursive: true });

      await execPromise(`unzip "${zipFilePath}" -d "${extractDir}"`);
      console.log(`Extracted ${zipFile} to ${extractDir}`);
    }

  } catch (error) {
    if (error instanceof Error) console.error(`Error unzipping files: ${error.message}`);
  }
}

async function writeResultsToOutputDir() {
  for await (const benchmarkResultPath of fs.glob("**/benchmark_results.json", {cwd: inputDir})) {
    try {
      const fullBenchmarkResultPath  = path.join(inputDir, benchmarkResultPath);
      const benchmarkResultContents = await fs.readFile(fullBenchmarkResultPath, {encoding: 'utf-8'});
      const benchmarkResults = JSON.parse(benchmarkResultContents);
      const { timestamp } = benchmarkResults;
      if (typeof timestamp !== 'number') throw new Error("No timestamp found in bechmark result");
      await fs.copyFile(fullBenchmarkResultPath, path.join(outputDir, `${timestamp}.json`));
    } catch (err) {
      console.error(`Error ${err}. Skipping: '${benchmarkResultPath}'`);
    }
  }
  console.log(`Wrote benchmark results to ${outputDir}`);
}
