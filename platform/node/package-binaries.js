#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const tar = require('tar');

// Read package.json
const packageJson = JSON.parse(fs.readFileSync('package.json', 'utf8'));

// Get platform and architecture mappings
function getPlatform() {
  // Use Node.js built-in process.platform
  switch (process.platform) {
    case 'linux':
      return 'linux';
    case 'darwin':
      return 'darwin';
    case 'win32':
      return 'win32';
    default:
      console.warn(`Unknown platform: ${process.platform}, using as-is`);
      return process.platform;
  }
}

function getArch() {
  // Use Node.js built-in process.arch
  switch (process.arch) {
    case 'x64':
      return 'x64';
    case 'arm64':
      return 'arm64';
    case 'ia32':
      return 'ia32';
    case 'arm':
      return 'arm';
    case 's390x':
      return 's390x';
    case 'ppc64':
      return 'ppc64';
    default:
      console.warn(`Unknown architecture: ${process.arch}, using as-is`);
      return process.arch;
  }
}

// Clean package name for filename
function cleanPackageName(name) {
  let cleanName = name;
  // If it's a scoped package, get the part after the last slash
  if (cleanName.includes('/')) {
    cleanName = cleanName.split('/').pop();
  }
  // Sanitize the name by removing all @ and / characters
  return cleanName.replace(/[@\/]/g, '-');
}

// Create tarballs for each ABI
async function createTarballs() {
  const platform = getPlatform();
  const arch = getArch();
  const version = packageJson.version;
  const cleanName = cleanPackageName(packageJson.name);

  console.log(`Creating tarballs for platform: ${platform}, arch: ${arch}, version: ${version}`);

  // Find all ABI directories
  const libDir = './lib';
  if (!fs.existsSync(libDir)) {
    console.error(`lib directory not found: ${libDir}`);
    process.exit(1);
  }

  const abiDirs = fs.readdirSync(libDir)
    .filter(dir => dir.startsWith('node-v') && fs.statSync(path.join(libDir, dir)).isDirectory())
    .filter(dir => fs.existsSync(path.join(libDir, dir, 'mbgl.node')));

  if (abiDirs.length === 0) {
    console.error('No ABI directories with mbgl.node found');
    process.exit(1);
  }

  console.log(`Found ABI directories: ${abiDirs.join(', ')}`);

  const createdTarballs = [];

  for (const abiDir of abiDirs) {
    const abi = abiDir.replace('node-v', '');
    const tarballName = `${cleanName}-v${version}-node-v${abi}-${platform}-${arch}.tar.gz`;
    const binaryPath = path.join('lib', abiDir, 'mbgl.node');

    console.log(`Creating tarball: ${tarballName}`);
    console.log(`  Platform: ${platform}, Arch: ${arch}, ABI: ${abi}`);
    console.log(`  Binary: ${binaryPath}`);

    try {
      // Create tarball preserving the lib/node-v[abi]/mbgl.node structure
      await tar.create({
        gzip: true,
        file: tarballName,
        cwd: '.',
      }, [binaryPath]);

      // Verify tarball was created and get its size
      const stats = fs.statSync(tarballName);
      console.log(`  Tarball created successfully (${stats.size} bytes)`);
      if (stats.size === 0) {
        throw new Error('Tarball is empty');
      }
      createdTarballs.push(tarballName);
    } catch (error) {
      console.error(`Failed to create tarball ${tarballName}:`, error.message);
      process.exit(1);
    }
  }
  console.log(`\nSuccessfully created ${createdTarballs.length} tarballs:`);
  createdTarballs.forEach(tarball => {
    const stats = fs.statSync(tarball);
    console.log(`  ${tarball} (${stats.size} bytes)`);
  });
  return createdTarballs;
}

// Main execution
if (require.main === module) {
  createTarballs()
    .then(() => {
      console.log('Tarball creation completed successfully');
      process.exit(0);
    })
    .catch(error => {
      console.error('Error creating tarballs:', error);
      process.exit(1);
    });
}

module.exports = {
  createTarballs,
  getPlatform,
  getArch,
  cleanPackageName
};
