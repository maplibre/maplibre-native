'use strict';

var test = require('tape');
var mbgl = require('../../index');

var style = {
    version: 8,
    state: {
        showLabels: {default: true},
        categories: {default: ['restaurant', 'hotel']}
    },
    sources: {},
    layers: []
};

test('Map#globalState', function(t) {
    t.test('applies defaults from the style root state property', function(t) {
        var map = new mbgl.Map({request: function() {}});
        map.load(style);
        t.deepEqual(map.getGlobalState(), {
            showLabels: true,
            categories: ['restaurant', 'hotel']
        });
        map.release();
        t.end();
    });

    t.test('sets and gets global state properties', function(t) {
        var map = new mbgl.Map({request: function() {}});
        map.load(style);

        map.setGlobalStateProperty('showLabels', false);
        t.equal(map.getGlobalState().showLabels, false);

        map.setGlobalStateProperty('custom', {nested: [1, 'two', true]});
        t.deepEqual(map.getGlobalState().custom, {nested: [1, 'two', true]});

        map.release();
        t.end();
    });

    t.test('resets a property to the style default on null', function(t) {
        var map = new mbgl.Map({request: function() {}});
        map.load(style);

        map.setGlobalStateProperty('showLabels', false);
        map.setGlobalStateProperty('showLabels', null);
        t.equal(map.getGlobalState().showLabels, true);

        map.release();
        t.end();
    });

    t.test('requires a string property name', function(t) {
        var map = new mbgl.Map({request: function() {}});
        map.load(style);

        t.throws(function() {
            map.setGlobalStateProperty(1, true);
        }, /First argument must be a string/);
        t.throws(function() {
            map.setGlobalStateProperty('name');
        }, /Two arguments required/);

        map.release();
        t.end();
    });

    t.end();
});
