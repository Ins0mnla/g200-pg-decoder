#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

extern int test_type0();
extern int test_type1();

static int tests_passed = 0;
static int tests_failed = 0;

void check(bool condition, const char* name) {
    if (condition) {
        tests_passed++;
        printf("  PASS: %s\n", name);
    } else {
        tests_failed++;
        printf("  FAIL: %s\n", name);
    }
}

int main() {
    printf("=== PG Packet Decoder Test Suite ===\n");

    test_type0();
    test_type1();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
