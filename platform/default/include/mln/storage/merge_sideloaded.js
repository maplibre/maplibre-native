var fs = require('fs');
fs.writeFileSync('platform/default/mln/storage/merge_sideloaded.hpp', `#pragma once

// THIS IS A GENERATED FILE; EDIT merge_sideloaded.sql INSTEAD
// To regenerate, run \`node platform/default/mln/storage/merge_sideloaded.js\`

namespace mln {

static constexpr const char* mergeSideloadedDatabaseSQL =
${fs.readFileSync('platform/default/mln/storage/merge_sideloaded.sql', 'utf8')
    .replace(/ *--.*/g, '')
    .split('\n')
    .filter(a => a)
    .map(line => '"' + line + '\\n"')
    .join('\n')
}
;

} // namespace mln
`);
