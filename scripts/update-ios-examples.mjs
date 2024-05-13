/**
 * This script updates the iOS examples part of the DocC documentation.
 * See platform/ios/CONTRIBUTING.md for the details.
 */

import fs from "fs/promises";

/** documentation files that will be updated */
const documentationPaths = [
  "platform/ios/MapLibre.docc/GettingStarted.md",
  "platform/ios/MapLibre.docc/LineOnUserTap.md",
  "platform/ios/MapLibre.docc/LocationPrivacyExample.md",
];

/** source files that contain examples */
const examplePaths = [
  "platform/ios/app-swift/Sources/SimpleMapView.swift",
  "platform/ios/app-swift/Sources/LineTapMapView.swift",
  "platform/ios/app-swift/Sources/LocationPrivacyExample.swift",
];

/**
 * Parses a source file and looks for examples fenced in
 * ```
 * // #-example-code(ExampleId)
 * ...
 * // #-end-example-code
 * ```
 * @param {string} examplePath
 * @returns Mapping from example id to example, e.g. {"ExampleId": "..."}
 */
async function parseExampleCode(examplePath) {
  const content = await fs.readFile(examplePath, "utf8");
  const lines = content.split("\n");
  /** @type {Record<string, string>} */
  const exampleCodeMap = {};

  let capturing = false;
  let currentId = "";

  for (const line of lines) {
    if (capturing) {
      if (line.includes("// #-end-example-code")) {
        capturing = false;
      } else {
        if (!exampleCodeMap[currentId]) {
          exampleCodeMap[currentId] = "";
        }
        exampleCodeMap[currentId] += line + "\n";
      }
    } else {
      const match = line.match(/\/\/ #\-example-code\(([^)]+)\)/);
      if (match) {
        currentId = match[1];
        capturing = true;
      }
    }
  }

  return exampleCodeMap;
}

/**
 * @returns Record of all example id to example code
 */
async function loadExamples() {
  /** @type {Record<string, string>} */
  let examples = {};
  for (const examplePath of examplePaths) {
    const newExamples = await parseExampleCode(examplePath);
    if (Object.keys(newExamples).length === 0) {
      throw new Error(`File '${examplePath}' does not contain any examples`);
    }
    examples = {...newExamples, ...examples};
  }
  return examples;
}

/**
 * Replaces code block in examples with actual example in source code.
 * 
 * @param {string} documentationPath 
 * @param {Record<string, string>} examples 
 */
async function updateDocumentation(documentationPath, examples) {
  const content = await fs.readFile(documentationPath, 'utf8');
  const lines = content.split('\n');

  // This status implements little state machine.
  /** @type {{tag: 'find-example'} | {tag: 'find-codeblock', id: string} | {tag: 'skip-to-codeblock-end'}} */
  let status = {tag: 'find-example'};
  let currentId = '';
  let outputLines = [];

  for (const line of lines) {
    if (status.tag === 'find-example') {  // look for the start of an example
      const match = line.match(/<!-- include-example\(([^)]+)\) -->/);
      if (match) { // found example
        currentId = match[1];
        status = {tag: 'find-codeblock', id: currentId};
      }
      outputLines.push(line); // otherwise include the line as-is
    } else if (status.tag === 'find-codeblock') {  // look for a code block to replace
      if (line === '```swift') {  // found it!
        outputLines.push(line);
        if (!examples[status.id]) throw Error(`Expected example with id='${status.id}'`);
        outputLines.push(examples[status.id].trim());
        status = {tag: 'skip-to-codeblock-end'};
      } else {
        outputLines.push(line);
      }
    } else if (status.tag === 'skip-to-codeblock-end') {
      if (line === '```') {
        outputLines.push(line);
        status = {tag: 'find-example'};  // look for the next example
      } else {
        continue;
      }
    }
  }

  if (status.tag === 'find-codeblock') {
    throw Error(`Ended evalution while looking for codeblock with id '${status.id}'`);
  }

  if (status.tag === 'skip-to-codeblock-end') {
    throw Error("Could not find end of codeblock");
  }

  await fs.writeFile(documentationPath, outputLines.join('\n'));
}

const examples = await loadExamples();
console.log(`Found examples: ${Object.keys(examples).join(", ")}`);

for (const documentationPath of documentationPaths) {
  await updateDocumentation(documentationPath, examples);
}