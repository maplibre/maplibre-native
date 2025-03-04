import { parseArgs } from "node:util";
import * as fs from "node:fs/promises";
import * as path from "node:path";
import { S3Client, PutObjectCommand } from "@aws-sdk/client-s3";

/**
 * @returns {never}
 */
function usage() {
  console.error("Uploads a directory of benchmark results (JSON files) to S3");
  console.error("Usage: node upload-benchmark-outputs-to-s3.mjs --dir DIR");
  process.exit(1);
}

function getArgs() {
  const {
    values
  } = parseArgs({
    options: {
      dir: {
        type: "string",
      },
    },
  });
  const { dir } = values;
  if (typeof dir !== 'string') usage();

  return {
    dir
  }
}

const { dir } = getArgs();

processFiles(dir);

///////////////////////////////////////////////////////////////////////////////

/**
 *
 * @param {string} filePath
 * @param {string} gitRevision
 * @param {string} filename
 */
async function uploadFileToS3(filePath, gitRevision, filename) {
  const s3Client = new S3Client({ region: "eu-central-1" });
  const bucket = "maplibre-native";
  const key = `android-benchmark-render/${gitRevision}/${filename}`;

  try {
    const fileContent = await fs.readFile(filePath);
    const command = new PutObjectCommand({
      Bucket: bucket,
      Key: key,
      Body: fileContent,
    });
    await s3Client.send(command);
    console.log(`Uploaded ${filename} to s3://${bucket}/${key}`);
  } catch (error) {
    console.error(`Failed to upload ${filename}:`, error);
  }
}

/**
 *
 * @param {string} dir
 */
async function processFiles(dir) {
  const files = await fs.readdir(dir);
  await Promise.all(files.map(async (filename) => {
    const filePath = path.join(dir, filename);
    if (path.extname(filename) !== ".json") return;

    try {
      const fileContent = await fs.readFile(filePath, "utf-8");
      const data = JSON.parse(fileContent);
      if (!data.gitRevision) {
        console.error(`File ${filename} does not have a 'gitRevision' key. Skipping.`);
        return;
      }

      await uploadFileToS3(filePath, data.gitRevision, filename);
    } catch (error) {
      console.error(`Error processing file ${filename}:`, error);
    }
  }));
}
