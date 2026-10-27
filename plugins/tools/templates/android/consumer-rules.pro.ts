import { code } from '../../code.ts';
import type { AndroidValues } from '../values.ts';

// The path of the file in plugins/android.
export const path = (v: AndroidValues) => `${v.id}/consumer-rules.pro`;

export default (v: AndroidValues) => code`
-keep class ${v.qualifiedClassName} { native <methods>; }
`;
