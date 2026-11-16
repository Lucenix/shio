import argparse

import pandas as pd

METADATA_OPS = {
    "open",
    "close",
    "fopen",
    "fclose",
    "open64",
    "close64",
    "fopen64",
    "fclose64",
    "mkdir",
    "socket",
    "mmap",
    "open_variadic",
    "unlink",
    "munmap",
    "creat",
    "creat64",
    "openat",
}
DATA_OPS = {"read", "write", "pread", "pwrite", "pread64", "pwrite64"}


def build_bandwidth_csv(input_csv: str, output_csv: str, nrows: int | None = None) -> pd.DataFrame:
    df = pd.read_csv(
        input_csv,
        header=None,
        usecols=[0, 1, 5],
        names=["operation", "timestamp_ns", "bytes"],
        dtype={"operation": "string", "timestamp_ns": "int64", "bytes": "string"},
        low_memory=False,
        nrows=nrows,
    )

    if df.empty:
        output = pd.DataFrame(columns=["timestamp", "data", "metadata"])
        output.to_csv(output_csv, index=False)
        return output

    df["timestamp_ns"] = pd.to_numeric(df["timestamp_ns"], errors="coerce").astype("int64")
    df["bytes"] = pd.to_numeric(df["bytes"], errors="coerce").fillna(0).astype("int64")

    start_ns = df["timestamp_ns"].min()
    df["second"] = ((df["timestamp_ns"] - start_ns) // 1_000_000_000) + 1

    df["is_metadata"] = df["operation"].isin(METADATA_OPS).astype("int64")
    df["data_bytes"] = df["bytes"].where(df["operation"].isin(DATA_OPS), 0).astype("int64")

    agg = (
        df.groupby("second", as_index=False)
        .agg(data=("data_bytes", "sum"), metadata=("is_metadata", "sum"))
        .sort_values("second")
        .reset_index(drop=True)
    )

    output = pd.DataFrame({"timestamp": range(agg["second"].min() - 1, agg["second"].max() + 1)})
    output = output.merge(agg, left_on="timestamp", right_on="second", how="left").drop(columns=["second"])
    output[["data", "metadata"]] = output[["data", "metadata"]].fillna(0).astype("int64")
    output.to_csv(output_csv, index=False)
    return output


def main():
    parser = argparse.ArgumentParser(description="Turn Trace Replayer trace into data/metadata csv.")
    parser.add_argument("input_csv", help="Original CSV with workload trace")
    parser.add_argument("output_csv", help="CSV file to write the bandwidths")
    parser.add_argument("--nrows", type=int, default=None, help="Limit the number of input rows for testing or sampling.")
    args = parser.parse_args()

    output = build_bandwidth_csv(args.input_csv, args.output_csv, nrows=args.nrows)
    print(output.head())
    print(f"Created {args.output_csv}")


if __name__ == "__main__":
    main()