import sklearn as sk
from sklearn import preprocessing
from sklearn.linear_model import LogisticRegression, LinearRegression
import pandas as pd
import os
import numpy as np


# directory = "logs"
directory = "/home/doma945/repos/prooving-gobang/build/data"

train_files=[
    "disproof/4x7.csv",
    "disproof/4x8.csv",
    "disproof/4x9.csv",
    "disproof/4x10.csv",
    "proof/4x7.csv",
    "proof/4x8.csv",
    "proof/4x9.csv",
    "proof/4x10.csv",
]

test_files=[
    "disproof/4x10.csv",
#    "disproof/4x11.csv",
    "proof/4x11.csv",
#    "proof/4x12.csv",
]

def read_files(files):
    dfs=[]
    for f in files:
        filename = os.path.join(directory, f)
        print("File: ", filename),
        df = pd.read_csv(filename, sep=' ', header=None, index_col=False, skiprows=1, names = ["white","black","current_player","pn","dn","empty_cells","potential","node_count","l0","l1","l2","l3","l4","l5","l6","l7"], nrows=30000000)
        dfs.append(df)    
    data = pd.concat(dfs)
    return data

train_data = read_files(train_files)
test_data = read_files(test_files)


print("Train Rows: ", len(train_data.index))
train_data = train_data.drop_duplicates()
print("Kept : {} rows".format(len(train_data.index)))
print("Test  Rows: ", len(test_data.index))

# white black current_player pn dn empty_cells potential node_count 
# features = ["current_player", "empty_cells", "potential", "l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7"]
features = ["current_player", "empty_cells", "potential"]
train_input = train_data[features]
test_input = test_data[features]


train_winner = (train_data.iloc[:,3] == 0) * 1.0
test_winner = (test_data.iloc[:,3] == 0) * 1.0
print("White wins train:{}%".format(100 * np.mean(train_winner)))
print("White wins test:{}%".format(100 * np.mean(test_winner)))

LR = LogisticRegression(random_state=0, solver='liblinear', multi_class='ovr', class_weight="balanced").fit(train_input, train_winner)

mask = test_winner == 1
i1 = test_input.loc[mask]
w1 = test_winner.loc[mask]
print("Coefficients:\n{}".format(LR.coef_))
print("Intercept:{}".format(LR.intercept_))
print("Maker accuracy: ", round(LR.score(i1, w1), 4))
i2 = test_input.loc[- mask]
w2 = test_winner.loc[-mask]
print("Breaker ccuracy: ", round(LR.score(i2, w2), 4))

train_node_count = train_data.iloc[:,7]
test_node_count = test_data.iloc[:,7]

LinReg = LinearRegression().fit(train_input, train_node_count)
print("Node count regression coefficients:\n{}\n".format(LinReg.coef_))
print("Node count regression intercept:{}".format(LinReg.intercept_))
print("Node count regression score: ", round(LinReg.score(test_input, test_node_count), 4))
