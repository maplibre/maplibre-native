#!/usr/bin/env node

// Generates the local Hvigor signing config consumed by ../hvigorfile.ts.
// Hvigor has some weird scheme where it encrypts the signing password and
// then requires keys in a sibling directory material/* to decrypt it.

import crypto from "node:crypto";
import fs from "node:fs";
import path from "node:path";
import readline from "node:readline";
import { fileURLToPath } from "node:url";

const signDir = path.dirname(fileURLToPath(import.meta.url));
const sampleDir = path.resolve(signDir, "..");
const materialDir = path.join(signDir, "material");
const signingConfigPath = path.join(signDir, "signing.local.json");

// Mirrors Hvigor's DecipherUtil.component constant, used when deriving the
// local signing-material root key.
const component = Buffer.from([
  49, 243, 9, 115, 214, 175, 91, 184, 211, 190, 177, 88, 101, 131, 192, 119,
]);

const defaults = {
  certpath: "./sign/maplibre-debug.cer",
  keyAlias: "maplibre_debug",
  profile: "./sign/maplibre-debug.p7b",
  signAlg: "SHA256withECDSA",
  storeFile: "./sign/maplibre-debug.p12",
  type: "HarmonyOS",
};

function parseArgs(argv) {
  const args = { ...defaults };
  for (let index = 0; index < argv.length; index += 1) {
    const arg = argv[index];
    const value = argv[index + 1];
    switch (arg) {
      case "--certpath":
      case "--keyAlias":
      case "--profile":
      case "--signAlg":
      case "--storeFile":
      case "--type":
        if (!value) {
          throw new Error(`${arg} requires a value`);
        }
        args[arg.slice(2)] = value;
        index += 1;
        break;
      default:
        throw new Error(`Unknown argument: ${arg}`);
    }
  }
  return args;
}

function resolveFromSample(filePath) {
  return path.isAbsolute(filePath)
    ? filePath
    : path.resolve(sampleDir, filePath);
}

async function promptHidden(prompt) {
  const input = process.stdin;
  const output = process.stdout;
  const rl = readline.createInterface({ input, output });
  const originalWrite = output.write;
  let muted = false;

  output.write = function writeMuted(chunk, encoding, callback) {
    if (muted && chunk !== "\n") {
      return true;
    }
    return originalWrite.call(output, chunk, encoding, callback);
  };

  try {
    return await new Promise((resolve) => {
      rl.question(prompt, (answer) => {
        output.write("\n");
        resolve(answer);
      });
      muted = true;
    });
  } finally {
    output.write = originalWrite;
    rl.close();
  }
}

function xor(left, right) {
  const result = Buffer.alloc(left.length);
  for (let index = 0; index < left.length; index += 1) {
    result[index] = left[index] ^ right[index];
  }
  return result;
}

function encryptPacket(key, plaintext) {
  const iv = crypto.randomBytes(12);
  const cipher = crypto.createCipheriv("aes-128-gcm", key, iv);
  const encrypted = Buffer.concat([cipher.update(plaintext), cipher.final()]);
  const authTag = cipher.getAuthTag();
  const header = Buffer.alloc(4);
  header.writeUInt32BE(encrypted.length + authTag.length, 0);
  return Buffer.concat([header, iv, encrypted, authTag]);
}

function writeMaterialFile(directory, data) {
  const filename = crypto.randomBytes(16).toString("hex");
  fs.writeFileSync(path.join(directory, filename), data, { mode: 0o600 });
}

function generateMaterial() {
  fs.rmSync(materialDir, { force: true, recursive: true });
  for (const directory of ["fd/0", "fd/1", "fd/2", "ac", "ce"]) {
    fs.mkdirSync(path.join(materialDir, directory), { recursive: true });
  }

  const fd0 = crypto.randomBytes(16);
  const fd1 = crypto.randomBytes(16);
  const fd2 = crypto.randomBytes(16);
  const salt = crypto.randomBytes(16);
  const workKey = crypto.randomBytes(16);

  let rootComponent = xor(fd0, fd1);
  rootComponent = xor(rootComponent, fd2);
  rootComponent = xor(rootComponent, component);
  const rootKey = crypto.pbkdf2Sync(
    rootComponent.toString(),
    salt,
    10000,
    16,
    "sha256",
  );

  writeMaterialFile(path.join(materialDir, "fd/0"), fd0);
  writeMaterialFile(path.join(materialDir, "fd/1"), fd1);
  writeMaterialFile(path.join(materialDir, "fd/2"), fd2);
  writeMaterialFile(path.join(materialDir, "ac"), salt);
  writeMaterialFile(
    path.join(materialDir, "ce"),
    encryptPacket(rootKey, workKey),
  );

  return workKey;
}

function encryptPassword(workKey, password) {
  return encryptPacket(workKey, Buffer.from(password, "utf8"))
    .toString("hex")
    .toUpperCase();
}

async function main() {
  const args = parseArgs(process.argv.slice(2));
  for (const filePath of [args.storeFile, args.certpath, args.profile]) {
    if (!fs.existsSync(resolveFromSample(filePath))) {
      throw new Error(`Missing signing file: ${filePath}`);
    }
  }

  const storePassword =
    process.env.HARMONY_STORE_PASSWORD ||
    (await promptHidden("Keystore password: "));
  const keyPassword =
    process.env.HARMONY_KEY_PASSWORD || (await promptHidden("Key password: "));
  if (!storePassword || !keyPassword) {
    throw new Error("Both passwords are required");
  }

  const workKey = generateMaterial();
  const signingConfig = {
    type: args.type,
    material: {
      storeFile: args.storeFile,
      storePassword: encryptPassword(workKey, storePassword),
      keyAlias: args.keyAlias,
      keyPassword: encryptPassword(workKey, keyPassword),
      signAlg: args.signAlg,
      profile: args.profile,
      certpath: args.certpath,
    },
  };

  fs.writeFileSync(
    signingConfigPath,
    `${JSON.stringify(signingConfig, null, 2)}\n`,
    { mode: 0o600 },
  );
  console.log(`Wrote ${path.relative(sampleDir, signingConfigPath)}`);
}

main().catch((error) => {
  console.error(error instanceof Error ? error.message : String(error));
  process.exit(1);
});
