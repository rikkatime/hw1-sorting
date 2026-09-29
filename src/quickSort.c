/* 퀵 정렬 — 피벗을 기준으로 작은 쪽 · 큰 쪽으로 나누고 각각을 다시 정렬한다.
 *
 * 분할은 Hoare 방식(양 끝에서 안쪽으로 훑으며 교환)을 쓴다. Lomuto 방식보다
 * 교환이 적고, 같은 값이 많을 때도 반으로 잘 갈라진다.
 *
 * 두 가지를 신경 썼다.
 *  1) 피벗: 기본은 세 값의 중앙값. 맨 앞 원소를 피벗으로 쓰면 이미 정렬된
 *     입력에서 매번 1 : n-1로 갈라져 O(n^2)가 된다 (--pivot 실험).
 *  2) 재귀 깊이: 짧은 쪽만 재귀하고 긴 쪽은 반복문으로 돈다. 이러면 짧은 쪽은
 *     늘 절반 이하이므로 깊이가 log2(n)을 넘지 않는다. 다만 이것은 스택만
 *     지켜 줄 뿐, 시간의 최악 O(n^2)은 막지 못한다.
 *
 * 떨어진 두 원소를 맞바꾸므로 안정 정렬이 아니다.
 */
#include "sort.h"

#include <stdlib.h>

#include "sortctx.h"

QuickPivot quickSortPivot = PIVOT_MEDIAN3;

/* a[i] > a[j]이면 맞바꾼다. */
static void orderPair(SortCtx *c, size_t i, size_t j) {
    if (sortCompareAt(c, i, j) > 0) {
        sortSwap(c, i, j);
    }
}

/* 피벗 자리를 고른다. 반환값은 hi-1이 될 수 없다 — 그래야 분할이 양쪽 모두
 * 비지 않는다(아래 partition의 주석). */
static size_t choosePivot(SortCtx *c, size_t lo, size_t hi) {
    if (quickSortPivot == PIVOT_FIRST || hi - lo < 3) {
        return lo;
    }
    size_t mid = lo + (hi - lo - 1) / 2; /* 가운데. 길이가 짝수면 왼쪽 */
    /* 세 원소를 정렬해 두면 가운데가 중앙값이고, 양 끝이 경계 역할을 한다. */
    orderPair(c, lo, mid);
    orderPair(c, mid, hi - 1);
    orderPair(c, lo, mid);
    return mid;
}

/* a[lo..hi)를 Hoare 방식으로 나눈다. 반환값 j에 대해
 *   a[lo..j] <= 피벗 <= a[j+1..hi)   이고   lo <= j < hi-1 이다.
 * 피벗 값은 pivot 버퍼에 따로 복사해 둔다(교환 중에 피벗 원소가 움직이므로). */
static size_t partition(SortCtx *c, size_t lo, size_t hi, char *pivot) {
    sortMove(c, pivot, sortElemAt(c, choosePivot(c, lo, hi)));
    size_t i = lo;
    size_t j = hi - 1;
    for (;;) {
        /* '<' · '>'로 멈추므로 피벗과 같은 값에서도 멈춘다. 같은 값이 많을 때
         * 양쪽이 번갈아 멈춰 교환하면서 가운데서 만나게 된다 — 반반 분할. */
        while (sortCompare(c, sortElemAt(c, i), pivot) < 0) {
            i++;
        }
        while (sortCompare(c, sortElemAt(c, j), pivot) > 0) {
            j--;
        }
        if (i >= j) {
            return j;
        }
        sortSwap(c, i, j);
        i++;
        j--;
    }
}

static void quickRange(SortCtx *c, size_t lo, size_t hi, char *pivot) {
    while (hi - lo > 1) {
        size_t p = partition(c, lo, hi, pivot);
        /* 왼쪽 [lo, p], 오른쪽 [p+1, hi). 짧은 쪽을 재귀, 긴 쪽은 반복. */
        if (p + 1 - lo < hi - (p + 1)) {
            sortEnter(c);
            quickRange(c, lo, p + 1, pivot);
            sortLeave(c);
            lo = p + 1;
        } else {
            sortEnter(c);
            quickRange(c, p + 1, hi, pivot);
            sortLeave(c);
            hi = p + 1;
        }
    }
}

void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    char *pivot = (char *)malloc(size); /* 피벗 값 한 칸 */
    if (pivot != NULL) {
        sortAddExtra(&c, size);
        quickRange(&c, 0, n, pivot);
        free(pivot);
    }
    sortEnd(&c);
}
