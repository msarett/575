/**
 *  Copyright 2015 Mike Reed
 */

#include "tests.h"
#include <string>

static bool is_arg(const char arg[], const char target[]) {
    std::string str("--");
    str += target;
    if (!strcmp(arg, str.c_str())) {
        return true;
    }

    char buffer[3];
    buffer[0] = '-';
    buffer[1] = target[0];
    buffer[2] = 0;
    return !strcmp(arg, buffer);
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (is_arg(argv[i], "verbose")) {
            gTestSuite_Verbose = true;
        } else if (is_arg(argv[i], "crash")) {
            gTestSuite_CrashOnFailure = true;
        } else if (is_arg(argv[i], "help")) {
            printf("tests [--verbose][-v] [--crash][-c] [--help][-h]\n");
            printf("--help     show this text\n");
            printf("--crash    crash if a test fails (good when running in a debugger)\n");
            printf("--verbose  give verbose status for each test\n");
            return 0;
        }
    }

    GTestStats stats;

    for (int i = 0; gTestRecs[i].fProc; ++i) {
        GTestStats localStats;
        gTestRecs[i].fProc(&localStats);
        if (gTestSuite_Verbose) {
            printf("%16s: [%3d/%3d]  %g\n", gTestRecs[i].fName,
                   localStats.fPassCounter, localStats.fTestCounter, localStats.percent());
        }
        stats.fTestCounter += localStats.fTestCounter;
        stats.fPassCounter += localStats.fPassCounter;
    }

    printf("%16s: [%3d/%3d]  %g\n", "tests",
           stats.fPassCounter, stats.fTestCounter, stats.percent());
    return 0;
}
