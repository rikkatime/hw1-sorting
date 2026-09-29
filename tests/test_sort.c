/* 유닛 테스트 — 외부 프레임워크 없이 표준 C만 쓴다.  실행: make test
 *
 * 구현 표(SORT_ALGORITHMS)를 훑으므로 정렬을 하나 더 넣으면 그 순간부터
 * 같은 검사를 받는다. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

static int checks = 0;
static int failures = 0;

static void expect(int ok, const char *algo, const char *what) {
    checks++;
    if (!ok) {
        failures++;
        printf("FAIL  %-10s %s\n", algo, what);
    }
}

/* int 배열 하나를 정렬해 want와 맞춰 본다. */
static void expectInts(const SortAlgorithm *s, const char *what,
                       const int *input, const int *want, size_t n) {
    int buf[16];
    memcpy(buf, input, n * sizeof(int));
    s->sort(buf, n, sizeof(int), sortCompareInt, NULL);
    expect(n == 0 || memcmp(buf, want, n * sizeof(int)) == 0, s->name, what);
}

static void basicCases(const SortAlgorithm *s) {
    const int mixed[] = {6, 8, 5, 9, 10, 1, 7, 2, 4, 3};
    const int mixedWant[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const int rev[] = {5, 4, 3, 2, 1};
    const int asc[] = {1, 2, 3, 4, 5};
    const int dup[] = {3, 1, 3, 1, 2, 3};
    const int dupWant[] = {1, 1, 2, 3, 3, 3};
    const int same[] = {7, 7, 7, 7};
    const int two[] = {2, 1};
    const int twoWant[] = {1, 2};
    const int neg[] = {0, -5, 2147483647, -2147483647 - 1, 3};
    const int negWant[] = {-2147483647 - 1, -5, 0, 3, 2147483647};
    const int one[] = {42};
    expectInts(s, "섞인 배열", mixed, mixedWant, 10);
    expectInts(s, "역순", rev, asc, 5);
    expectInts(s, "이미 정렬됨", asc, asc, 5);
    expectInts(s, "중복", dup, dupWant, 6);
    expectInts(s, "모두 같은 값", same, same, 4);
    expectInts(s, "원소 2개", two, twoWant, 2);
    expectInts(s, "음수 · INT 극값", neg, negWant, 5);
    expectInts(s, "원소 1개", one, one, 1);
    expectInts(s, "빈 배열", one, one, 0);
}

/* 난수 배열을 표준 qsort 결과와 대조한다. n = 0..300 전부 + 큰 n 몇 개. */
static void againstQsort(const SortAlgorithm *s) {
    size_t sizes[310];
    size_t count = 0;
    for (size_t n = 0; n <= 300; n++) {
        sizes[count++] = n;
    }
    sizes[count++] = 1023;
    sizes[count++] = 4096;
    sizes[count++] = 10007;
    int ok = 1;
    unsigned state = 12345u;
    for (size_t t = 0; t < count && ok; t++) {
        size_t n = sizes[t];
        int *a = (int *)malloc((n + 1) * sizeof(int));
        int *b = (int *)malloc((n + 1) * sizeof(int));
        for (size_t i = 0; i < n; i++) {
            state ^= state << 13; state ^= state >> 17; state ^= state << 5;
            a[i] = (int)(state % 50u) - 25; /* 중복이 잦게 */
        }
        memcpy(b, a, n * sizeof(int));
        s->sort(a, n, sizeof(int), sortCompareInt, NULL);
        qsort(b, n, sizeof(int), sortCompareInt);
        if (n > 0 && memcmp(a, b, n * sizeof(int)) != 0) {
            ok = 0;
            printf("      n = %zu 에서 qsort와 다름\n", n);
        }
        free(a);
        free(b);
    }
    expect(ok, s->name, "qsort 대조 (n = 0..300, 1023, 4096, 10007)");
}

/* 네 입력 모양 모두에서 정렬되는지 (Record, n = 5000) */
static void allShapes(const SortAlgorithm *s) {
    const size_t n = 5000;
    Record *in = (Record *)malloc(n * sizeof(Record));
    for (int k = 0; k < INPUT_KIND_COUNT; k++) {
        makeInput(in, n, (InputKind)k, 7u);
        BenchResult r = benchRun(s, in, n, 1);
        char what[64];
        snprintf(what, sizeof what, "입력 모양 '%s'", inputKindName((InputKind)k));
        expect(r.sorted, s->name, what);
    }
    free(in);
}

/* 구현 표의 stable 주장이 실측과 맞는가.
 * 중복이 많은 입력에서 재야 의미가 있다 — key가 모두 다르면 어떤 정렬이든
 * "안정"으로 나온다. */
static void stabilityClaim(const SortAlgorithm *s) {
    const size_t n = 2000;
    Record *in = (Record *)malloc(n * sizeof(Record));
    makeInput(in, n, INPUT_FEW_UNIQUE, 99u);
    BenchResult r = benchRun(s, in, n, 1);
    expect(r.stable == s->stable, s->name, "안정성 주장 = 실측");
    free(in);
}

/* 측정값이 말이 되는가 */
static void statsSanity(const SortAlgorithm *s) {
    const size_t n = 4096;
    Record *in = (Record *)malloc(n * sizeof(Record));
    makeInput(in, n, INPUT_RANDOM, 3u);
    BenchResult r = benchRun(s, in, n, 1);
    expect(r.stats.compares > 0 && r.stats.moves > 0, s->name, "비교 · 이동을 센다");
    expect(r.stats.maxDepth >= 1 && r.stats.maxDepth <= 64, s->name, "재귀 깊이가 log 수준");
    free(in);
}

/* 판정 도구 자체가 위반을 잡는지 */
static void toolChecks(void) {
    Record bad[] = {{1, 1}, {1, 0}};
    Record good[] = {{1, 0}, {1, 1}, {2, 2}};
    Record unsorted[] = {{2, 0}, {1, 1}};
    expect(!recordsStable(bad, 2), "bench", "안정성 위반을 잡는다");
    expect(recordsStable(good, 3), "bench", "안정한 결과를 통과시킨다");
    expect(!recordsSorted(unsorted, 2), "bench", "정렬 안 됨을 잡는다");
}

/* 퀵 정렬: 피벗을 맨 앞으로 두면 정렬된 입력에서 비교가 n^2/2 수준이 된다 */
static void quickPivotWorstCase(void) {
    const size_t n = 2000;
    Record *in = (Record *)malloc(n * sizeof(Record));
    makeInput(in, n, INPUT_SORTED, 1u);
    quickSortPivot = PIVOT_FIRST;
    BenchResult first = benchRun(&SORT_ALGORITHMS[0], in, n, 1);
    quickSortPivot = PIVOT_MEDIAN3;
    BenchResult med = benchRun(&SORT_ALGORITHMS[0], in, n, 1);
    expect(first.sorted && first.stats.compares >= n * n / 2, "quickSort", "맨 앞 피벗 + 정렬 입력 = O(n^2)");
    expect(med.stats.compares < n * 20, "quickSort", "중앙값 피벗 + 정렬 입력 = O(n log n)");
    free(in);
}

int main(void) {
    for (size_t a = 0; a < SORT_ALGORITHM_COUNT; a++) {
        const SortAlgorithm *s = &SORT_ALGORITHMS[a];
        int before = failures;
        basicCases(s);
        againstQsort(s);
        allShapes(s);
        stabilityClaim(s);
        statsSanity(s);
        printf("%-4s  %s\n", failures == before ? "ok" : "FAIL", s->name);
    }
    toolChecks();
    quickPivotWorstCase();
    printf("\n%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
