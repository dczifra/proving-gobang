import sklearn as sk
from sklearn import preprocessing
from sklearn.linear_model import LogisticRegression, LinearRegression
import pandas as pd
import os
import numpy as np


directory = "logs"
# directory = "/home/doma945/repos/prooving-gobang/logs"

train_files=[
    "disproof_4x5.csv",
    "disproof_4x6.csv",
    "disproof_4x7.csv",
    "disproof_4x8.csv",
    "disproof_4x9.csv",
    "proof_4x5.csv",
    "proof_4x6.csv",
    "proof_4x7.csv",
    "proof_4x8.csv",
    "proof_4x9.csv",
]

test_files=[
    "disproof_4x10.csv",
#    "disproof_4x11.csv",
    "proof_4x10.csv",
    "proof_4x11.csv",
]

dfs=[]
for f in train_files:
    filename = os.path.join(directory, f)
    print("File: ", filename),
    df = pd.read_csv(filename, sep=' ', index_col=False)
    dfs.append(df)    
train_data = pd.concat(dfs)

dfs=[]
for f in test_files:
    filename = os.path.join(directory, f)
    print("File: ", filename),
    df = pd.read_csv(filename, sep=' ', index_col=False)
    dfs.append(df)    
test_data = pd.concat(dfs)

print("Train Rows: ", len(train_data.index))
print("Test  Rows: ", len(test_data.index))

# white black current_player pn dn empty_cells potential node_count l0 l1 l2 l3 l4 l5 l6 l7
train_input = train_data.iloc[:,[2,5,6,8,9,10,11,12,13,14,15]]
test_input = test_data.iloc[:,[2,5,6,8,9,10,11,12,13,14,15]]
# input_features = data.iloc[:,[2,5,8,9,10,11,12,13,14,15]]

train_winner = (train_data.iloc[:,3] == 0) * 1.0
test_winner = (test_data.iloc[:,3] == 0) * 1.0
print("White wins train:{}%".format(np.mean(train_winner)))
print("White wins test:{}%".format(np.mean(test_winner)))

LR = LogisticRegression(random_state=0, solver='liblinear', multi_class='ovr', class_weight="balanced").fit(train_input, train_winner)

mask = test_winner == 1
i1 = test_input.loc[mask]
w1 = test_winner.loc[mask]
print("Coefficients:\n{}\n".format(LR.coef_))
print("Maker accuracy: ", round(LR.score(i1, w1), 4))
i2 = test_input.loc[- mask]
w2 = test_winner.loc[-mask]
print("Breaker ccuracy: ", round(LR.score(i2, w2), 4))

train_node_count = train_data.iloc[:,7]
test_node_count = test_data.iloc[:,7]

LinReg = LinearRegression().fit(train_input, train_node_count)
print("Node count regression coefficients:\n{}\n".format(LinReg.coef_))
print("Node count regression score: ", round(LinReg.score(test_input, test_node_count), 4))
