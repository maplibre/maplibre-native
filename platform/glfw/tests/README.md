# Vulkan resize check

Build with `MLN_WITH_GLFW=ON` and `MLN_WITH_VULKAN=ON`, then run in a desktop
session with a Vulkan presentation driver:

```sh
cmake --build build-plugin-glfw-vulkan --target mln-glfw-vulkan-resize-test
build-plugin-glfw-vulkan/platform/glfw/mln-glfw-vulkan-resize-test
```

The test opens a small window, resizes it five times, and renders eight frames
at each size. It checks that the swapchain matches GLFW's physical framebuffer
on both the first and eighth frame. Wayland exercises the application-selected
extent path, where reusing the old swapchain size causes persistent stretching.
The first-frame check also catches delayed resizing when a static map has no
further frames scheduled. The test does not depend on plugins or network data.
