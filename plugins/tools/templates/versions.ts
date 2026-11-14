import { readFileSync } from 'node:fs';
import { join } from 'node:path';

const versionsFile = join(import.meta.dirname, '../../../platform/android/buildSrc/src/main/kotlin/Versions.kt');

function version(name: string): string {
  const version = readFileSync(versionsFile, 'utf8').match(new RegExp(`${name}\\s*=\\s*"([^"]+)"`))?.[1];
  if (!version) throw new Error(`Cannot find ${name} in ${versionsFile}`);
  return version;
}

/** The NDK version that the Android SDK builds with. */
export const ndkVersion = () => version('ndkVersion');

/** The CMake version that the Android SDK builds with. */
export const cmakeVersion = () => version('cmakeVersion');
