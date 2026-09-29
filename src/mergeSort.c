/* 병합 정렬 — 반으로 나눠 각각 정렬한 뒤, 정렬된 두 쪽을 합친다.
 *
 * 입력 모양과 상관없이 늘 O(n log n)이고 안정 정렬이다. 대신 합칠 때
 * 원소를 잠시 담아 둘 보조 배열이 필요하다 — 이것이 병합 정렬이 치르는 값이다.
 *
 * 두 가지를 손봤다.
 *  1) 보조 배열을 n이 아니라 n/2만 잡는다. 합칠 때 "왼쪽 절반"만 보조 배열로
 *     빼 두고, 결과는 원래 배열 앞에서부터 채운다. 쓰는 자리가 아직 읽지 않은
 *     오른쪽 원소를 앞지르는 일이 없으므로 이것으로 충분하다.
 *  2) 왼쪽의 마지막 <= 오른쪽의 첫 원소이면 이미 이어져 있으므로 합치지 않는다.
 *     정렬된 입력에서 비교가 n-1번으로 끝난다.
 */
#include "sort.h"

#include <stdlib.h>

#include "sortctx.h"

/* a[lo..mid)와 a[mid..hi)는 각각 정렬되어 있다. 둘을 합친다. */
static void merge(SortCtx *c, size_t lo, size_t mid, size_t hi, char *aux) {
    size_t leftLen = mid - lo;
    for (size_t k = 0; k < leftLen; k++) {
        sortMove(c, aux + k * c->size, sortElemAt(c, lo + k));
    }
    size_t i = 0;   /* aux(왼쪽)에서 읽을 자리 */
    size_t j = mid; /* 오른쪽에서 읽을 자리 */
    size_t k = lo;  /* 결과를 쓸 자리 */
    while (i < leftLen && j < hi) {
        /* 오른쪽이 "더 작을 때만" 오른쪽을 가져온다. 같으면 왼쪽이 먼저 —
         * 이 한 줄이 안정성을 만든다. ('<='로 바꾸면 불안정해진다.) */
        if (sortCompare(c, sortElemAt(c, j), aux + i * c->size) < 0) {
            sortMove(c, sortElemAt(c, k++), sortElemAt(c, j++));
        } else {
            sortMove(c, sortElemAt(c, k++), aux + (i++) * c->size);
        }
    }
    while (i < leftLen) { /* 왼쪽이 남았으면 마저 옮긴다 */
        sortMove(c, sortElemAt(c, k++), aux + (i++) * c->size);
    }
    /* 오른쪽이 남았다면 이미 제자리에 있다. */
}

static void mergeRange(SortCtx *c, size_t lo, size_t hi, char *aux) {
    if (hi - lo < 2) {
        return;
    }
    size_t mid = lo + (hi - lo) / 2;
    sortEnter(c);
    mergeRange(c, lo, mid, aux);
    mergeRange(c, mid, hi, aux);
    sortLeave(c);
    if (sortCompareAt(c, mid - 1, mid) <= 0) {
        return; /* 이미 이어져 있다 */
    }
    merge(c, lo, mid, hi, aux);
}

void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    size_t auxLen = n / 2; /* 왼쪽 절반의 최대 길이 */
    char *aux = (char *)malloc(auxLen * size);
    if (aux != NULL) {
        sortAddExtra(&c, auxLen * size);
        mergeRange(&c, 0, n, aux);
        free(aux);
    }
    sortEnd(&c);
}
