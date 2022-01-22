echo "Inserting MapLibre API key..."
token_file=~/.maplibre
token_file2=~/maplibre
token="$(cat $token_file 2>/dev/null || cat $token_file2 2>/dev/null  || $MGL_API_KEY)"
if [ "$token" ]; then
    plutil -replace MGLApiKey -string $token "$TARGET_BUILD_DIR/$INFOPLIST_PATH"
    echo "API key insertion successful"
else
    echo 'Warning: Missing API key.'
fi
