# GLFW is not available on iOS (Dawn auto-detects it via UNIX AND NOT ANDROID)
set(DAWN_USE_GLFW OFF CACHE BOOL "Disable GLFW on iOS" FORCE)
