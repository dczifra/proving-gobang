import sklearn as sk
from sklearn import preprocessing
from sklearn.linear_model import LogisticRegression, LinearRegression
import pandas as pd
import os
import numpy as np


directory = "logs"
# directory = "/home/doma945/repos/prooving-gobang/logs"

files=[
    "disproof_4x5.csv",
    "disproof_4x6.csv",
    "disproof_4x7.csv",
    "disproof_4x8.csv",
    "disproof_4x9.csv",
#    "disproof_4x10.csv",
#    "disproof_4x11.csv",
    "proof_4x5.csv",
    "proof_4x6.csv",
    "proof_4x7.csv",
    "proof_4x8.csv",
    "proof_4x9.csv",
    "proof_4x10.csv",
    "proof_4x11.csv",
]

dfs=[]
for f in files:
    filename = os.path.join(directory, f)
    print("File: ", filename),
    df = pd.read_csv(filename, sep=' ', index_col=False)
    dfs.append(df)    
data = pd.concat(dfs)

print("Rows: ", len(data.index))
# white black current_player pn dn empty_cells potential node_count l0 l1 l2 l3 l4 l5 l6 l7
input_features = data.iloc[:,[2,5,6,8,9,10,11,12,13,14,15]]
# input_features = data.iloc[:,[2,5,8,9,10,11,12,13,14,15]]

winner = (data.iloc[:,3] == 0) * 1.0
print("White wins:{}%".format(np.mean(winner)))
node_count = data.iloc[:,7]
LR = LogisticRegression(random_state=0, solver='liblinear', multi_class='ovr', class_weight="balanced").fit(input_features, winner)

mask = winner == 1
i1 = input_features.loc[mask]
w1 = winner.loc[mask]
print("Coefficients:\n{}\n".format(LR.coef_))
print("Maker accuracy: ", round(LR.score(i1, w1), 4))
i2 = input_features.loc[- mask]
w2 = winner.loc[-mask]
print("Breaker ccuracy: ", round(LR.score(i2, w2), 4))

LinReg = LinearRegression().fit(input_features, node_count)
print("Node count regression coefficients:\n{}\n".format(LinReg.coef_))
print("Node count regression score: ", round(LinReg.score(input_features, node_count), 4))

# print(reg.score(input_features, node_count))
# print(reg.intercept_)
