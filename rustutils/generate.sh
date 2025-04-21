#!/bin/bash

# Run this script from the repository root
# Install cxxbridge with:
# $ cargo install cxxbridge-cmd@1.0.157 --locked

set -e

if [ "$#" -lt 2 ]; then
  echo "Error: output_dir and at least one input file are required." >&2
  echo "Usage: $0 <output_dir> <input_file1> [input_file2 ...]" >&2
  exit 1
fi

# Use CXXBRIDGE_CMD environment variable if set, otherwise fallback to "cxxbridge"
CXXBRIDGE_CMD="${CXXBRIDGE_CMD:-cxxbridge}"

output_dir="$1"
shift

for input_file in "$@"; do
  if [[ "$input_file" != *.rs ]]; then
    echo "Error: All input files must have a .rs extension. Invalid file: $input_file" >&2
    exit 1
  fi
done

mkdir -p "$output_dir/cpp/include/rustutils"
mkdir -p "$output_dir/cpp/src"

for input_file in "$@"; do
  base_name=$(basename "$input_file" .rs)
  "$CXXBRIDGE_CMD" "$input_file" --header > "$output_dir/cpp/include/rustutils/${base_name}.hpp"
  "$CXXBRIDGE_CMD" "$input_file" > "$output_dir/cpp/src/${base_name}.rs.cpp"
done
