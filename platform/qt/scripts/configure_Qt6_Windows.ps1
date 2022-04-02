$path = $args[0]
$install = $args[1]

qt-cmake -S "$path" `
  -GNinja `
  -DCMAKE_BUILD_TYPE="Release" `
  -DCMAKE_INSTALL_PREFIX="$install" `
  -DMBGL_WITH_QT=ON
