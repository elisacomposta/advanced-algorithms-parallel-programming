import pandas as pd

# Read csv
real_data = pd.read_csv("output_real.csv", sep=" ")
my_data = pd.read_csv("output_mine.csv", sep=" ")

# Compare column names
print("Real Data Columns:", real_data.columns)
print("My Data Columns:", my_data.columns)
real_data.columns = real_data.columns.str.strip()
my_data.columns = my_data.columns.str.strip()

# Sort dataframes according to column "NGRAM"
"""real_data = real_data.sort_values(by="NGRAM").reset_index(drop=True)
my_data = my_data.sort_values(by="NGRAM").reset_index(drop=True)
"""
# Compare data
differences = real_data.compare(my_data)
if differences.empty:
    print("The two output files are identical.")
else:
    print("The differences are:")
    print(differences)
