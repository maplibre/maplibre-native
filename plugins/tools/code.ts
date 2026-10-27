// The tag of the file templates in templates/. It interpolates like an untagged template literal, joins arrays, and
// drops the newline after the opening backtick so that a template can start on its own line.
export function code(strings: TemplateStringsArray, ...values: (string | string[])[]): string {
  const text = strings.reduce((result, string, i) => result + string + (i < values.length ? [values[i]].flat().join('') : ''), '');
  return text.startsWith('\n') ? text.slice(1) : text;
}
