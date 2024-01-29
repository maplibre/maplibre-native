// Global functions //

const ejs = require('ejs');
const fs = require('fs');
const path = require('path');

global.iff = function (condition, val) {
  return condition() ? val : "";
};

global.camelize = function (str) {
  return str.replace(/(?:^|-)(.)/g, function (_, x) {
    return x.toUpperCase();
  });
};

global.camelizeWithLeadingLowercase = function (str) {
  return str.replace(/-(.)/g, function (_, x) {
    return x.toUpperCase();
  });
};

global.snakeCaseUpper = function snakeCaseUpper(str) {
  return str.replace(/-/g, "_").toUpperCase();
};

global.unhyphenate = function (str) {
 return str.replace(/-/g, " ");
};

// Write out a list of files that this script is modifying so that we can check
var files = [];
process.on('exit', function() {
  const list = path.join(path.dirname(process.argv[1]), path.basename(process.argv[1], '.js') + '.list');
  console.log(`Writing files that this script modifies to: ${list}`);
  fs.writeFileSync(list, files.join("\n"));
});

global.writeIfModified = function(filename, newContent, output) {
  if (output) {
    filename = path.resolve(path.join(output, filename));
  }

  const info = path.parse(filename);
  if (!fs.existsSync(info.dir)) {
    fs.mkdirSync(info.dir, {recursive: true});
  }

  files.push(filename);
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

global.readAndCompile = function(filename, root) {
  if (root) {
    filename = path.resolve(path.join(root, filename));
  }

  return ejs.compile(fs.readFileSync(filename, 'utf8'), {strict: true});
}
