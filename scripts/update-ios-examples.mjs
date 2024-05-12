import fs from "fs/promises";

const documentationPaths = [
  "platform/ios/MapLibre.docc/GettingStarted.md",
  "platform/ios/MapLibre.docc/LineOnUserTap.md",
];

// source files that contain examples
const examplePaths = [
  "platform/ios/app-swift/Sources/SimpleMapView.swift",
  "platform/ios/app-swift/Sources/LineTapMapView.swift"
];

/**
 * @param {string} examplePath
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
 * 
 * @param {string} documentationPath 
 * @param {Record<string, string>} examples 
 */
async function updateDocumentation(documentationPath, examples) {
  const content = await fs.readFile(documentationPath, 'utf8');
  const lines = content.split('\n');

  /** @type {{tag: 'find-example'} | {tag: 'find-codeblock', id: string} | {tag: 'skip-to-codeblock-end'}} */
  let status = {tag: 'find-example'};
  let currentId = '';
  let outputLines = [];

  for (const line of lines) {
    if (status.tag === 'find-example') {
      const match = line.match(/<!-- include-example\(([^)]+)\) -->/);
      if (match) {
        currentId = match[1];
        status = {tag: 'find-codeblock', id: currentId};
      }
      outputLines.push(line);
    } else if (status.tag === 'find-codeblock') {
      if (line === '```swift') {
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
        status = {tag: 'find-example'};
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