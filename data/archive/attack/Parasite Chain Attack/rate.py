import os
import csv
import numpy as np
import matplotlib.pyplot as plt 

paths = [r".\data\archive\attack\Parasite Chain Attack\IOTA",r"..\G-IOTA",r"..\E-IOTA"]

res = []

for path in paths:
    os.chdir(path)
    temp = dict()
    with open('DiffTxChain.txt', 'r',) as file:
        reader = csv.reader(file, delimiter = ';')
        for row in reader:
            if row[0] != "null":
                if not row[0] in temp:
                    temp[row[0]] = [0,0]
                if int(row[2]) > 0:
                    temp[row[0]][0] += 1
                else:
                    temp[row[0]][1] += 1
    res.append(temp)

tsa = ["IOTA","G-IOTA","E-IOTA"]

L = []
LG = []
LE = []

for i in range(0,3):
    print("*" + tsa[i] + ":")
    TSA = res[i]
    if not TSA:
        print("No result")
    else:
        for k,v in TSA.items():
            sr = float(v[0])
            fr = float(v[1])
            print("PropComputingPower value: {}, Success rate: {:.2f}%, Fail rate: {:.2f}%".format(k,100.0*sr/(sr+fr),100.0*fr/(sr+fr)))
            if k != "0.5":
                if i == 0:
                    L.append(100.0*fr/(sr+fr))
                elif i == 1:
                    LG.append(100.0*fr/(sr+fr))
                else:
                    LE.append(100.0*fr/(sr+fr))
    print('\n')

os.chdir(r"..\.")

x = np.arange(0.01,0.11,0.01)
L.reverse()
LG.reverse()
LE.reverse()

plt.figure(figsize=(8,5))

plt.plot(x,L,label="IOTA")
plt.plot(x,LG,label="G-IOTA")
plt.plot(x,LE,label="E-OTA")

plt.xticks(x)
plt.gca().invert_xaxis()

plt.xlabel(r"PropComputingPower value")
plt.ylabel(r"Resistance in %")
plt.legend()

plt.savefig('ResistancePCA.png',bbox_inches='tight')