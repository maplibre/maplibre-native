#include <QMapLibreGL/Settings>

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/tile_server_options.hpp>
#include <mbgl/util/traits.hpp>

#include <QCoreApplication>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4805)
#endif

// mbgl::GLContextMode
static_assert(mbgl::underlying_type(QMapLibreGL::Settings::UniqueGLContext) == mbgl::underlying_type(mbgl::gfx::ContextMode::Unique), "error");
static_assert(mbgl::underlying_type(QMapLibreGL::Settings::SharedGLContext) == mbgl::underlying_type(mbgl::gfx::ContextMode::Shared), "error");

// mbgl::MapMode
static_assert(mbgl::underlying_type(QMapLibreGL::Settings::Continuous) == mbgl::underlying_type(mbgl::MapMode::Continuous), "error");
static_assert(mbgl::underlying_type(QMapLibreGL::Settings::Static) == mbgl::underlying_type(mbgl::MapMode::Static), "error");

// mbgl::ConstrainMode
static_assert(mbgl::underlying_type(QMapLibreGL::Settings::NoConstrain) == mbgl::underlying_type(mbgl::ConstrainMode::None), "error");
static_assert(mbgl::underlying_type(QMapLibreGL::Settings::ConstrainHeightOnly) == mbgl::underlying_type(mbgl::ConstrainMode::HeightOnly), "error");
static_assert(mbgl::underlying_type(QMapLibreGL::Settings::ConstrainWidthAndHeight) == mbgl::underlying_type(mbgl::ConstrainMode::WidthAndHeight), "error");

