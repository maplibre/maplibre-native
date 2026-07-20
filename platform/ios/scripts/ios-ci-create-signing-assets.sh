#!/usr/bin/env bash

# Create Apple signing assets for CI and print them as YAML.
#
# Usage:
#   APPSTORE_ISSUER_ID=<issuer-uuid> \
#   APPSTORE_KEY_ID=<key-id> \
#   APPSTORE_PRIVATE_KEY="$(cat AuthKey_<key-id>.p8)" \
#   platform/ios/scripts/ios-ci-create-signing-assets.sh
#
# Optional environment:
#   IOS_BUNDLE_ID_PREFIX=org.maplibre
#   IOS_BUNDLE_IDENTIFIER=org.maplibre.*
#   IOS_CERTIFICATE_TYPE=IOS_DEVELOPMENT
#   IOS_PROFILE_TYPE=IOS_APP_DEVELOPMENT
#   IOS_EXISTING_CERTIFICATE_BASE64=<base64-pkcs12>
#   IOS_EXISTING_CERTIFICATE_PASSWORD=<pkcs12-password>
#   IOS_CI_OUTPUT_DIR=/path/to/private/output
#   IOS_CI_RECREATE_PROFILE=1
#   IOS_CI_RECREATE_DEVELOPMENT_CERTIFICATE=1
#
# By default, the script creates a wildcard iOS Development provisioning profile
# for "${IOS_BUNDLE_ID_PREFIX}.*" and prints the certificate, profile, passwords,
# and bundle ID prefix as YAML for embedding in ios-ci.yml.

set -euo pipefail

bundle_id_prefix="${IOS_BUNDLE_ID_PREFIX:-org.maplibre}"
profile_name="${IOS_PROFILE_NAME:-MapLibre iOS CI Wildcard Development}"
certificate_name="${IOS_CERTIFICATE_NAME:-MapLibre iOS CI}"
bundle_identifier="${IOS_BUNDLE_IDENTIFIER:-$bundle_id_prefix.*}"
certificate_type="${IOS_CERTIFICATE_TYPE:-IOS_DEVELOPMENT}"
profile_type="${IOS_PROFILE_TYPE:-IOS_APP_DEVELOPMENT}"

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

json_api_created_certificate_ids() {
  ruby -rjson -e '
    JSON.parse(STDIN.read).fetch("data", []).each do |item|
      attrs = item.fetch("attributes")
      puts item.fetch("id") if attrs["certificateType"] == "IOS_DEVELOPMENT" && attrs["displayName"] == "Created via API"
    end
  '
}

json_profile_ids_with_name() {
  ruby -rjson -e '
    profile_name = ARGV.fetch(0)
    JSON.parse(STDIN.read).fetch("data", []).each do |item|
      attrs = item.fetch("attributes")
      puts item.fetch("id") if attrs["name"] == profile_name
    end
  ' -- "$1"
}

json_bundle_id_for_identifier() {
  ruby -rjson -e '
    identifier = ARGV.fetch(0)
    item = JSON.parse(STDIN.read).fetch("data", []).find do |candidate|
      candidate.fetch("attributes").fetch("identifier") == identifier
    end
    puts item.fetch("id") if item
  ' -- "$1"
}

json_certificate_id_for_serial() {
  ruby -rjson -e '
    serial = ARGV.fetch(0).delete(":").upcase
    item = JSON.parse(STDIN.read).fetch("data", []).find do |candidate|
      candidate.fetch("attributes").fetch("serialNumber").delete(":").upcase == serial
    end
    puts item.fetch("id") if item
  ' -- "$1"
}

urlencode() {
  ruby -rcgi -e 'print CGI.escape(ARGV.fetch(0))' "$1"
}

base64_one_line() {
  if base64 --help 2>&1 | grep -q -- '-w'; then
    base64 -w 0 "$1"
  else
    base64 -i "$1" | tr -d '\n'
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
  local response_path
  local status
  local attempt=0
  local max_attempts=4
  local delay=5

  response_path="$(mktemp)"

  while true; do
    attempt=$(( attempt + 1 ))

    if [[ -n "$body" ]]; then
      status="$(curl -sS \
        -X "$method" \
        -H "Authorization: Bearer $ASC_TOKEN" \
        -H "Content-Type: application/json" \
        -d "$body" \
        -o "$response_path" \
        -w "%{http_code}" \
        "$url")"
    else
      status="$(curl -sS \
        -X "$method" \
        -H "Authorization: Bearer $ASC_TOKEN" \
        -H "Content-Type: application/json" \
        -o "$response_path" \
        -w "%{http_code}" \
        "$url")"
    fi

    echo "$status" > "${asc_status_file:-/dev/null}"

    if [[ "$status" -ge 500 && "$attempt" -lt "$max_attempts" ]]; then
      echo "App Store Connect request failed: $method $path returned HTTP $status (attempt $attempt/$max_attempts, retrying in ${delay}s)" >&2
      cat "$response_path" >&2
      sleep "$delay"
      delay=$(( delay * 2 ))
      continue
    fi

    break
  done

  if [[ "$status" -lt 200 || "$status" -ge 300 ]]; then
    echo "App Store Connect request failed: $method $path returned HTTP $status" >&2
    if [[ "$path" == "/v1/certificates" && "$status" == "403" ]]; then
      echo "Check that the API key has access to Certificates, Identifiers & Profiles." >&2
    fi
    cat "$response_path" >&2
    rm -f "$response_path"
    return 1
  fi

  cat "$response_path"
  rm -f "$response_path"
}

