$path = $args[0]
$install = $args[1]

cmake -S "$path" `
  -GNinja `
  -DCMAKE_BUILD_TYPE="Release" `
  -DCMAKE_INSTALL_PREFIX="$install" `
  -DMLN_WITH_QT=ON `
  -DMLN_QT_STATIC=ON `
  -DMLN_QT_LIBRARY_ONLY=ON
