import { AbilityConstant, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { window } from '@kit.ArkUI';

const LOG_DOMAIN: number = 0x4d4c4e;
const LOG_TAG: string = 'MapLibreSample';

export default class EntryAbility extends UIAbility {
  onCreate(_want: Want, _launchParam: AbilityConstant.LaunchParam): void {
    hilog.info(LOG_DOMAIN, LOG_TAG, 'Ability created');
  }

  onDestroy(): void {
    hilog.info(LOG_DOMAIN, LOG_TAG, 'Ability destroyed');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    windowStage.loadContent('pages/Index', (error) => {
      if (error.code) {
        hilog.error(LOG_DOMAIN, LOG_TAG, 'Could not load page: %{public}s', JSON.stringify(error));
      }
    });
  }
}

