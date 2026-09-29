/* 정렬 비교 실행기.
 *
 *   ./src/main.out              사람이 읽는 표 (실험 셋 전부)
 *   ./src/main.out --csv shapes 입력 모양별 측정을 CSV로
 *   ./src/main.out --csv growth n을 키우며 잰 측정을 CSV로
 *   ./src/main.out --csv pivot  퀵 정렬 피벗 실험을 CSV로
 *
 * 무엇을 잴지는 아래 실험 함수에만 적혀 있고, 표로 찍을지 CSV로 찍을지는
 * 함수 포인터(RowSink)로 갈아 끼운다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

#define SEED 20260930u

typedef void (*RowSink)(const char *group, const char *label, const BenchResult *r);

static void printRow(const char *group, const char *label, const BenchResult *r) {
    printf("%-10s %-14s %8zu %10.3f %12zu %12zu %9zu %5zu  %s%s\n",
           group, label, r->n, r->millis, r->stats.compares, r->stats.moves,
           r->stats.extraBytes, r->stats.maxDepth,
           r->stable ? "안정" : "불안정", r->sorted ? "" : "  ** 정렬 실패 **");
}

static void csvRow(const char *group, const char *label, const BenchResult *r) {
    printf("%s,%s,%zu,%.4f,%zu,%zu,%zu,%zu,%d,%d\n",
           group, label, r->n, r->millis, r->stats.compares, r->stats.moves,
           r->stats.extraBytes, r->stats.maxDepth, r->stable, r->sorted);
}

static void header(RowSink sink, const char *title) {
    if (sink == csvRow) {
        printf("group,algorithm,n,ms,compares,moves,extra_bytes,max_depth,stable,sorted\n");
    } else {
        printf("\n== %s ==\n", title);
        printf("%-10s %-14s %8s %10s %12s %12s %9s %5s  %s\n",
               "그룹", "알고리즘", "n", "시간(ms)", "비교", "이동", "추가(B)", "깊이", "안정성");
    }
}

static int repsFor(size_t n) {
    return n <= 10000 ? 20 : (n <= 100000 ? 5 : 3);
}

/* 실험 1: 입력 모양별 (n 고정) */
static void runShapes(RowSink sink) {
    const size_t n = 100000;
    Record *input = (Record *)malloc(n * sizeof(Record));
    header(sink, "입력 모양별 (n = 100,000)");
    for (int k = 0; k < INPUT_KIND_COUNT; k++) {
        makeInput(input, n, (InputKind)k, SEED);
        for (size_t a = 0; a < SORT_ALGORITHM_COUNT; a++) {
            BenchResult r = benchRun(&SORT_ALGORITHMS[a], input, n, repsFor(n));
            sink(sink == csvRow ? inputKindSlug((InputKind)k) : inputKindName((InputKind)k),
                 SORT_ALGORITHMS[a].name, &r);
        }
    }
    free(input);
}

/* 실험 2: n을 두 배씩 (무작위) */
static void runGrowth(RowSink sink) {
    header(sink, "n을 두 배씩 키우며 (무작위)");
    for (size_t n = 1000; n <= 512000; n *= 2) {
        Record *input = (Record *)malloc(n * sizeof(Record));
        makeInput(input, n, INPUT_RANDOM, SEED);
        for (size_t a = 0; a < SORT_ALGORITHM_COUNT; a++) {
            BenchResult r = benchRun(&SORT_ALGORITHMS[a], input, n, repsFor(n));
            sink("growth", SORT_ALGORITHMS[a].name, &r);
        }
        free(input);
    }
}

/* 실험 3: 퀵 정렬의 피벗 — 이미 정렬된 입력에서 맨 앞 vs 세 값의 중앙값 */
static void runPivot(RowSink sink) {
    header(sink, "퀵 정렬 피벗 (정렬된 입력)");
    const SortAlgorithm *quick = &SORT_ALGORITHMS[0];
    for (size_t n = 1000; n <= 32000; n *= 2) {
        Record *input = (Record *)malloc(n * sizeof(Record));
        makeInput(input, n, INPUT_SORTED, SEED);
        quickSortPivot = PIVOT_FIRST;
        BenchResult first = benchRun(quick, input, n, 3);
        quickSortPivot = PIVOT_MEDIAN3;
        BenchResult med = benchRun(quick, input, n, 3);
        sink("pivot", "first", &first);
        sink("pivot", "median3", &med);
        free(input);
    }
    quickSortPivot = PIVOT_MEDIAN3;
}

int main(int argc, char **argv) {
    if (argc >= 3 && strcmp(argv[1], "--csv") == 0) {
        if (strcmp(argv[2], "shapes") == 0) {
            runShapes(csvRow);
        } else if (strcmp(argv[2], "growth") == 0) {
            runGrowth(csvRow);
        } else if (strcmp(argv[2], "pivot") == 0) {
            runPivot(csvRow);
        } else {
            fprintf(stderr, "알 수 없는 실험: %s\n", argv[2]);
            return 1;
        }
        return 0;
    }
    printf("정렬 비교 — 원소는 Record(8B), 시간은 평균, 비교·이동은 재현되는 값\n");
    for (size_t a = 0; a < SORT_ALGORITHM_COUNT; a++) {
        const SortAlgorithm *s = &SORT_ALGORITHMS[a];
        printf("  %-10s 시간 %-24s 공간 %-9s %s\n", s->name, s->timeComplexity,
               s->spaceComplexity, s->stable ? "안정(주장)" : "불안정(주장)");
    }
    runShapes(printRow);
    runGrowth(printRow);
    runPivot(printRow);
    return 0;
}
