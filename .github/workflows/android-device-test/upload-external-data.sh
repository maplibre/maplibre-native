#!/bin/bash

set -e

# Check if the required variables are defined
if [ -z "$RESULTS_API" ] || [ -z "$AWS_DEVICE_FARM_PROJECT_ARN" ]; then
  echo "Error: Missing required variables."
  exit 1
fi

cd "$(mktemp -d)"

cat << EOF > instrumentation-test-input.json
{
  "styleNames": ["Facebook Light", "MapTiler Basic", "Americana"],
  "styleURLs": [
    "https://external.xx.fbcdn.net/maps/vt/style/canterbury_1_0/?locale=en_US",
    "https://api.maptiler.com/maps/basic-v2/style.json?key=get_your_own_OpIi9ZULNHzrESv6T2vL",
    "https://zelonewolf.github.io/openstreetmap-americana/style.json"
  ],
  "resultsAPI": "$RESULTS_API",
  "apiKey": "get_your_own_OpIi9ZULNHzrESv6T2vL"
}
EOF

>&2 zip instrumentation-test-input.zip instrumentation-test-input.json
upload_json="$(aws devicefarm create-upload --project-arn "$AWS_DEVICE_FARM_PROJECT_ARN" --type EXTERNAL_DATA --name instrumentation-test-input.zip --output json)"
upload_url="$(echo "$upload_json" | jq -r '.upload.url')"
upload_arn="$(echo "$upload_json" | jq -r '.upload.arn')"
curl -T instrumentation-test-input.zip "$upload_url"

retries=5
while true; do
  upload_status="$(aws devicefarm get-upload --arn "$upload_arn" --output text --query upload.status)"
  >&2 echo "Upload $upload_status"
  sleep 1
  if [[ "$upload_status" == "SUCCEEDED" ]]; then
    break
  fi

  if (( --retries == 0 )); then
    exit 1
  fi
done

echo "$upload_arn"
