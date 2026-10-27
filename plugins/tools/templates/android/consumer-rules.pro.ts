import { code } from '../../code.ts';
import { androidClass, androidNamespace } from '../names.ts';
import type { AndroidValues } from '../values.ts';

// The path of the file in plugins/android.
export const path = ({ plugin: { id } }: AndroidValues) => `${id}/consumer-rules.pro`;

export default ({ plugin: { id } }: AndroidValues) => code`
-keep class ${androidNamespace(id)}.${androidClass(id)} { native <methods>; }
`;
