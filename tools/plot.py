"""report/data/*.csv 로 보고서용 그래프를 그린다.  실행: make charts

측정은 C(main.out)가 하고, 이 스크립트는 그림만 그린다.
matplotlib이 필요하다: pip install matplotlib
"""
import csv
import math
import os

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib import font_manager  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA = os.path.join(ROOT, "report", "data")
OUT = os.path.join(ROOT, "report", "figures")

# 한글 글꼴: 시스템 글꼴 폴더의 CJK 글꼴을 등록한 뒤, 설치된 것 중 첫 번째를 쓴다.
for path in font_manager.findSystemFonts():
    if "CJK" in path or "Nanum" in path:
        try:
            font_manager.fontManager.addfont(path)
        except Exception:  # noqa: BLE001 — 읽을 수 없는 글꼴은 건너뛴다
            pass
for name in ["Noto Sans CJK KR", "Noto Sans CJK JP", "NanumGothic", "Malgun Gothic", "AppleGothic"]:
    if any(name in f.name for f in font_manager.fontManager.ttflist):
        plt.rcParams["font.family"] = name
        break
plt.rcParams["axes.unicode_minus"] = False
plt.rcParams["figure.dpi"] = 150

ALGOS = ["quickSort", "mergeSort", "heapSort"]
LABEL = {"quickSort": "퀵", "mergeSort": "병합", "heapSort": "힙"}
COLOR = {"quickSort": "#2563eb", "mergeSort": "#16a34a", "heapSort": "#ea580c"}
SHAPES = ["random", "sorted", "reversed", "few_unique"]
SHAPE_KO = {"random": "무작위", "sorted": "정렬됨", "reversed": "역순", "few_unique": "중복많음"}


def load(name):
    with open(os.path.join(DATA, name + ".csv"), encoding="utf-8") as f:
        rows = list(csv.DictReader(f))
    for r in rows:
        for k in ["n", "compares", "moves", "extra_bytes", "max_depth"]:
            r[k] = int(r[k])
        r["ms"] = float(r["ms"])
    return rows


def save(fig, name):
    fig.tight_layout()
    fig.savefig(os.path.join(OUT, name + ".png"))
    plt.close(fig)
    print("wrote", name + ".png")


def grouped_bars(ax, rows, field, title, scale=1.0):
    width = 0.26
    for i, algo in enumerate(ALGOS):
        vals = [next(r[field] for r in rows if r["group"] == s and r["algorithm"] == algo) / scale
                for s in SHAPES]
        xs = [j + (i - 1) * width for j in range(len(SHAPES))]
        ax.bar(xs, vals, width, label=LABEL[algo], color=COLOR[algo])
    ax.set_xticks(range(len(SHAPES)))
    ax.set_xticklabels([SHAPE_KO[s] for s in SHAPES])
    ax.set_title(title)
    ax.grid(axis="y", alpha=0.3)


def shapes():
    rows = load("shapes")
    fig, ax = plt.subplots(figsize=(7, 3.4))
    grouped_bars(ax, rows, "ms", "입력 모양별 걸린 시간 (n = 100,000)")
    ax.set_ylabel("ms")
    ax.legend()
    save(fig, "shapes_time")

    fig, axes = plt.subplots(1, 2, figsize=(10, 3.4))
    grouped_bars(axes[0], rows, "compares", "비교 횟수 (백만)", 1e6)
    grouped_bars(axes[1], rows, "moves", "이동 횟수 (백만)", 1e6)
    axes[0].legend()
    save(fig, "shapes_ops")


def growth():
    rows = load("growth")
    fig, axes = plt.subplots(1, 2, figsize=(10, 3.6))
    for algo in ALGOS:
        rs = [r for r in rows if r["algorithm"] == algo]
        ns = [r["n"] for r in rs]
        axes[0].plot(ns, [r["ms"] for r in rs], "o-", label=LABEL[algo], color=COLOR[algo])
        axes[1].plot(ns, [r["compares"] / (r["n"] * math.log2(r["n"])) for r in rs], "o-",
                     label=LABEL[algo], color=COLOR[algo])
    axes[0].set_xscale("log", base=2)
    axes[0].set_yscale("log")
    axes[0].set_title("n에 따른 시간 (무작위, 로그-로그)")
    axes[0].set_xlabel("n")
    axes[0].set_ylabel("ms")
    axes[1].set_xscale("log", base=2)
    axes[1].set_ylim(0, 2.2)
    axes[1].set_title("비교 횟수 ÷ (n log2 n)")
    axes[1].set_xlabel("n")
    for ax in axes:
        ax.grid(alpha=0.3, which="both")
        ax.legend()
    save(fig, "growth")


def memory():
    rows = load("growth")
    fig, axes = plt.subplots(1, 2, figsize=(10, 3.4))
    for algo in ALGOS:
        rs = [r for r in rows if r["algorithm"] == algo]
        ns = [r["n"] for r in rs]
        axes[0].plot(ns, [r["extra_bytes"] for r in rs], "o-", label=LABEL[algo], color=COLOR[algo])
        axes[1].plot(ns, [r["max_depth"] for r in rs], "o-", label=LABEL[algo], color=COLOR[algo])
    axes[0].set_xscale("log", base=2)
    axes[0].set_yscale("log")
    axes[0].set_title("추가 메모리 (바이트, 로그-로그)")
    axes[1].set_xscale("log", base=2)
    axes[1].set_title("최대 재귀 깊이")
    for ax in axes:
        ax.set_xlabel("n")
        ax.grid(alpha=0.3, which="both")
        ax.legend()
    save(fig, "memory")


def pivot():
    rows = load("pivot")
    fig, ax = plt.subplots(figsize=(7, 3.6))
    for label, color, name in [("first", "#dc2626", "맨 앞 원소"), ("median3", "#2563eb", "세 값의 중앙값")]:
        rs = [r for r in rows if r["algorithm"] == label]
        ax.plot([r["n"] for r in rs], [r["compares"] for r in rs], "o-", color=color, label=name)
    ns = sorted({r["n"] for r in rows})
    ax.plot(ns, [n * n / 2 for n in ns], ":", color="gray", label="n²/2")
    ax.plot(ns, [n * math.log2(n) for n in ns], "--", color="gray", label="n log2 n")
    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_title("퀵 정렬 피벗 선택 — 이미 정렬된 입력의 비교 횟수")
    ax.set_xlabel("n")
    ax.grid(alpha=0.3, which="both")
    ax.legend()
    save(fig, "pivot")


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    shapes()
    growth()
    memory()
    pivot()
