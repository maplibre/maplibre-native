#!/usr/bin/env bash

set -euo pipefail

repo="${GITHUB_REPOSITORY:-louwers/maplibre-native}"
bundle_id_prefix="${IOS_BUNDLE_ID_PREFIX:-com.louwers.maplibrenative.ci}"
profile_name="${IOS_PROFILE_NAME:-MapLibre iOS CI Wildcard Development}"
certificate_name="${IOS_CERTIFICATE_NAME:-MapLibre iOS CI}"

require_env() {
  local name="$1"
  if [[ -z "${!name:-}" ]]; then
    echo "$name is required" >&2
    exit 1
  fi
}

json_get() {
  ruby -rjson -e 'path = ARGV.map { |item| item.match?(/\A\d+\z/) ? item.to_i : item }; value = JSON.parse(STDIN.read).dig(*path); puts value if value' "$@"
}

json_array_ids() {
  ruby -rjson -e 'puts JSON.parse(STDIN.read).fetch("data", []).map { |item| item.fetch("id") }'
}

urlencode() {
  ruby -rcgi -e 'print CGI.escape(ARGV.fetch(0))' "$1"
}

base64_one_line() {
  if base64 --help 2>&1 | grep -q -- '-w'; then
    base64 -w 0 "$1"
  else
    base64 "$1" | tr -d '\n'
  fi
}

make_jwt() {
  ruby -ropenssl -rjson -rbase64 -e '
    def b64url(value)
      Base64.urlsafe_encode64(value).delete("=")
    end

    issuer_id = ENV.fetch("APPSTORE_ISSUER_ID")
    key_id = ENV.fetch("APPSTORE_KEY_ID")
    private_key = OpenSSL::PKey.read(ENV.fetch("APPSTORE_PRIVATE_KEY"))
    now = Time.now.to_i
    header = { "alg" => "ES256", "kid" => key_id, "typ" => "JWT" }
    claims = { "iss" => issuer_id, "iat" => now - 60, "exp" => now + 1200, "aud" => "appstoreconnect-v1" }
    signing_input = [b64url(header.to_json), b64url(claims.to_json)].join(".")
    der_signature = private_key.sign(OpenSSL::Digest::SHA256.new, signing_input)
    sequence = OpenSSL::ASN1.decode(der_signature)
    raw_signature = sequence.value.map { |integer| integer.value.to_s(2).rjust(32, "\x00") }.join
    puts [signing_input, b64url(raw_signature)].join(".")
  '
}

asc_request() {
  local method="$1"
  local path="$2"
  local body="${3:-}"
  local url="https://api.appstoreconnect.apple.com$path"

  if [[ -n "$body" ]]; then
    curl -fsS \
      -X "$method" \
      -H "Authorization: Bearer $ASC_TOKEN" \
      -H "Content-Type: application/json" \
      -d "$body" \
      "$url"
  else
    curl -fsS \
      -X "$method" \
      -H "Authorization: Bearer $ASC_TOKEN" \
      -H "Content-Type: application/json" \
      "$url"
  fi
}

require_env APPSTORE_ISSUER_ID
require_env APPSTORE_KEY_ID
require_env APPSTORE_PRIVATE_KEY

command -v gh >/dev/null || {
  echo "gh is required" >&2
  exit 1
}

command -v openssl >/dev/null || {
  echo "openssl is required" >&2
  exit 1
}

work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT

p12_password="${P12_PASSWORD:-$(openssl rand -base64 32)}"
keychain_password="${KEYCHAIN_PASSWORD:-$(openssl rand -base64 32)}"
wildcard_identifier="$bundle_id_prefix.*"

private_key_path="$work_dir/ios-ci.key"
csr_path="$work_dir/ios-ci.csr"
certificate_der_path="$work_dir/ios-ci.cer"
certificate_pem_path="$work_dir/ios-ci.pem"
p12_path="$work_dir/ios-ci.p12"
profile_path="$work_dir/ios-ci.mobileprovision"

