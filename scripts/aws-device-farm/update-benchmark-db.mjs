import { parseArgs, promisify } from "node:util";
import * as path from "node:path";
import os, { type } from "node:os";
import * as fs from "node:fs/promises";
import { exec as execCallback } from "node:child_process";
import { DatabaseSync } from "node:sqlite";

const exec = promisify(execCallback);

/**
 * @returns {never}
 */
function usage() {
  console.error("Update database with benchmark results");
  console.error("Usage: node update-benchmark-db.mjs --dbPath PATH [--gitRevision SHA] [--jsonDir JSON_DIR] [--download]");
  process.exit(1);
}

function getArgs() {
  const {
    values
  } = parseArgs({
    options: {
      dbPath: {
        type: "string"
      },
      gitRevision: {
        type: "string",
      },
      jsonDir: {
        type: "string"
      },
      download: {
        type: "boolean"
      }
    },
  });
  const { gitRevision, jsonDir, download, dbPath } = values;

  if (typeof dbPath !== 'string') {
    console.error("--dbPath is required");
    throw usage();
  }

  return {
    gitRevision,
    jsonDir,
    download,
    dbPath
  }
}

const { gitRevision, jsonDir, download, dbPath } = getArgs();


async function getDir() {
  if (jsonDir) {
    return jsonDir;
  }
  if (!download) {
    console.error("--jsonDir is required when not using --download");
    throw usage();
  }
  return await fs.mkdtemp(path.join(os.tmpdir(), `benchmark-results-${gitRevision}-`));
}

let dir = await getDir();

if (download) await downloadBenchmarkResults(dir);

const results = await loadResults(dir);

updateDb(dbPath, results);

///////////////////////////////////////////////////////////////////////////////

/**
 * @param {string | undefined} gitRevision
 * @returns {Promise<string>}
 */
async function downloadBenchmarkResults(gitRevision) {
  if (!gitRevision) {
    console.error("--gitRevision is required when downloading");
    throw usage();
  }

  const bucket = "maplibre-native";
  const prefix = `android-benchmark-render/${gitRevision}`;

  try {
    // Run the AWS CLI sync command
    const { stdout, stderr } = await exec(`aws s3 sync s3://${bucket}/${prefix} ${dir}`);
    console.log(stdout);
    if (stderr) {
      console.error(`Warning: ${stderr}`);
    }
  } catch (error) {
    throw new Error(`Failed to sync directory s3://${bucket}/${prefix}. ${error}`);
  }

  console.log(`Downloaded benchmark results to ${dir}`);
  return dir;
}

/**
 * @param {string} dir
 * @returns {Promise<Object[]>} An array with parsed JSON objects from each file
 */
async function loadResults(dir) {
  const files = await fs.readdir(dir);

  return (await Promise.all(
    files.flatMap(async (filename) => {
      if (path.extname(filename) !== ".json") return [];
      const filePath = path.join(dir, filename);

      try {
        const fileContent = await fs.readFile(filePath, "utf-8");
        return [JSON.parse(fileContent)];
      } catch (error) {
        console.error(`Failed to load or parse ${filename}:`, error);
        return [];
      }
    })
  )).flat();
}

/**
 * @param {string} dbPath
 * @param {any[]} results 
 */
function updateDb(dbPath, results) {
  const db = new DatabaseSync(dbPath);
  db.exec(`
    CREATE TABLE IF NOT EXISTS benchmark_result (
      id INTEGER PRIMARY KEY,
      styleName TEXT NOT NULL,
      fps REAL NOT NULL,
      avgEncodingTime REAL NOT NULL,
      low1pEncodingTime REAL NOT NULL,
      avgRenderingTime REAL NOT NULL,
      low1pRenderingTime REAL NOT NULL,
      syncRendering BOOLEAN NOT NULL,
      deviceManufacturer TEXT NOT NULL,
      model TEXT NOT NULL,
      renderer TEXT NOT NULL,
      gitRevision TEXT NOT NULL,
      timestamp INTEGER NOT NULL UNIQUE
    )`);

  const stmt = db.prepare(
    `INSERT INTO benchmark_result (
      styleName,
      fps,
      avgEncodingTime,
      low1pEncodingTime,
      avgRenderingTime,
      low1pRenderingTime,
      syncRendering,
      deviceManufacturer,
      model,
      renderer,
      gitRevision,
      timestamp
    ) VALUES (
      @styleName,
      @fps,
      @avgEncodingTime,
      @low1pEncodingTime,
      @avgRenderingTime,
      @low1pRenderingTime,
      @syncRendering,
      @deviceManufacturer,
      @model,
      @renderer,
      @gitRevision,
      @timestamp
  ) ON CONFLICT IGNORE`);

  for (const run of results) {
    for (const result of run.results) {
      const params = {
        ...result,
        ...run,
        syncRendering: result.syncRendering ? 1: 0
      };
      delete params['results'];
      delete params['debugBuild'];
      stmt.run(params);
    }
  }

  return db;
}
