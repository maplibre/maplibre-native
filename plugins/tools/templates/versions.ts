import { readFileSync } from 'node:fs';
import { join } from 'node:path';

const versionsFile = join(import.meta.dirname, '../../../platform/android/buildSrc/src/main/kotlin/Versions.kt');

/** The NDK version that the Android SDK builds with. */
export function ndkVersion(): string {
  const version = readFileSync(versionsFile, 'utf8').match(/ndkVersion\s*=\s*"([^"]+)"/)?.[1];
  if (!version) throw new Error(`Cannot find ndkVersion in ${versionsFile}`);
  return version;
}
