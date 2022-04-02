$path = $args[0]
$install = $args[1]

cmake -S "$path" `
  -GNinja `
  -DCMAKE_BUILD_TYPE="Release" `
  -DCMAKE_INSTALL_PREFIX="$install" `
  -DMBGL_WITH_QT=ON `
  -DMBGL_QT_STATIC=ON `
  -DMBGL_QT_LIBRARY_ONLY=ON
