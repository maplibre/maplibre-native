#!/usr/bin/env bash

# This script allows you to run commands from a graphical context
# on macOS. This may be needed to run tests that depend on testmanagerd.
# See:
# https://stackoverflow.com/questions/67688130/run-macos-test-cases-on-the-jenkins-pipeline/71417206#71417206

# This script writes out a script that runs a command, then runs it with Terminal.app.
# Next, it prints the output from the script and exits with the same
# exit code as the command. See also this blog post:
# https://aahlenst.dev/blog/accessing-the-macos-gui-in-automation-contexts/#continuous-integration-with-ssh

set -eo pipefail

if [ $# -ne 1 ]; then
  echo "Usage: $0 <command>"
  exit 1
fi
command="$1"

run_command_file="$(mktemp)"

output_file="$(mktemp)"
exit_code_file="$(mktemp)"

cat << EOF > "$run_command_file"
cd "$(pwd)"

$command 2>&1 | tee "$output_file"
exit_code=$?
echo $exit_code > "$exit_code_file"

osascript -e 'tell application "Terminal" to quit' &
EOF

chmod +x "$run_command_file"
open --wait-apps --new --fresh -a Terminal.app "$run_command_file"

cat "$output_file"

exit $(cat "$exit_code_file")
