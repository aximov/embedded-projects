import sys
import matplotlib.pyplot as plt
import polars as pl

csv_path = sys.argv[1]

df = pl.read_csv(
    csv_path,
    has_header=False,
    new_columns=["timestamp_ms", "ax", "ay", "az"],
)

timestamp_origin = df["timestamp_ms"][0]

df = df.with_columns(
    time_s=(pl.col("timestamp_ms") - timestamp_origin) / 1000.0,
    norm=(pl.col("ax") ** 2 + pl.col("ay") ** 2 + pl.col("az") ** 2) ** 0.5,
)

fig, (axis_plot, norm_plot) = plt.subplots(
    2,
    1,
    figsize=(10, 7),
    sharex=True,
    gridspec_kw={"height_ratios": [3, 1]},
)

axis_plot.plot(df["time_s"], df["ax"], label="ax", linewidth=1.8)
axis_plot.plot(df["time_s"], df["ay"], label="ay", linewidth=1.8)
axis_plot.plot(df["time_s"], df["az"], label="az", linewidth=1.8)
axis_plot.set_ylabel("axis raw value")
axis_plot.legend()
axis_plot.grid(True)

norm_plot.plot(df["time_s"], df["norm"], label="norm", color="black", linewidth=1.8)
norm_plot.set_xlabel("time [s]")
norm_plot.set_ylabel("norm")
norm_plot.legend()
norm_plot.grid(True)

fig.suptitle(csv_path)
plt.tight_layout()
plt.savefig("bno055_plot.png", dpi=150)
plt.show()
