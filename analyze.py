import pandas as pd
import matplotlib.pyplot as plt

df = pd.DataFrame(columns={"white", "black", "common", "depth", "intersection"})
df = pd.read_csv("ands.txt")

print(df["depth"])
plt.hist(df["depth"])

print(len(df))
