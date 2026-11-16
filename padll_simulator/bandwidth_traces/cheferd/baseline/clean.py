import pandas as pd

def split_csv(input_file):
    # Read the input CSV
    df = pd.read_csv(input_file)

    # Get column names
    columns = df.columns.tolist()

    # First column is shared across all output files
    first_col = columns[0]

    # Remaining columns should be split into 4 pairs
    remaining_cols = columns[1:]

    if len(remaining_cols) != 8:
        raise ValueError(
            f"Expected 8 columns after the first column, "
            f"but found {len(remaining_cols)}."
        )

    # Create 4 CSV files, each containing the first column and a pair
    for i in range(4):
        pair = remaining_cols[i * 2:(i + 1) * 2]

        if i > 0:
            df[first_col] -= 180
        output_df = df[[first_col] + pair]
        output_df = output_df.drop(output_df[(output_df[first_col] < 0) | (output_df[first_col] > 2160)].index)

        output_file = f"25_n{i}.csv"
        output_df.to_csv(output_file, index=False)

        print(f"Created {output_file}")


# Run the script
split_csv("25_stacked.csv")