require_env APPSTORE_ISSUER_ID
require_env APPSTORE_KEY_ID
require_env APPSTORE_PRIVATE_KEY

command -v openssl >/dev/null || {
  echo "openssl is required" >&2
  exit 1
}

work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT
asc_status_file="$work_dir/asc_status"

if [[ -n "${IOS_EXISTING_CERTIFICATE_BASE64:-}" ]]; then
  require_env IOS_EXISTING_CERTIFICATE_PASSWORD
  p12_password="$IOS_EXISTING_CERTIFICATE_PASSWORD"
else
  p12_password="${P12_PASSWORD:-$(openssl rand -base64 32)}"
fi
keychain_password="${KEYCHAIN_PASSWORD:-$(openssl rand -base64 32)}"

private_key_path="$work_dir/ios-ci.key"
csr_path="$work_dir/ios-ci.csr"
certificate_der_path="$work_dir/ios-ci.cer"
certificate_pem_path="$work_dir/ios-ci.pem"
p12_path="$work_dir/ios-ci.p12"
profile_path="$work_dir/ios-ci.mobileprovision"

ASC_TOKEN="$(make_jwt)"
export ASC_TOKEN

if [[ "${IOS_CI_RECREATE_DEVELOPMENT_CERTIFICATE:-}" == "1" ]]; then
  certificates_response="$(asc_request GET '/v1/certificates?filter%5BcertificateType%5D=IOS_DEVELOPMENT&limit=10')"
  while IFS= read -r certificate_id_to_delete; do
    [[ -z "$certificate_id_to_delete" ]] && continue
    echo "Deleting existing API-created iOS Development certificate '$certificate_id_to_delete'."
    asc_request DELETE "/v1/certificates/$certificate_id_to_delete" >/dev/null
  done < <(printf '%s' "$certificates_response" | json_api_created_certificate_ids)

  profile_filter="$(urlencode "$profile_name")"
  profiles_response="$(asc_request GET "/v1/profiles?filter%5Bname%5D=$profile_filter&limit=200")"
  while IFS= read -r profile_id_to_delete; do
    [[ -z "$profile_id_to_delete" ]] && continue
    echo "Deleting existing iOS CI provisioning profile '$profile_id_to_delete'."
    asc_request DELETE "/v1/profiles/$profile_id_to_delete" >/dev/null
  done < <(printf '%s' "$profiles_response" | json_profile_ids_with_name "$profile_name")
fi

if [[ -n "${IOS_EXISTING_CERTIFICATE_BASE64:-}" ]]; then
  printf '%s' "$IOS_EXISTING_CERTIFICATE_BASE64" | base64 --decode >"$p12_path" 2>/dev/null ||
    printf '%s' "$IOS_EXISTING_CERTIFICATE_BASE64" | base64 -D >"$p12_path"
  openssl pkcs12 \
    -in "$p12_path" \
    -passin "pass:$p12_password" \
    -clcerts \
    -nokeys \
    -out "$certificate_pem_path"
  certificate_serial="$(openssl x509 -in "$certificate_pem_path" -noout -serial | cut -d= -f2)"
  certificates_response="$(asc_request GET "/v1/certificates?filter%5BcertificateType%5D=$certificate_type&limit=200")"
  certificate_id="$(printf '%s' "$certificates_response" | json_certificate_id_for_serial "$certificate_serial")"
  if [[ -z "$certificate_id" ]]; then
    echo "Existing $certificate_type certificate was not found in App Store Connect." >&2
    exit 1
  fi
