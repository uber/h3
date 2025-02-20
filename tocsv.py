import csv
import re
import sys

# Ensure a filename argument is provided
if len(sys.argv) != 2:
    print("Usage: python script.py <filename_without_extension>")
    sys.exit(1)

# Get the base filename from the argument
base_filename = sys.argv[1]
input_file = f"{base_filename}.log"
output_file = f"{base_filename}.csv"

# Pattern to match function name and time
pattern = re.compile(r"-- pentagonChildren_(\S+): (\d+\.\d+) microseconds")

try:
    with open(input_file, "r") as infile, open(output_file, "w", newline="") as outfile:
        csv_writer = csv.writer(outfile)
        csv_writer.writerow(["Function Name", "Time (microseconds)"])  # CSV Header

        for line in infile:
            match = pattern.search(line)
            if match:
                function_suffix, time = match.groups()
                csv_writer.writerow([function_suffix, time])

    print(f"CSV file '{output_file}' has been created successfully.")

except FileNotFoundError:
    print(f"Error: File '{input_file}' not found.")
    sys.exit(1)
