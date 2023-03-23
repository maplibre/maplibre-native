import {run} from '../../../metrics/integration/lib/render';
import implementation from './suite_implementation';
import ignores from './ignores.json';

run('native', ignores, implementation);
