/**
 * Test runner for Generic Binary heap
 */

#include <libdxwifi/details/heap.h>
#include <libdxwifi/details/utils.h>
#include <libdxwifi/details/logging.h>

void logger(enum dxwifi_log_level log_level, const char* fmt, va_list args) {
    // For now just log everything to stdout
    fprintf(stderr, "[ %s ] : ", log_level_to_str(log_level));
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

bool less_than(const uint8_t* lhs, const uint8_t* rhs) {
    int* a = (int*) lhs;
    int* b = (int*) rhs;

    return *a < *b;
}

int main(int argc, char** argv) {

    init_logging(DXWIFI_LOG_TRACE, logger);

    binary_heap heap;

    int nums[]     = { 4, 3, 2, 1, 0, 1, 2, 3, 4, 5 };
    int expected[] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5 };

    init_heap(&heap, 10 * sizeof(int), sizeof(int), less_than);

    for (size_t i = 0; i < sizeof(nums)/sizeof(int); ++i)
    {
        heap_push(&heap, &nums[i]);
    }

    int temp = 0;
    int i = 0;
    while(heap_pop(&heap, &temp)) {
        assert_continue(temp == expected[i], "Fail: %d != %d", temp, expected[i]);
        ++i;
    }
    


}