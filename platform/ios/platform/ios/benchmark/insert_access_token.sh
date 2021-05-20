if [[ "$CI" ]]; then
  echo "CI environment, API key not required"
  exit 0
else
  echo "Inserting MapLibre API key..."
  token_file=~/.maplibre
  token_file2=~/maplibre
  token="$(cat $token_file 2>/dev/null || cat $token_file2 2>/dev/null)"
  if [ "$token" ]; then
    echo "Inserting token: $token into $TARGET_BUILD_DIR/$INFOPLIST_PATH"
    plutil -replace MGLApiKey -string $token "$TARGET_BUILD_DIR/$INFOPLIST_PATH"
    echo "API key insertion successful"
  else
    echo \'Warning: Missing API key.\'
  fi
fi
