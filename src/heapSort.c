/* 힙 정렬 — 수업에서 다루지 않은 정렬.
 *
 * 배열을 "최대 힙"(부모 >= 자식인 완전 이진 트리)으로 본다. 트리를 따로
 * 만들지 않고 인덱스 계산만으로 부모 · 자식을 찾는다.
 *     i의 자식: 2i+1, 2i+2        i의 부모: (i-1)/2
 *
 *  1단계 (heapify): 마지막 부모 n/2-1부터 거꾸로 내려오며 siftDown.
 *                   아래쪽 부모일수록 내려갈 거리가 짧아 전체가 O(n)이다.
 *  2단계 (추출):    루트(최댓값)를 배열 끝과 맞바꾸고 힙 크기를 1 줄인 뒤,
 *                   새 루트를 siftDown. 이것을 n-1번 되풀이한다 — O(n log n).
 *
 * 보조 배열도 재귀도 없어 추가 메모리가 원소 한 칸뿐이다(O(1)).
 * 대신 멀리 떨어진 원소를 맞바꾸므로 안정 정렬이 아니고, 트리를 위아래로
 * 뛰어다니며 읽어 캐시 효율이 나쁘다.
 */
#include "sort.h"

#include "sortctx.h"

/* c->tmp에 든 원소를 a[i] 자리(비어 있는 "구멍")에서 시작해, 힙 a[0..heapLen)
 * 안의 알맞은 자리까지 내려 보낸다.
 * 교환을 되풀이하면 한 단계마다 이동 3회지만, 구멍을 내려 보내면 1회다. */
static void siftDown(SortCtx *c, size_t i, size_t heapLen) {
    for (;;) {
        size_t child = 2 * i + 1;
        if (child >= heapLen) {
            break;
        }
        /* 두 자식 중 큰 쪽 */
        if (child + 1 < heapLen && sortCompareAt(c, child + 1, child) > 0) {
            child++;
        }
        /* 큰 자식이 들고 있는 값보다 크지 않으면 여기가 자리다. */
        if (sortCompare(c, sortElemAt(c, child), c->tmp) <= 0) {
            break;
        }
        sortMove(c, sortElemAt(c, i), sortElemAt(c, child)); /* 자식을 끌어올림 */
        i = child;
    }
    sortMove(c, sortElemAt(c, i), c->tmp);
}

void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    /* 1단계: 힙 만들기 (Floyd의 방법) */
    for (size_t i = n / 2; i-- > 0;) {
        sortMove(&c, c.tmp, sortElemAt(&c, i));
        siftDown(&c, i, n);
    }
    /* 2단계: 최댓값을 하나씩 뒤로 뺀다 */
    for (size_t end = n - 1; end > 0; end--) {
        sortMove(&c, c.tmp, sortElemAt(&c, end));        /* 끝 원소를 들고 */
        sortMove(&c, sortElemAt(&c, end), sortElemAt(&c, 0)); /* 최댓값을 끝으로 */
        siftDown(&c, 0, end);                            /* 들고 있던 것을 루트에서 내려 보냄 */
    }
    sortEnd(&c);
}