// mbgl::ViewportMode
static_assert(mbgl::underlying_type(QMapLibreGL::Settings::DefaultViewport) == mbgl::underlying_type(mbgl::ViewportMode::Default), "error");
static_assert(mbgl::underlying_type(QMapLibreGL::Settings::FlippedYViewport) == mbgl::underlying_type(mbgl::ViewportMode::FlippedY), "error");

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace QMapLibreGL {

/*!
    \class QMapLibreGL::Settings
    \brief The Settings class stores the initial configuration for QMapLibreGL::Map.

    \inmodule MapLibre Maps SDK for Qt

    Settings is used to configure QMapLibreGL::Map at the moment of its creation.
    Once created, the Settings of a QMapLibreGL::Map can no longer be changed.

    Cache-related settings are shared between all QMapLibreGL::Map instances using the same cache path.
    The first map to configure cache properties such as size will force the configuration
    to all newly instantiated QMapLibreGL::Map objects using the same cache in the same process.
*/

/*!
    \enum QMapLibreGL::Settings::GLContextMode

    This enum sets the expectations for the OpenGL state.

    \value UniqueGLContext  The OpenGL context is only used by QMapLibreGL::Map, so it is not
    reset before each rendering. Use this mode if the intention is to only draw a
    fullscreen map.

    \value SharedGLContext  The OpenGL context is shared and the state will be
    marked dirty - which invalidates any previously assumed GL state. The
    embedder is responsible for clearing up the viewport prior to calling
    QMapLibreGL::Map::render. The embedder is also responsible for resetting its own
    GL state after QMapLibreGL::Map::render has finished, if needed.

    \sa contextMode()
*/

/*!
    \enum QMapLibreGL::Settings::MapMode

    This enum sets the map rendering mode

    \value Continuous  The map will render as data arrives from the network and
    react immediately to state changes.

    This is the default mode and the preferred when the map is intended to be
    interactive.

    \value Static  The map will no longer react to state changes and will only
    be rendered when QMapLibreGL::Map::startStaticRender is called. After all the
    resources are loaded, the QMapLibreGL::Map::staticRenderFinished signal is emitted.

    This mode is useful for taking a snapshot of the finished rendering result
    of the map into a QImage.

    \sa mapMode()
*/

/*!
    \enum QMapLibreGL::Settings::ConstrainMode

    This enum determines if the map wraps.

    \value NoConstrain              The map will wrap on the horizontal axis. Since it doesn't
    make sense to wrap on the vertical axis in a Web Mercator projection, the map will scroll
    and show some empty space.

    \value ConstrainHeightOnly      The map will wrap around the horizontal axis, like a spinning
    globe. This is the recommended constrain mode.

    \value ConstrainWidthAndHeight  The map won't wrap and panning is restricted to the boundaries
    of the map.

    \sa constrainMode()
*/

/*!
    \enum QMapLibreGL::Settings::ViewportMode

    This enum flips the map vertically.

    \value DefaultViewport  Native orientation.

    \value FlippedYViewport  Mirrored vertically.

    \sa viewportMode()
*/

/*!
    Constructs a Settings object with the default values. The default
    configuration is valid for initializing a QMapLibreGL::Map.
*/
Settings::Settings()
    : m_contextMode(Settings::SharedGLContext)
    , m_mapMode(Settings::Continuous)
    , m_constrainMode(Settings::ConstrainHeightOnly)
    , m_viewportMode(Settings::DefaultViewport)
    , m_cacheMaximumSize(mbgl::util::DEFAULT_MAX_CACHE_SIZE)
    , m_cacheDatabasePath(":memory:")
    , m_assetPath(QCoreApplication::applicationDirPath())
    , m_apiKey(qgetenv("MGL_API_KEY"))
    , m_tileServerOptionsInternal(new mbgl::TileServerOptions(mbgl::TileServerOptions::DefaultConfiguration()))
{
}

/*!
    Returns the OpenGL context mode. This is specially important when mixing
    with other OpenGL draw calls.

    By default, it is set to Settings::SharedGLContext.
*/
Settings::GLContextMode Settings::contextMode() const
{
    return m_contextMode;
}

/*!
    Sets the OpenGL context \a mode.
*/
void Settings::setContextMode(GLContextMode mode)
{
    m_contextMode = mode;
}

/*!
    Returns the map mode. Static mode will emit a signal for
    rendering a map only when the map is fully loaded.
    Animations like style transitions and labels fading won't
    be seen.

    The Continuous mode will emit the signal for every new
    change on the map and it is usually what you expect for
    a interactive map.

    By default, it is set to QMapLibreGL::Settings::Continuous.
*/
Settings::MapMode Settings::mapMode() const
{
    return m_mapMode;
}

/*!
    Sets the map \a mode.
*/
void Settings::setMapMode(MapMode mode)
{
    m_mapMode = mode;
}

/*!
    Returns the constrain mode. This is used to limit the map to wrap
    around the globe horizontally.

    By default, it is set to QMapLibreGL::Settings::ConstrainHeightOnly.
*/
Settings::ConstrainMode Settings::constrainMode() const
{
    return m_constrainMode;
}

/*!
    Sets the map constrain \a mode.
*/
void Settings::setConstrainMode(ConstrainMode mode)
{
    m_constrainMode = mode;
}

/*!
    Returns the viewport mode. This is used to flip the vertical
    orientation of the map as some devices may use inverted orientation.

    By default, it is set to QMapLibreGL::Settings::DefaultViewport.
*/
Settings::ViewportMode Settings::viewportMode() const
{
    return m_viewportMode;
}

/*!
    Sets the viewport \a mode.
*/
void Settings::setViewportMode(ViewportMode mode)
{
    m_viewportMode = mode;
}

/*!
    Returns the cache database maximum hard size in bytes. The database
    will grow until the limit is reached. Setting a maximum size smaller
    than the current size of an existing database results in undefined
    behavior

    By default, it is set to 50 MB.
*/
unsigned Settings::cacheDatabaseMaximumSize() const
{
    return m_cacheMaximumSize;
}

/*!
    Returns the maximum allowed cache database \a size in bytes.
*/
void Settings::setCacheDatabaseMaximumSize(unsigned size)
{
    m_cacheMaximumSize = size;
}

/*!
    Returns the cache database path. The cache is used for storing
    recently used resources like tiles and also an offline tile database
    pre-populated by the
    \l {https://github.com/mapbox/mapbox-gl-native/blob/master/bin/offline.sh}
    {Offline Tool}.

    By default, it is set to \c :memory: meaning it will create an in-memory
    cache instead of a file on disk.
*/
QString Settings::cacheDatabasePath() const
{
    return m_cacheDatabasePath;
}

/*!
    Sets the cache database \a path.

    Setting the \a path to \c :memory: will create an in-memory cache.
*/
void Settings::setCacheDatabasePath(const QString &path)
{
    m_cacheDatabasePath = path;
}

/*!
    Returns the asset path, which is the root directory from where
    the \c asset:// scheme gets resolved in a style. \c asset:// can be used
    for loading a resource from the disk in a style rather than fetching
    it from the network.

    By default, it is set to the value returned by QCoreApplication::applicationDirPath().
*/
QString Settings::assetPath() const
{
    return m_assetPath;
}

/*!
    Sets the asset \a path.
*/
void Settings::setAssetPath(const QString &path)
{
    m_assetPath = path;
}

/*!
    Returns the API key.

    By default, it is taken from the environment variable \c MGL_API_KEY
    or empty if the variable is not set.
*/
QString Settings::apiKey() const {
    return m_apiKey;
}

/*!
    Sets the API key.

    MapTiler-hosted and Mapbox-hosted vector tiles and styles require an API
    key or access token.
*/
void Settings::setApiKey(const QString &key)
{
    m_apiKey = key;
}

/*!
    Returns the API base URL.
*/
QString Settings::apiBaseUrl() const
{
    return QString::fromStdString(m_tileServerOptionsInternal->baseURL());
}

/*!
    Sets the API base \a url.

    The API base URL is the URL that the \b "mapbox://" protocol will
    be resolved to. It defaults to "https://api.mapbox.com" but can be
    changed, for instance, to a tile cache server address.
*/
void Settings::setApiBaseUrl(const QString& url)
{
    m_tileServerOptionsInternal = &m_tileServerOptionsInternal->withBaseURL(url.toStdString());
}

/*!
    Returns the local font family. Returns an empty string if no local font family is set.
*/
QString Settings::localFontFamily() const
{
    return m_localFontFamily;
}

/*!
    Sets the local font family.

   Rendering Chinese/Japanese/Korean (CJK) ideographs and precomposed Hangul Syllables requires
   downloading large amounts of font data, which can significantly slow map load times. Use the
   localIdeographFontFamily setting to speed up map load times by using locally available fonts
   instead of font data fetched from the server.
*/
void Settings::setLocalFontFamily(const QString &family)
{
    m_localFontFamily = family;
}

/*!
    Returns the client name. Returns an empty string if no client name is set.
*/
QString Settings::clientName() const
{
    return m_clientName;
}

/*!
    Sets the client name.
*/
void Settings::setClientName(const QString &name)
{
    m_clientName = name;
}

/*!
    Returns the client version. Returns an empty string if no client version is set.
*/
QString Settings::clientVersion() const
{
    return m_clientVersion;
}

/*!
    Sets the client version.
*/
void Settings::setClientVersion(const QString &version)
{
    m_clientVersion = version;
}

/*!
    Returns resource transformation callback used to transform requested URLs.
*/
std::function<std::string(const std::string &)> Settings::resourceTransform() const {
    return m_resourceTransform;
}

/*!
    Sets the resource \a transform callback.

    When given, resource transformation callback will be used to transform the
    requested resource URLs before they are requested from internet. This can be
    used add or remove custom parameters, or reroute certain requests to other
    servers or endpoints.
*/
void Settings::setResourceTransform(const std::function<std::string(const std::string &)> &transform) {
    m_resourceTransform = transform;
}

/*!
    Reset all settings based on the given template.

    MapLibre can support servers with different resource path structure.
    Some of the most common servers like Maptiler and Mapbox are defined
    in the library. This function will re-initialise all settings based
    on the default values of specific service provider defaults.
*/
void Settings::resetToTemplate(SettingsTemplate settings_template)
{
    if(m_tileServerOptionsInternal) delete m_tileServerOptionsInternal;

    if(settings_template == MapLibreSettings){
        m_tileServerOptionsInternal = new mbgl::TileServerOptions(mbgl::TileServerOptions::MapLibreConfiguration());
    }else if(settings_template == MapTilerSettings){
        m_tileServerOptionsInternal = new mbgl::TileServerOptions(mbgl::TileServerOptions::MapTilerConfiguration());
    }else if(settings_template == MapboxSettings){
        m_tileServerOptionsInternal = new mbgl::TileServerOptions(mbgl::TileServerOptions::MapboxConfiguration());
    }else{
        m_tileServerOptionsInternal = new mbgl::TileServerOptions(mbgl::TileServerOptions::DefaultConfiguration());
    }
}

/*!
    All predefined styles.

    Return all styles that are defined in default settings.
*/
QVector<QPair<QString, QString> > Settings::defaultStyles() const {
    QVector<QPair<QString, QString>> styles;
    for (const auto &style : tileServerOptionsInternal()->defaultStyles()) {
        styles.append(QPair<QString, QString>(
                                 QString::fromStdString(style.getUrl()), QString::fromStdString(style.getName())));
    }
    return styles;
}

mbgl::TileServerOptions *Settings::tileServerOptionsInternal() const {
    return m_tileServerOptionsInternal;
}

} // namespace QMapLibreGL
