#include <mln/util/event.hpp>
#include <mln/util/enum.hpp>

namespace mln {

MBGL_DEFINE_ENUM(EventSeverity,
                 {
                     {EventSeverity::Debug, "DEBUG"},
                     {EventSeverity::Info, "INFO"},
                     {EventSeverity::Warning, "WARNING"},
                     {EventSeverity::Error, "ERROR"},
                     {EventSeverity(-1), "UNKNOWN"},
                 });

MBGL_DEFINE_ENUM(Event,
                 {
                     {Event::General, "General"},
                     {Event::Setup, "Setup"},
                     {Event::Shader, "Shader"},
                     {Event::ParseStyle, "ParseStyle"},
                     {Event::ParseTile, "ParseTile"},
                     {Event::Render, "Render"},
                     {Event::Style, "Style"},
                     {Event::Database, "Database"},
                     {Event::HttpRequest, "HttpRequest"},
                     {Event::Sprite, "Sprite"},
                     {Event::Image, "Image"},
                     {Event::OpenGL, "OpenGL"},
                     {Event::JNI, "JNI"},
                     {Event::Android, "Android"},
                     {Event::Crash, "Crash"},
                     {Event::Glyph, "Glyph"},
                     {Event::Timing, "Timing"},
                     {Event(-1), "Unknown"},
                 });

} // namespace mln
