/* 공통 토대 — 세 정렬이 함께 쓰는 도구와 구현 표.
 * 알고리즘 자체는 quickSort.c · mergeSort.c · heapSort.c에 있다. */
#include "sort.h"

#include <stdlib.h>
#include <string.h>

#include "sortctx.h"

void sortStatsReset(SortStats *stats) {
    if (stats == NULL) {
        return;
    }
    stats->compares = 0;
    stats->moves = 0;
    stats->extraBytes = 0;
    stats->maxDepth = 1;
}

int sortCompareInt(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y); /* 뺄셈은 overflow 위험이 있어 쓰지 않는다 */
}

int sortBegin(SortCtx *c, void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    sortStatsReset(stats);
    if (base == NULL || cmp == NULL || size == 0 || n < 2) {
        return 0;
    }
    c->base = (char *)base;
    c->size = size;
    c->cmp = cmp;
    c->stats = stats;
    c->depth = 1;
    c->tmp = (char *)malloc(size);
    if (c->tmp == NULL) {
        return 0;
    }
    sortAddExtra(c, size);
    return 1;
}

void sortEnd(SortCtx *c) {
    free(c->tmp);
    c->tmp = NULL;
}

void sortAddExtra(SortCtx *c, size_t bytes) {
    if (c->stats != NULL) {
        c->stats->extraBytes += bytes;
    }
}

void sortEnter(SortCtx *c) {
    c->depth++;
    if (c->stats != NULL && c->depth > c->stats->maxDepth) {
        c->stats->maxDepth = c->depth;
    }
}

void sortLeave(SortCtx *c) {
    c->depth--;
}

char *sortElemAt(const SortCtx *c, size_t i) {
    return c->base + i * c->size;
}

int sortCompare(SortCtx *c, const void *a, const void *b) {
    if (c->stats != NULL) {
        c->stats->compares++;
    }
    return c->cmp(a, b);
}

int sortCompareAt(SortCtx *c, size_t i, size_t j) {
    return sortCompare(c, sortElemAt(c, i), sortElemAt(c, j));
}

void sortMove(SortCtx *c, void *dst, const void *src) {
    memcpy(dst, src, c->size);
    if (c->stats != NULL) {
        c->stats->moves++;
    }
}

void sortSwap(SortCtx *c, size_t i, size_t j) {
    sortMove(c, c->tmp, sortElemAt(c, i));
    sortMove(c, sortElemAt(c, i), sortElemAt(c, j));
    sortMove(c, sortElemAt(c, j), c->tmp);
}

/* 정렬을 하나 더 만들면 파일을 하나 두고 여기에 한 줄 넣는다. */
const SortAlgorithm SORT_ALGORITHMS[] = {
    {"quickSort", "O(n log n) / O(n^2)",      "O(log n)", 0, quickSort},
    {"mergeSort", "O(n log n) / O(n log n)",  "O(n)",     1, mergeSort},
    {"heapSort",  "O(n log n) / O(n log n)",  "O(1)",     0, heapSort},
};

const size_t SORT_ALGORITHM_COUNT = sizeof(SORT_ALGORITHMS) / sizeof(SORT_ALGORITHMS[0]);
