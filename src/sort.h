/* 정렬 비교 과제 — 퀵 · 병합 · 힙 정렬을 하나의 공통 인터페이스로 묶는다.
 *
 * C에는 interface가 없으므로 "함수 포인터를 담은 구조체"를 인터페이스로 쓴다.
 * 비교 규약은 표준 라이브러리 qsort와 같게 맞춰서, 정렬 코드가 원소의 타입을
 * 전혀 모르게 했다. 덕분에 (key, tag) 쌍으로 안정성을 실측할 수 있다.
 */
#ifndef SORT_H
#define SORT_H

#include <stddef.h>

/* a<b면 음수, a==b면 0, a>b면 양수 (qsort와 같은 규약). */
typedef int (*SortCompare)(const void *a, const void *b);

/* 한 번 정렬하는 동안 모은 측정값. 시계와 무관하게 재현되는 값만 담는다. */
typedef struct SortStats {
    size_t compares;   /* 비교 함수 호출 횟수 */
    size_t moves;      /* 원소 복사 횟수 (교환 1회 = 3) */
    size_t extraBytes; /* 입력 배열 밖에 잡은 작업 공간(바이트) */
    size_t maxDepth;   /* 재귀 깊이 최댓값. 반복문만 쓰면 1 */
} SortStats;

typedef struct SortAlgorithm {
    const char *name;
    const char *timeComplexity;  /* 평균 / 최악 */
    const char *spaceComplexity;
    int stable;                  /* 안정 정렬이라고 "주장"하는 값 — 테스트가 실측과 대조한다 */
    void (*sort)(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
} SortAlgorithm;

void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);

extern const SortAlgorithm SORT_ALGORITHMS[];
extern const size_t SORT_ALGORITHM_COUNT;

/* 퀵 정렬의 피벗 고르는 방법. 기본은 세 값의 중앙값.
 * 피벗 실험(main.c --pivot)이 이 값을 바꿔 가며 잰다. */
typedef enum QuickPivot {
    PIVOT_MEDIAN3, /* 처음 · 가운데 · 끝의 중앙값 */
    PIVOT_FIRST    /* 교과서식: 맨 앞 원소 */
} QuickPivot;
extern QuickPivot quickSortPivot;

void sortStatsReset(SortStats *stats);
int sortCompareInt(const void *a, const void *b);

#endif /* SORT_H */
