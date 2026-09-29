#include "bench.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

int recordCompare(const void *a, const void *b) {
    int x = ((const Record *)a)->key;
    int y = ((const Record *)b)->key;
    return (x > y) - (x < y);
}

const char *inputKindName(InputKind kind) {
    static const char *names[] = {"무작위", "정렬됨", "역순", "중복많음"};
    return kind < INPUT_KIND_COUNT ? names[kind] : "?";
}

const char *inputKindSlug(InputKind kind) {
    static const char *names[] = {"random", "sorted", "reversed", "few_unique"};
    return kind < INPUT_KIND_COUNT ? names[kind] : "?";
}

/* xorshift32 — 표준 rand()는 구현마다 수열이 달라 재현성이 떨어진다. */
static unsigned nextRandom(unsigned *state) {
    unsigned x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

void makeInput(Record *a, size_t n, InputKind kind, unsigned seed) {
    unsigned state = seed ? seed : 1u;
    for (size_t i = 0; i < n; i++) {
        int key;
        switch (kind) {
        case INPUT_SORTED:     key = (int)i; break;
        case INPUT_REVERSED:   key = (int)(n - i); break;
        case INPUT_FEW_UNIQUE: key = (int)(nextRandom(&state) % 10u); break;
        default:               key = (int)(nextRandom(&state) % 1000000u); break;
        }
        a[i].key = key;
        a[i].tag = (int)i;
    }
}

int recordsSorted(const Record *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (a[i - 1].key > a[i].key) {
            return 0;
        }
    }
    return 1;
}

int recordsStable(const Record *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (a[i - 1].key == a[i].key && a[i - 1].tag > a[i].tag) {
            return 0;
        }
    }
    return 1;
}

static double nowMillis(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC); /* C11 표준. 고해상도 시계 */
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

BenchResult benchRun(const SortAlgorithm *algo, const Record *input, size_t n, int reps) {
    BenchResult r;
    memset(&r, 0, sizeof r);
    r.algo = algo;
    r.n = n;
    Record *work = (Record *)malloc((n ? n : 1) * sizeof(Record));
    if (work == NULL) {
        return r;
    }
    double total = 0.0;
    for (int k = 0; k < reps; k++) {
        memcpy(work, input, n * sizeof(Record)); /* 복사는 시간에 넣지 않는다 */
        double t0 = nowMillis();
        algo->sort(work, n, sizeof(Record), recordCompare, &r.stats);
        total += nowMillis() - t0;
    }
    r.millis = total / (reps > 0 ? reps : 1);
    r.sorted = recordsSorted(work, n);
    r.stable = recordsStable(work, n);
    free(work);
    return r;
}
