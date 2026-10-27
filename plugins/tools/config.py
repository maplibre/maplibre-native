"""Loads, validates and resolves the plugin.json of every plugin in plugins/plugins/.

plugin.json is schema 1 of https://github.com/louwers/maplibre-native-plugins, so a plugin can move between this
repository and a standalone plugin repository. Paths are relative to the plugin's directory. In-tree differences:

* includeDirectories may point outside the plugin directory, as long as they stay inside this repository
  (for example at a vendored dependency).
* Optional `backends`: the renderers the plugin implements, a subset of opengl, vulkan and metal (default: all).
  A plugin without OpenGL or Vulkan gets no Android wrapper, and one without OpenGL or Metal gets no iOS wrapper.
* `demo.style` may be an https URL, and `demo.camera` may have `pitch` and `bearing`.

Every tool reads plugins through resolve(), so naming defaults live in one place.
"""

import json
import re
from pathlib import Path, PurePosixPath

TOOLS_ROOT = Path(__file__).resolve().parent
PLUGINS_ROOT = TOOLS_ROOT.parent
REPO_ROOT = PLUGINS_ROOT.parent
PLUGIN_SOURCES = PLUGINS_ROOT / "plugins"
BACKENDS = ("opengl", "vulkan", "metal")
ANDROID_BACKENDS = ("opengl", "vulkan")
APPLE_BACKENDS = ("opengl", "metal")


class ConfigError(Exception):
    pass


def _require(condition, message):
    if not condition:
        raise ConfigError(message)


def _pascal(identifier):
    return "".join(part[:1].upper() + part[1:] for part in identifier.split("-"))


def _inside(path, directory):
    return directory == path or directory in path.parents


def _relative_path(plugin_dir, path, what, root, must_exist=True):
    _require(isinstance(path, str) and path and not path.startswith("/"), f"{what} must be a relative path")
    full = (plugin_dir / path).resolve()
    _require(_inside(full, root), f"{what} escapes the repository: {path}")
    if must_exist:
        _require(full.exists(), f"{what} does not exist: {path}")
    return path


def android_versions():
    """The NDK version comes from the SDK build; the SDK levels match the SDK's own modules."""
    versions = (REPO_ROOT / "platform/android/buildSrc/src/main/kotlin/Versions.kt").read_text(encoding="utf-8")
    ndk = re.search(r'ndkVersion\s*=\s*"([^"]+)"', versions)
    _require(ndk, "Cannot find ndkVersion in platform/android/buildSrc/src/main/kotlin/Versions.kt")
    return {"ndkVersion": ndk.group(1), "minSdk": 23, "compileSdk": 35}


def resolve(plugin_dir):
    plugin_dir = Path(plugin_dir).resolve()
    path = plugin_dir / "plugin.json"
    _require(path.is_file(), f"Missing {path}")
    where = f"{path.relative_to(REPO_ROOT)}"
    raw = json.loads(path.read_text(encoding="utf-8"))
    _require(raw.get("schemaVersion") == 1, f"{where}: schemaVersion must be 1")

    plugin_id = raw.get("id")
    _require(isinstance(plugin_id, str) and re.fullmatch(r"[a-z][a-z0-9]*(-[a-z0-9]+)*", plugin_id),
             f"{where}: id must be kebab-case, for example square-layer")
    _require(plugin_dir.name == plugin_id, f"{where}: the directory must be named after the id ({plugin_id})")
    version = raw.get("version")
    _require(isinstance(version, str) and re.fullmatch(r"\d+\.\d+\.\d+(-[0-9A-Za-z.-]+)?", version),
             f"{where}: version must be a semantic version")
    register = raw.get("registerFunction")
    _require(isinstance(register, str) and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", register),
             f"{where}: registerFunction must be a C identifier")

    def relative(value, what, **kwargs):
        return _relative_path(plugin_dir, value, f"{where}: {what}", REPO_ROOT, **kwargs)

    header = relative(raw.get("header"), "header")
    sources = raw.get("sources")
    _require(isinstance(sources, list) and sources, f"{where}: sources must be a non-empty list")
    sources = [relative(source, "source") for source in sources]
    include_directories = raw.get("includeDirectories", [str(PurePosixPath(header).parent)])
    # Vendored dependencies are git submodules, which may not be checked out.
    include_directories = [relative(d, "include directory", must_exist=not (plugin_dir / d).resolve().is_relative_to(
        REPO_ROOT / "vendor")) for d in include_directories]
    _require(any(_inside((plugin_dir / header).resolve(), (plugin_dir / d).resolve()) for d in include_directories),
             f"{where}: header {header} must be inside one of includeDirectories")

    backends = raw.get("backends", list(BACKENDS))
    _require(isinstance(backends, list) and backends and all(b in BACKENDS for b in backends),
             f"{where}: backends must be a non-empty subset of {', '.join(BACKENDS)}")

    product = _pascal(plugin_id)
    android = dict(raw.get("android", {}))
    android.setdefault("namespace", "org.maplibre.plugins." + plugin_id.replace("-", ""))
    android.setdefault("className", product + "Plugin")
    android.setdefault("libraryName", plugin_id)
    _require(re.fullmatch(r"[a-z][a-z0-9_]*(\.[a-z][a-z0-9_]*)+", android["namespace"]),
             f"{where}: android.namespace must be a Java package")
    _require(re.fullmatch(r"[A-Z][A-Za-z0-9_]*", android["className"]), f"{where}: android.className")
    android["qualifiedClassName"] = android["namespace"] + "." + android["className"]
    android["supported"] = any(b in ANDROID_BACKENDS for b in backends)

    apple = dict(raw.get("apple", {}))
    apple.setdefault("product", product)
    apple.setdefault("className", "MLN" + apple["product"] + "Plugin")
    apple.setdefault("swiftName", apple["product"] + "Plugin")
    apple.setdefault("errorDomain", "org.maplibre.plugins." + plugin_id)
    for key in ("product", "className", "swiftName"):
        _require(re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", apple[key]), f"{where}: apple.{key}")
    apple["supported"] = any(b in APPLE_BACKENDS for b in backends)
    # A plugin that has no OpenGL implementation can only be linked into a Metal build.
    apple["metalOnly"] = "opengl" not in backends

    demo = dict(raw.get("demo", {}))
    if "style" in demo:
        if not demo["style"].startswith("https://"):
            relative(demo["style"], "demo.style")
    camera = demo.get("camera")
    if camera is not None:
        _require(all(isinstance(camera.get(k), (int, float)) for k in ("latitude", "longitude", "zoom")),
                 f"{where}: demo.camera needs latitude, longitude and zoom")
        _require(all(isinstance(camera.get(k, 0), (int, float)) for k in ("pitch", "bearing")),
                 f"{where}: demo.camera pitch and bearing must be numbers")

    return {
        "directory": plugin_dir,
        "repositoryPath": str(PurePosixPath(plugin_dir.relative_to(REPO_ROOT).as_posix())),
        "id": plugin_id,
        "displayName": raw.get("displayName", plugin_id),
        "description": raw.get("description", ""),
        "version": version,
        "registerFunction": register,
        "header": header,
        "sources": sources,
        "includeDirectories": include_directories,
        "backends": backends,
        "android": android,
        "apple": apple,
        "demo": demo,
    }


def discover():
    """Every plugin under plugins/plugins/, ordered by id."""
    return [resolve(path.parent) for path in sorted(PLUGIN_SOURCES.glob("*/plugin.json"))]
