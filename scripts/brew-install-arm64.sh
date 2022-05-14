#!/usr/bin/env bash

mkdir -p ~/arm-target/packages

export HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK=1

echo brew fetch --force --bottle-tag=arm64_monterey $1
brew fetch --force --bottle-tag=arm64_monterey $1 |\
  grep -E "(Downloaded to:|Already downloaded:)" |\
  grep -v "\.json" |\
  awk '{ print $3 }' |\
  xargs -n 1 brew install --ignore-dependencies --force-bottle
