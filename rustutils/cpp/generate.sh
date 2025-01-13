# Run this script from the repository root
# Install cxxbridge with:
# $ cargo install cxxbridge-cmd

set -e

cxxbridge rustutils/src/lib.rs --header > rustutils/cpp/include/rustutils/lib.hpp
cxxbridge rustutils/src/lib.rs > rustutils/cpp/src/lib.rs.cc
