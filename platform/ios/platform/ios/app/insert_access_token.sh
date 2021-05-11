if [[ "$CI" ]]; then
  echo "CI environment, access token not required"
  exit 0
else
  echo "Inserting MapLibre API key..."
  token_file=~/.maplibre
  token_file2=~/maplibre
  token="$(cat $token_file 2>/dev/null || cat $token_file2 2>/dev/null)"
  if [ "$token" ]; then
    plutil -replace MGLApiKey -string $token "$TARGET_BUILD_DIR/$INFOPLIST_PATH"
    echo "Token insertion successful"
  else
    echo \'Warning: Missing API key.\'
  fi
fi
