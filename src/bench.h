/* 세 정렬을 같은 잣대로 재는 도구.
 * 정렬은 자기가 측정당하는 줄 모르고, 측정은 어떤 정렬인지 모른다.
 * 둘을 잇는 것은 SortAlgorithm 구조체뿐이다. */
#ifndef BENCH_H
#define BENCH_H

#include <stddef.h>

#include "sort.h"

/* 측정용 원소. key로 정렬하고 tag에 입력 순서를 새겨 둔다.
 * 정렬 뒤 같은 key끼리 tag가 오름차순이면 안정 정렬이다. */
typedef struct Record {
    int key;
    int tag;
} Record;

int recordCompare(const void *a, const void *b); /* key만 본다 */

typedef enum InputKind {
    INPUT_RANDOM,     /* 무작위 */
    INPUT_SORTED,     /* 이미 정렬됨 */
    INPUT_REVERSED,   /* 역순 */
    INPUT_FEW_UNIQUE, /* 중복 많음 (서로 다른 key 10개) */
    INPUT_KIND_COUNT
} InputKind;

const char *inputKindName(InputKind kind);   /* 표에 찍는 한글 이름 */
const char *inputKindSlug(InputKind kind);   /* CSV에 쓰는 영문 이름 */

/* seed가 같으면 어느 기계에서나 같은 입력이 나온다 (rand()를 쓰지 않는다). */
void makeInput(Record *a, size_t n, InputKind kind, unsigned seed);

int recordsSorted(const Record *a, size_t n);
int recordsStable(const Record *a, size_t n);

typedef struct BenchResult {
    const SortAlgorithm *algo;
    size_t n;
    double millis;   /* reps회 평균 */
    SortStats stats; /* 마지막 회차 측정값 (입력이 같으므로 매 회차 같다) */
    int sorted;
    int stable;      /* 실측 */
} BenchResult;

BenchResult benchRun(const SortAlgorithm *algo, const Record *input, size_t n, int reps);

#endif /* BENCH_H */
