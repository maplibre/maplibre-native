#!/bin/bash
set -euo pipefail

# Ensure required env variables are set
if [ -z "${SENTRY_AUTH_TOKEN:-}" ]; then
  echo "Error: SENTRY_AUTH_TOKEN is not set."
  exit 1
fi

if [ -z "${SENTRY_ORG:-}" ]; then
  echo "Error: SENTRY_ORG is not set."
  exit 1
fi

if [ -z "${SENTRY_PROJECT:-}" ]; then
  echo "Error: SENTRY_PROJECT is not set."
  exit 1
fi

if [ -z "${DSYM_PATH:-}" ]; then
  echo "Error: DSYM_PATH is not set."
  exit 1
fi

# Upload debug symbols
sentry-cli debug-files upload "$DSYM_PATH" --force-foreground --log-level=debug
