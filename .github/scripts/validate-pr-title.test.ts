import test from "node:test";
import { validateTitle } from "./validate-pr-title.ts";

test("validateTitle - missing Android label", (t) => {
  t.assert.deepEqual(validateTitle({
    labels: new Set(["android"]),
    title: "make a change"
  }), {
    mismatches: ["android"],
    ok: false
  });
});

test("validateTitle - android required and title contains Android", (t) => {
  t.assert.deepEqual(validateTitle({
    labels: new Set(["android"]),
    title: "make change to Android"
  }), {
    ok: true
  });
});

test("validateTitle - only core required when label contains core", (t) => {
  t.assert.deepEqual(validateTitle({
    labels: new Set(["core", "android"]),
    title: "core: make some change"
  }), {
    ok: true
  });
});
