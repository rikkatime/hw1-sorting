/* 정렬 구현끼리만 쓰는 도구 (구현 전용 헤더).
 * main.c · bench.c · 테스트는 이 파일을 include하지 않는다. */
#ifndef SORTCTX_H
#define SORTCTX_H

#include <stddef.h>

#include "sort.h"

/* 정렬 함수의 인자를 한 덩어리로 들고 다닌다.
 * tmp는 원소 한 칸짜리 임시 자리(교환 · 구멍 채우기 · 피벗 보관에 쓴다). */
typedef struct SortCtx {
    char *base;
    size_t size;
    SortCompare cmp;
    SortStats *stats;
    char *tmp;
    size_t depth; /* 지금 재귀 깊이 */
} SortCtx;

/* 정렬할 것이 없으면 0을 돌려주고, 그때는 sortEnd를 부르지 않는다. */
int sortBegin(SortCtx *c, void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void sortEnd(SortCtx *c);

void sortAddExtra(SortCtx *c, size_t bytes); /* 추가 메모리를 장부에 적는다 */
void sortEnter(SortCtx *c);                  /* 재귀 한 단계 들어감 */
void sortLeave(SortCtx *c);

char *sortElemAt(const SortCtx *c, size_t i);
int sortCompare(SortCtx *c, const void *a, const void *b); /* 세면서 비교 */
int sortCompareAt(SortCtx *c, size_t i, size_t j);
void sortMove(SortCtx *c, void *dst, const void *src);     /* 세면서 복사 */
void sortSwap(SortCtx *c, size_t i, size_t j);

#endif /* SORTCTX_H */
