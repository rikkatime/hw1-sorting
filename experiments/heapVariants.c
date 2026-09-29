/* 힙 정렬 학습 내용을 실측으로 확인하는 실험 (보고서 4장).
 *
 *   make heap-exp
 *
 * 1) sift-down 방식: 매 단계 교환(이동 3회) vs 구멍 내려 보내기(이동 1회)
 * 2) 힙 만들기 방식: 하나씩 삽입(sift-up) vs 바닥부터(Floyd, sift-down)
 *    바닥부터는 O(n), 하나씩 삽입은 최악 O(n log n)임을 비교 횟수로 본다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"
#include "sortctx.h"

#define SEED 20260930u

/* 교과서식 sift-down: 큰 자식과 매번 교환한다 */
static void siftDownSwap(SortCtx *c, size_t i, size_t len) {
    for (;;) {
        size_t child = 2 * i + 1;
        if (child >= len) {
            break;
        }
        if (child + 1 < len && sortCompareAt(c, child + 1, child) > 0) {
            child++;
        }
        if (sortCompareAt(c, child, i) <= 0) {
            break;
        }
        sortSwap(c, i, child);
        i = child;
    }
}

static void heapSortSwap(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    for (size_t i = n / 2; i-- > 0;) {
        siftDownSwap(&c, i, n);
    }
    for (size_t end = n - 1; end > 0; end--) {
        sortSwap(&c, 0, end);
        siftDownSwap(&c, 0, end);
    }
    sortEnd(&c);
}

/* 힙 만들기 단계만 떼어 비교 횟수를 센다 */
static size_t buildFloyd(Record *a, size_t n) {
    SortStats st;
    SortCtx c;
    if (!sortBegin(&c, a, n, sizeof(Record), recordCompare, &st)) {
        return 0;
    }
    for (size_t i = n / 2; i-- > 0;) {
        siftDownSwap(&c, i, n);
    }
    sortEnd(&c);
    return st.compares;
}

static size_t buildByInsert(Record *a, size_t n) {
    SortStats st;
    SortCtx c;
    if (!sortBegin(&c, a, n, sizeof(Record), recordCompare, &st)) {
        return 0;
    }
    for (size_t k = 1; k < n; k++) { /* a[k]를 힙 a[0..k)에 넣고 위로 올린다 */
        size_t i = k;
        while (i > 0) {
            size_t parent = (i - 1) / 2;
            if (sortCompareAt(&c, i, parent) <= 0) {
                break;
            }
            sortSwap(&c, i, parent);
            i = parent;
        }
    }
    sortEnd(&c);
    return st.compares;
}

int main(void) {
    const SortAlgorithm swapVersion = {"heapSwap", "", "", 0, heapSortSwap};
    const SortAlgorithm *holeVersion = &SORT_ALGORITHMS[2];

    const size_t n = 100000;
    Record *input = (Record *)malloc(n * sizeof(Record));
    printf("== sift-down 방식 비교 (n = 100,000) ==\n");
    printf("%-10s %12s %12s %9s | %12s %12s %9s\n", "입력", "구멍:비교", "구멍:이동", "ms",
           "교환:비교", "교환:이동", "ms");
    for (int k = 0; k < INPUT_KIND_COUNT; k++) {
        makeInput(input, n, (InputKind)k, SEED);
        BenchResult h = benchRun(holeVersion, input, n, 5);
        BenchResult s = benchRun(&swapVersion, input, n, 5);
        printf("%-10s %12zu %12zu %9.2f | %12zu %12zu %9.2f\n", inputKindSlug((InputKind)k),
               h.stats.compares, h.stats.moves, h.millis, s.stats.compares, s.stats.moves, s.millis);
    }
    free(input);

    printf("\n== 힙 만들기: 비교 횟수 / n ==\n");
    printf("%9s %14s %14s %14s %14s\n", "n", "바닥부터:무작위", "바닥부터:정렬", "삽입:무작위", "삽입:정렬");
    for (size_t m = 1000; m <= 1000000; m *= 10) {
        Record *a = (Record *)malloc(m * sizeof(Record));
        double r[4];
        for (int t = 0; t < 4; t++) {
            makeInput(a, m, t % 2 == 0 ? INPUT_RANDOM : INPUT_SORTED, SEED);
            size_t cmp = t < 2 ? buildFloyd(a, m) : buildByInsert(a, m);
            r[t] = (double)cmp / (double)m;
        }
        printf("%9zu %14.2f %14.2f %14.2f %14.2f\n", m, r[0], r[1], r[2], r[3]);
        free(a);
    }
    return 0;
}
