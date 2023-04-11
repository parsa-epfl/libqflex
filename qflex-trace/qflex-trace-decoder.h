#include <stdint.h>
#include <stdbool.h>

typedef struct TraceParams {
    int size;
    bool is_load;
    bool is_store;
    bool is_vector;
    bool is_signed;
    bool is_pair;
    int accesses;
} TraceParams;

bool aarch64_insn_get_params(TraceParams *s, uint32_t insn);