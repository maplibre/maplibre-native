import { code } from '../../code.ts';
import { androidClass, androidNamespace } from '../names.ts';
import type { Plugin } from '../values.ts';

// The path of the file in plugins/android.
export const path = ({ id }: Plugin) => `${id}/consumer-rules.pro`;

export default ({ id }: Plugin) => code`
-keep class ${androidNamespace(id)}.${androidClass(id)} { native <methods>; }
`;
