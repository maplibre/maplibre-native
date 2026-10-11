"use strict";

// Shim to wrap req.respond while preserving callback-passing API

const mbgl = require("./lib/node-v" + process.versions.modules + "/mbgl");
const constructor = mbgl.Map.prototype.constructor;

const Map = function (options) {
    if (options) {
        if (typeof options !== "object") {
            throw TypeError("Requires an options object as first argument");
        }

        if (options.request) {
            if (typeof options.request !== "function") {
                throw TypeError("Options object 'request' property must be a function");
            }

            const request = options.request;

            return new constructor(
                Object.assign(options, {
                    request: function (req) {
                        // Protect against `request` implementations that call the callback synchronously,
                        // call it multiple times, or throw exceptions.
                        // http://blog.izs.me/post/59142742143/designing-apis-for-asynchrony

                        let responded = false;
                        const callback = function () {
                            const args = arguments;
                            if (!responded) {
                                responded = true;
                                process.nextTick(function () {
                                    req.respond.apply(req, args);
                                });
                            } else {
                                console.warn("request function responded multiple times; it should call the callback only once");
                            }
                        };

                        try {
                            request(req, callback);
                        } catch (e) {
                            console.warn("request function threw an exception; it should call the callback with an error instead");
                            callback(e);
                        }
                    },
                })
            );
        }
        return new constructor(options);
    } else {
        return new constructor();
    }
};

Map.prototype = mbgl.Map.prototype;
Map.prototype.constructor = Map;

module.exports = Object.assign(mbgl, { Map: Map });
