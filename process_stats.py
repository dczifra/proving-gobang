import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import os

directory = "logs"

dfs=[]
for f in sorted(os.listdir(directory)):
    if f.endswith(".csv"):
        filename = os.path.join(directory, f)
        print("File: ", filename),
        df = pd.read_csv(filename, sep=' ', index_col=False)
        dfs.append(df)

data = pd.concat(dfs)
data["potential"] = round(data["potential"],2)
data["pn_heur"] = round(15 * data["potential"] - data["empty_cells"],2)
data['winner'] = data['pn'].apply(lambda x: 1 if x == 0 else -1)

print("Total: {} rows".format(len(data.index)))
data = data.drop_duplicates()
print("Kept : {} rows".format(len(data.index)))


for t in (0, 1):
    curr_data = data.loc[data['current_player'] == t]
    pots = set(curr_data.potential.unique())
    emptys = set(curr_data.empty_cells.unique())
    dummy_data = []
    for pot in np.arange(0, 4, 0.01):
        for empty in range(0,50):
            if not (pot in pots and empty in emptys):
                dummy_data.append([pot,empty,np.NaN,np.NaN])
    
    curr_data = curr_data.append(pd.DataFrame(dummy_data,columns=["potential","empty_cells","winner","pn_heur"]))
    heatmap_data1 = pd.pivot_table(curr_data, values='winner', index=['empty_cells'], columns='pn_heur', dropna=False)
    heatmap1 = sns.heatmap(heatmap_data1, cmap="icefire", robust=True, vmin=-1, vmax=1)
    heatmap1.figure.savefig("winner_{}.png".format(t), dpi=200)
    heatmap1.figure.clf()

    # curr_data2 = curr_data.loc[curr_data['tree_size'] > 0]
    # heatmap_data2 = pd.pivot_table(curr_data2, values='tree_size', index=['empty_cells'], columns='potential', dropna=False, fill_value=0)
    # heatmap2 = sns.heatmap(heatmap_data2, cmap="vlag")
    # heatmap2.figure.savefig("tree_size_{}.png".format(t))
    # heatmap2.figure.clf()

