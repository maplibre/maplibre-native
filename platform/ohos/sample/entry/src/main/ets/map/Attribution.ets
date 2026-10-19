export interface AttributionItem {
  key: string;
  label: string;
  url?: string;
}

function decodeHtmlEntities(value: string): string {
  return value
    .replace(/&copy;/g, '(c)')
    .replace(/&amp;/g, '&')
    .replace(/&lt;/g, '<')
    .replace(/&gt;/g, '>')
    .replace(/&quot;/g, '"')
    .replace(/&#39;/g, "'")
    .replace(/\s+/g, ' ')
    .trim();
}

function stripHtml(value: string): string {
  return decodeHtmlEntities(value.replace(/<[^>]*>/g, ''));
}

function appendItem(items: AttributionItem[], label: string, url?: string): void {
  const trimmed = label.replace(/\s+/g, ' ').trim();
  if (trimmed.length === 0) {
    return;
  }

  items.push({
    key: items.length.toString() + ':' + trimmed + ':' + (url ?? ''),
    label: trimmed,
    url: url
  });
}

function appendHtml(items: AttributionItem[], value: string): void {
  const anchorPattern: RegExp = /<a[^>]*href=["']([^"']+)["'][^>]*>(.*?)<\/a>/g;
  let cursor = 0;
  let match: RegExpExecArray | null = anchorPattern.exec(value);
  while (match !== null) {
    if (match.index > cursor) {
      appendItem(items, stripHtml(value.slice(cursor, match.index)));
    }

    appendItem(items, stripHtml(match[2]), decodeHtmlEntities(match[1]));
    cursor = match.index + match[0].length;
    match = anchorPattern.exec(value);
  }

  if (cursor < value.length) {
    appendItem(items, stripHtml(value.slice(cursor)));
  }
}

export function parseAttributions(values: string[]): AttributionItem[] {
  const items: AttributionItem[] = [];
  const seen: string[] = [];
  values.forEach((value: string) => {
    const trimmed = value.trim();
    if (trimmed.length === 0 || seen.indexOf(trimmed) >= 0) {
      return;
    }

    if (items.length > 0) {
      appendItem(items, '|');
    }
    seen.push(trimmed);
    appendHtml(items, trimmed);
  });
  return items;
}
