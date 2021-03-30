import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import os

#directory = "logs"
directory = "/home/doma945/repos/prooving-gobang/build/data"

files=[
    "disproof/4x7.csv",
    "disproof/4x8.csv",
    "disproof/4x9.csv",
    "disproof/4x10.csv",
    "proof/4x7.csv",
    "proof/4x8.csv",
    "proof/4x9.csv",
    "proof/4x10.csv",
]

def read_files(files):
    dfs=[]
    for f in files:
        filename = os.path.join(directory, f)
        print("File: ", filename),
        df = pd.read_csv(filename, sep=' ', header=None, index_col=False, skiprows=1, names = ["white","black","current_player","pn","dn","empty_cells","potential","node_count","l0","l1","l2","l3","l4","l5","l6","l7"])
        dfs.append(df)    
    data = pd.concat(dfs)
    return data

    
data = read_files(files)
data["potential"] = round(data["potential"],2)
data["pn_heur"] = round(15 * data["potential"] - data["empty_cells"],2)
data['winner'] = data['pn'].apply(lambda x: 1 if x == 0 else 0)

print("Total: {} rows".format(len(data.index)))
data = data.drop_duplicates()
print("Kept : {} rows".format(len(data.index)))


#######################################################

# hbs = [2,2,1.9,2.1,2,2,2,2]
# data["heuristic"] = round(data.l0 * hbs[0] ** 0 + data.l1 * hbs[1] ** -1 + data.l2 * hbs[2] ** -2 + data.l3 * hbs[3] ** -3 + data.l4 * hbs[4] ** -4 + data.l5 * hbs[5] ** -5 + data.l6 * hbs[6] ** -6 + data.l7 * hbs[7] ** -7, 2)

pd.set_option('display.max_rows', None)

# # for column in ('potential', 'heuristic'):
# for column in ('potential', ):
#     keys = [column, 'empty_cells']
#     for t in (0, 1):
#         curr_data = data.loc[data['current_player'] == t]
#         curr_data = curr_data.loc[curr_data.groupby(keys)["winner"].transform("std") > 0]
    
#         for col in ["l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7"]:
#             curr_data_x = curr_data.loc[curr_data.groupby(keys)[col].transform("std") > 0]
#             curr_data_x = curr_data_x.loc[curr_data_x.groupby(keys)[col].transform("size") > 100]

#             if len(curr_data_x.index) > 0:
#                 corr = curr_data_x.groupby(keys)[["winner",col]].corr().unstack().iloc[:,1]
#                 c = pd.DataFrame({'corr': corr}).reset_index()
#                 # print("Column: ", col)
#                 # print(corr)
            
#                 pivot = pd.pivot_table(c, values="corr", index=["empty_cells"], columns=column, dropna=False)
#                 heatmap = sns.heatmap(pivot, cmap="icefire", robust=True, vmin=-1, vmax=1)
#                 heatmap.figure.savefig("corr_{}_{}_{}.png".format(t,column,col), dpi=200)
#                 heatmap.figure.clf()


#######################################################
# xxx


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
    curr_data["potential"] = round(curr_data["potential"],2)
    curr_data["empty cells"] = curr_data["empty_cells"]
    heatmap_data1 = pd.pivot_table(curr_data, values='winner', index=['empty cells'], columns='potential', dropna=False)
    heatmap1 = sns.heatmap(heatmap_data1, cmap="icefire", robust=True, vmin=0, vmax=1)
    heatmap1.figure.savefig("winner_{}.png".format(t), dpi=200)
    heatmap1.figure.clf()

    # curr_data2 = curr_data.loc[curr_data['tree_size'] > 0]
    # heatmap_data2 = pd.pivot_table(curr_data2, values='tree_size', index=['empty_cells'], columns='potential', dropna=False, fill_value=0)
    # heatmap2 = sns.heatmap(heatmap_data2, cmap="vlag")
    # heatmap2.figure.savefig("tree_size_{}.png".format(t))
    # heatmap2.figure.clf()

