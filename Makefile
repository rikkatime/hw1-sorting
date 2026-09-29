# 빌드와 테스트를 한 단어로 돌리기 위한 Makefile.
# 컨테이너 안에서 실행한다 (docker compose exec lab bash).
#
#   make run     세 정렬 비교 표 출력
#   make test    유닛 테스트
#   make data    측정값을 report/data/*.csv로 저장
#   make heap-exp 힙 정렬 변형 실험 (보고서 4장)
#   make charts  CSV로 그래프(PNG) 생성 — matplotlib 필요 (pip install matplotlib)
#   make debug   디버그 심볼을 넣어 빌드 (VS Code의 F5가 쓴다)
#   make clean   빌드 산출물 정리
#
# 실행 파일은 `*.out`으로 만든다. .gitignore가 그것만 걸러낸다.

CC ?= gcc
CFLAGS ?= -std=c17 -Wall -Wextra -O2
DEBUGFLAGS ?= -std=c17 -Wall -Wextra -g -O0

.PHONY: all run run-c test test-c data charts heap-exp debug clean

all: test

run: run-c

run-c: src/main.out
	@./src/main.out

test: test-c

test-c: tests/test_sort.out
	@./tests/test_sort.out

data: src/main.out
	@mkdir -p report/data
	./src/main.out --csv shapes > report/data/shapes.csv
	./src/main.out --csv growth > report/data/growth.csv
	./src/main.out --csv pivot  > report/data/pivot.csv

charts:
	python3 tools/plot.py

debug: src/main.debug.out

# 파일 하나를 그 자리에서 빌드한다. 같은 폴더의 .c를 함께 링크한다.
%.out: %.c
	$(CC) $(CFLAGS) -I$(@D) -o $@ $(wildcard $(@D)/*.c)

%.debug.out: %.c
	$(CC) $(DEBUGFLAGS) -I$(@D) -o $@ $(wildcard $(@D)/*.c)

SORT_SRC = src/sort.c src/quickSort.c src/mergeSort.c src/heapSort.c

tests/test_sort.out: tests/test_sort.c $(SORT_SRC) src/sort.h src/sortctx.h src/bench.c src/bench.h
	$(CC) $(CFLAGS) -Isrc -o $@ tests/test_sort.c $(SORT_SRC) src/bench.c

experiments/heapVariants.out: experiments/heapVariants.c $(SORT_SRC) src/bench.c src/sort.h src/sortctx.h src/bench.h
	$(CC) $(CFLAGS) -Isrc -o $@ experiments/heapVariants.c $(SORT_SRC) src/bench.c

heap-exp: experiments/heapVariants.out
	@./experiments/heapVariants.out

clean:
	rm -f src/*.out tests/*.out experiments/*.out
