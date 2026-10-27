import { code } from '../../code.ts';

export const scope = 'plugins';

// The path of the file in plugins/android.
export const path = () => '.gitignore';

export default () => code`
/*/build/
/*/.cxx/
/*/.gradle/
`;
