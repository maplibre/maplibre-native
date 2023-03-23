#include <QGuiApplication>

#include <mbgl/test.hpp>
#include <cstring>
#include <cerrno>
#include <cstdio>

#if defined(WIN32) && !defined(__clang__)
#include <direct.h>
#else
#include <unistd.h>
#endif

#define xstr(s) str(s)
#define str(s) #s

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

#ifdef WORK_DIRECTORY
#ifdef WIN32
    const int result = _chdir(xstr(WORK_DIRECTORY));
#else
    const int result = chdir(xstr(WORK_DIRECTORY));
#endif
    if (result != 0) {
        fprintf(stderr, "failed to change directory: %s\n", strerror(errno));
        return errno;
    }
#endif

    return mbgl::runTests(argc, argv);
}