openssl genrsa -out "$private_key_path" 2048
openssl req -new -key "$private_key_path" -subj "/CN=$certificate_name" -out "$csr_path"

ASC_TOKEN="$(make_jwt)"
export ASC_TOKEN

csr_content="$(cat "$csr_path")"
certificate_response="$(asc_request POST /v1/certificates "$(ruby -rjson -e 'puts JSON.generate({ data: { type: "certificates", attributes: { certificateType: "IOS_DEVELOPMENT", csrContent: ARGV.fetch(0) } } })' "$csr_content")")"
certificate_id="$(printf '%s' "$certificate_response" | json_get data id)"
certificate_content="$(printf '%s' "$certificate_response" | json_get data attributes certificateContent)"

printf '%s' "$certificate_content" | base64 --decode >"$certificate_der_path" 2>/dev/null || printf '%s' "$certificate_content" | base64 -D >"$certificate_der_path"
openssl x509 -inform DER -in "$certificate_der_path" -out "$certificate_pem_path"
openssl pkcs12 -export -inkey "$private_key_path" -in "$certificate_pem_path" -out "$p12_path" -password "pass:$p12_password"

bundle_filter="$(urlencode "$wildcard_identifier")"
bundle_response="$(asc_request GET "/v1/bundleIds?filter%5Bidentifier%5D=$bundle_filter")"
bundle_id="$(printf '%s' "$bundle_response" | json_get data 0 id)"

if [[ -z "$bundle_id" ]]; then
  bundle_response="$(asc_request POST /v1/bundleIds "$(ruby -rjson -e 'puts JSON.generate({ data: { type: "bundleIds", attributes: { name: ARGV.fetch(0), identifier: ARGV.fetch(1), platform: "IOS" } } })' "$profile_name" "$wildcard_identifier")")"
  bundle_id="$(printf '%s' "$bundle_response" | json_get data id)"
fi

devices_response="$(asc_request GET '/v1/devices?filter%5Bplatform%5D=IOS&filter%5Bstatus%5D=ENABLED&limit=200')"
device_ids="$(printf '%s' "$devices_response" | json_array_ids)"

profile_body="$(
  ruby -rjson -e '
    devices = STDIN.read.lines.map(&:strip).reject(&:empty?).map { |id| { type: "devices", id: id } }
    relationships = {
      bundleId: { data: { type: "bundleIds", id: ARGV.fetch(1) } },
      certificates: { data: [{ type: "certificates", id: ARGV.fetch(2) }] },
    }
    relationships[:devices] = { data: devices } unless devices.empty?
    puts JSON.generate({
      data: {
        type: "profiles",
        attributes: { name: ARGV.fetch(0), profileType: "IOS_APP_DEVELOPMENT" },
        relationships: relationships,
      },
    })
  ' "$profile_name" "$bundle_id" "$certificate_id" <<<"$device_ids"
)"

profile_response="$(asc_request POST /v1/profiles "$profile_body")"
profile_content="$(printf '%s' "$profile_response" | json_get data attributes profileContent)"
printf '%s' "$profile_content" | base64 --decode >"$profile_path" 2>/dev/null || printf '%s' "$profile_content" | base64 -D >"$profile_path"

gh secret set BUILD_CERTIFICATE_BASE64 --repo "$repo" --body "$(base64_one_line "$p12_path")"
gh secret set P12_PASSWORD --repo "$repo" --body "$p12_password"
gh secret set BUILD_PROVISION_PROFILE_BASE64 --repo "$repo" --body "$(base64_one_line "$profile_path")"
gh secret set KEYCHAIN_PASSWORD --repo "$repo" --body "$keychain_password"
gh variable set IOS_BUNDLE_ID_PREFIX --repo "$repo" --body "$bundle_id_prefix"

echo "Created signing certificate '$certificate_name' and provisioning profile '$profile_name'."
echo "Updated signing secrets and IOS_BUNDLE_ID_PREFIX for $repo."
