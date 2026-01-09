import os
from pathlib import Path

def find_file(start_directory, target_filename):
    """Recursively search for a file by name starting from start_directory."""
    for root, dirs, files in os.walk(start_directory):
        if target_filename in files:
            path = Path(os.path.join(root, target_filename))
            return path.relative_to("/docs")
    raise FileNotFoundError(f"File '{target_filename}' could not be found in {start_directory}")

def define_env(env):
    """
    This is the hook for the variables, macros and filters.
    """

    @env.macro
    def activity_source_note(filename):
        file_path = find_file(f"{os.getcwd()}/MapLibreAndroidTestApp", filename)

        return f"""
!!! note

    You can find the full source code of this example in [`{filename}`](https://github.com/maplibre/maplibre-native/blob/main/platform/android/{file_path}) of the MapLibreAndroidTestApp.
"""

    @env.macro
    def s3_url(filename):
        return f"https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/{filename}"

    @env.macro
    def openmaptiles_caption():
        return f"""
<caption>
    <p class="inline-attribution">Map data [OpenStreetMap](https://www.openstreetmap.org/copyright). © [OpenMapTiles](https://openmaptiles.org/).</p>
</caption>
"""
