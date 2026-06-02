import argparse
import math
from dataclasses import dataclass

import polars as pl


@dataclass(frozen=True)
class Window:
    label: str
    start_s: float
    end_s: float


DEFAULT_WINDOWS = [
    Window("通常 0-10 s", 0.0, 10.0),
    Window("振動 15-20 s", 15.0, 20.0),
]


def parse_window(value):
    try:
        label, start_s, end_s = value.rsplit(":", 2)
        return Window(label, float(start_s), float(end_s))
    except ValueError as exc:
        raise argparse.ArgumentTypeError(
            "Window must be LABEL:START_SECONDS:END_SECONDS."
        ) from exc


def read_imu_csv(csv_path):
    df = pl.read_csv(
        csv_path,
        has_header=False,
        new_columns=["timestamp_ms", "ax", "ay", "az"],
    )
    timestamp_origin = df["timestamp_ms"][0]

    return df.with_columns(
        time_s=(pl.col("timestamp_ms") - timestamp_origin) / 1000.0,
        norm=(pl.col("ax") ** 2 + pl.col("ay") ** 2 + pl.col("az") ** 2) ** 0.5,
    )


def summarize(series):
    values = [float(value) for value in series]
    if not values:
        return {
            "n": 0,
            "rms": math.nan,
            "variance": math.nan,
            "mean_abs_delta": math.nan,
            "max_abs_delta": math.nan,
        }

    rms = math.sqrt(sum(value * value for value in values) / len(values))

    if len(values) == 1:
        variance = 0.0
        deltas = []
    else:
        mean = sum(values) / len(values)
        variance = sum((value - mean) ** 2 for value in values) / (len(values) - 1)
        deltas = [
            abs(current - previous)
            for previous, current in zip(values, values[1:])
        ]

    return {
        "n": len(values),
        "rms": rms,
        "variance": variance,
        "mean_abs_delta": sum(deltas) / len(deltas) if deltas else 0.0,
        "max_abs_delta": max(deltas) if deltas else 0.0,
    }


def format_number(value):
    if math.isnan(value):
        return "nan"
    return f"{value:.2f}"


def print_markdown_table(df, windows):
    print("| 区間 | 軸 | n | RMS | 分散 | 平均絶対変化量 | 最大絶対変化量 |")
    print("| --- | --- | ---: | ---: | ---: | ---: | ---: |")

    for window in windows:
        window_df = df.filter(
            (pl.col("time_s") >= window.start_s)
            & (pl.col("time_s") < window.end_s)
        )

        for column in ["ax", "ay", "az", "norm"]:
            stats = summarize(window_df[column])
            print(
                f"| {window.label} | {column} | {stats['n']} | "
                f"{format_number(stats['rms'])} | "
                f"{format_number(stats['variance'])} | "
                f"{format_number(stats['mean_abs_delta'])} | "
                f"{format_number(stats['max_abs_delta'])} |"
            )


def main():
    parser = argparse.ArgumentParser(
        description="Print initial IMU feature comparison as a Markdown table."
    )
    parser.add_argument("csv_path", help="Input CSV path.")
    parser.add_argument(
        "--window",
        action="append",
        type=parse_window,
        help=(
            "Comparison window as LABEL:START_SECONDS:END_SECONDS. "
            "May be specified multiple times."
        ),
    )
    args = parser.parse_args()

    df = read_imu_csv(args.csv_path)
    print_markdown_table(df, args.window or DEFAULT_WINDOWS)


if __name__ == "__main__":
    main()
