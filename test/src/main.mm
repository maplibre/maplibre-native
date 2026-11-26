#include <mln/test.hpp>

#import <Foundation/Foundation.h>

int main(int argc, char* argv[]) {
  [[NSFileManager defaultManager] changeCurrentDirectoryPath:[[NSBundle mainBundle] bundlePath]];
  return mln::runTests(argc, argv);
}
