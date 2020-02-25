var spec = module.exports = require('../vendor/mapbox-gl-native/mapbox-gl-js/src/style-spec/reference/v8');

// FIXME: https://github.com/mapbox/mapbox-gl-native/issues/15008
delete spec.layout_circle["circle-sort-key"];
