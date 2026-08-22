import { existsSync, readFileSync } from 'fs';
import { resolve } from 'path';
import { appTasks } from '@ohos/hvigor-ohos-plugin';

const localSigningConfigPath = resolve(__dirname, 'sign/signing.local.json');
const localSigningConfig = existsSync(localSigningConfigPath)
  ? JSON.parse(readFileSync(localSigningConfigPath, 'utf8'))
  : undefined;

export default {
  system: appTasks,
  plugins: [],
  config: localSigningConfig
    ? {
        ohos: {
          overrides: {
            signingConfig: localSigningConfig,
          },
        },
      }
    : {},
};
