// EJS Helpers

import ejs from "ejs";
import fs from "node:fs";

import path from "node:path";

/**
 * @param {() => boolean} condition
 * @param {string} val
 * @returns {string}
 */
export function iff(condition, val) {
  return condition() ? val : "";
};

/**
 * @param {string} str
 * @returns {string}
 */
export function camelize(str) {
  return str.replace(/(?:^|-)(.)/g, function (_, x) {
    return x.toUpperCase();
  });
};

/**
 * @param {string} str
 * @returns {string}
 */
export function camelizeWithLeadingLowercase(str) {
  return str.replace(/-(.)/g, function (_, x) {
    return x.toUpperCase();
  });
};

export function snakeCaseUpper(/** @type {string} **/ str) {
  return str.replace(/-/g, "_").toUpperCase();
};

export function unhyphenate (/** @type {string} **/ str) {
 return str.replace(/-/g, " ");
};

/**
 *
 * @param {string} filename
 * @param {string} newContent
 * @param {string} output
 */
export function writeIfModified(filename, newContent, output) {
  if (output) {
    filename = path.resolve(path.join(output, filename));
  }

  const info = path.parse(filename);
  if (!fs.existsSync(info.dir)) {
    fs.mkdirSync(info.dir, {recursive: true});
  }

  try {
    const oldContent = fs.readFileSync(filename, 'utf8');
    if (oldContent == newContent) {
      console.warn(`* Skipping file '${filename}' because it is up-to-date`);
      return;
    }
  } catch(err) {
  }
  if (['0', 'false'].indexOf(process.env.DRY_RUN || '0') !== -1) {
    fs.writeFileSync(filename, newContent);
  }
  console.warn(`* Updating outdated file '${filename}'`);
};

/**
 *
 * @param {string} filename
 * @param {string} root
 * @returns
 */
export function readAndCompile(filename, root) {
  if (root) {
    filename = path.resolve(path.join(root, filename));
  }

  return ejs.compile(fs.readFileSync(filename, 'utf8'), {strict: true});
}