else
  openssl genrsa -out "$private_key_path" 2048
  openssl req -new -key "$private_key_path" -subj "/CN=$certificate_name" -out "$csr_path"

  csr_content="$(cat "$csr_path")"
  csr_body="$(ruby -rjson -e 'puts JSON.generate({ data: { type: "certificates", attributes: { certificateType: ARGV.fetch(1), csrContent: ARGV.fetch(0) } } })' -- "$csr_content" "$certificate_type")"
  if ! certificate_response="$(asc_request POST /v1/certificates "$csr_body")"; then
    if [[ "$(cat "$asc_status_file" 2>/dev/null)" == "409" && "$certificate_type" == "IOS_DEVELOPMENT" ]]; then
      echo "A current iOS Development certificate already exists; deleting and retrying." >&2
      certificates_response="$(asc_request GET '/v1/certificates?filter%5BcertificateType%5D=IOS_DEVELOPMENT&limit=10')"
      while IFS= read -r certificate_id_to_delete; do
        [[ -z "$certificate_id_to_delete" ]] && continue
        echo "Deleting existing iOS Development certificate '$certificate_id_to_delete'."
        asc_request DELETE "/v1/certificates/$certificate_id_to_delete" >/dev/null
      done < <(printf '%s' "$certificates_response" | json_api_created_certificate_ids)
      profile_filter="$(urlencode "$profile_name")"
      profiles_response="$(asc_request GET "/v1/profiles?filter%5Bname%5D=$profile_filter&limit=200")"
      while IFS= read -r profile_id_to_delete; do
        [[ -z "$profile_id_to_delete" ]] && continue
        echo "Deleting existing iOS CI provisioning profile '$profile_id_to_delete'."
        asc_request DELETE "/v1/profiles/$profile_id_to_delete" >/dev/null
      done < <(printf '%s' "$profiles_response" | json_profile_ids_with_name "$profile_name")
      certificate_response="$(asc_request POST /v1/certificates "$csr_body")"
    else
      exit 1
    fi
  fi
  certificate_id="$(printf '%s' "$certificate_response" | json_get data id)"
  certificate_content="$(printf '%s' "$certificate_response" | json_get data attributes certificateContent)"

  printf '%s' "$certificate_content" | base64 --decode >"$certificate_der_path" 2>/dev/null ||
    printf '%s' "$certificate_content" | base64 -D >"$certificate_der_path"
  openssl x509 -inform DER -in "$certificate_der_path" -out "$certificate_pem_path"
  openssl pkcs12 -export -inkey "$private_key_path" -in "$certificate_pem_path" -out "$p12_path" -password "pass:$p12_password"
fi

bundle_filter="$(urlencode "$bundle_identifier")"
bundle_response="$(asc_request GET "/v1/bundleIds?filter%5Bidentifier%5D=$bundle_filter")"
bundle_id="$(printf '%s' "$bundle_response" | json_bundle_id_for_identifier "$bundle_identifier")"

if [[ -z "$bundle_id" ]]; then
  bundle_response="$(asc_request POST /v1/bundleIds "$(ruby -rjson -e 'puts JSON.generate({ data: { type: "bundleIds", attributes: { name: ARGV.fetch(0), identifier: ARGV.fetch(1), platform: "IOS" } } })' -- "$profile_name" "$bundle_identifier")")"
  bundle_id="$(printf '%s' "$bundle_response" | json_get data id)"
fi

device_ids=""
if [[ "$profile_type" == "IOS_APP_DEVELOPMENT" || "$profile_type" == "IOS_APP_ADHOC" ]]; then
  devices_response="$(asc_request GET '/v1/devices?filter%5Bplatform%5D=IOS&filter%5Bstatus%5D=ENABLED&limit=200')"
  device_ids="$(printf '%s' "$devices_response" | json_array_ids)"
fi

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
        attributes: { name: ARGV.fetch(0), profileType: ARGV.fetch(3) },
        relationships: relationships,
      },
    })
  ' -- "$profile_name" "$bundle_id" "$certificate_id" "$profile_type" <<<"$device_ids"
)"

if [[ "${IOS_CI_RECREATE_PROFILE:-}" == "1" ]]; then
  profile_filter="$(urlencode "$profile_name")"
  profiles_response="$(asc_request GET "/v1/profiles?filter%5Bname%5D=$profile_filter&limit=200")"
  while IFS= read -r profile_id_to_delete; do
    [[ -z "$profile_id_to_delete" ]] && continue
    echo "Deleting existing provisioning profile '$profile_id_to_delete'."
    asc_request DELETE "/v1/profiles/$profile_id_to_delete" >/dev/null
  done < <(printf '%s' "$profiles_response" | json_profile_ids_with_name "$profile_name")
fi

profile_response="$(asc_request POST /v1/profiles "$profile_body")"
profile_content="$(printf '%s' "$profile_response" | json_get data attributes profileContent)"
printf '%s' "$profile_content" | base64 --decode >"$profile_path" 2>/dev/null || printf '%s' "$profile_content" | base64 -D >"$profile_path"

if [[ -n "${IOS_CI_OUTPUT_DIR:-}" ]]; then
  mkdir -p "$IOS_CI_OUTPUT_DIR"
  chmod 700 "$IOS_CI_OUTPUT_DIR"
  cp "$p12_path" "$IOS_CI_OUTPUT_DIR/certificate.p12"
  cp "$profile_path" "$IOS_CI_OUTPUT_DIR/profile.mobileprovision"
  printf '%s' "$p12_password" >"$IOS_CI_OUTPUT_DIR/p12-password"
  chmod 600 "$IOS_CI_OUTPUT_DIR"/*
else
  cat <<YAML
# Generated by platform/ios/scripts/ios-ci-create-signing-assets.sh.
# Paste the following block into the 'Install Apple signing assets' step env in ios-ci.yml.
BUILD_CERTIFICATE_BASE64: $(base64_one_line "$p12_path")
P12_PASSWORD: $p12_password
BUILD_PROVISION_PROFILE_BASE64: $(base64_one_line "$profile_path")
KEYCHAIN_PASSWORD: $keychain_password
IOS_BUNDLE_ID_PREFIX: $bundle_id_prefix
YAML
fi

echo "Created signing certificate '$certificate_name' and provisioning profile '$profile_name'." >&2
