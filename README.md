# 과제1. 정렬 비교 — 퀵 · 병합 · 힙

2026-2 고급알고리즘(SIT2001-01) 과제1. 퀵 정렬 · 병합 정렬 · 힙 정렬(수업에서
다루지 않은 정렬)을 하나의 공통 인터페이스로 묶고, 같은 입력에서 시간 · 비교 ·
이동 · 추가 메모리 · 재귀 깊이 · 안정성을 비교한다.

- 보고서: [`report/REPORT.md`](report/REPORT.md)
- 이 저장소는 [lec-algorithm/algorithm-env](https://github.com/lec-algorithm/algorithm-env) 템플릿으로 시작했다.

## 실행

컨테이너 안(Codespaces 터미널 또는 `docker compose exec lab bash`)에서:

```sh
make run     # 세 정렬 비교 표 (입력 모양별 · n 증가 · 퀵 피벗 실험)
make test    # 유닛 테스트
make data    # 측정값을 report/data/*.csv로 저장
make charts  # CSV로 report/figures/*.png 생성 (matplotlib 필요: pip install matplotlib)
```

## 구조

```plaintext
src/    sort.h · sort.c · sortctx.h        공통 인터페이스와 도구
        quickSort.c · mergeSort.c · heapSort.c
        bench.h · bench.c                  입력 생성 · 측정 · 판정
        main.c                             실험 실행기
tests/  test_sort.c                        유닛 테스트 (표준 C만 사용)
tools/  plot.py                            그래프
report/ REPORT.md · data/ · figures/
```
