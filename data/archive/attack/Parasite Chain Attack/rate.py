import os
import csv

paths = [r".\data\archive\attack\Parasite Chain Attack\IOTA",r"..\G-IOTA",r"..\E-IOTA"]

res = []

for path in paths:
    os.chdir(path)
    temp = dict()
    with open('DiffTxChain.txt', 'r',) as file:
        reader = csv.reader(file, delimiter = ';')
        for row in reader:
            if not row[0] in temp:
                temp[row[0]] = [0,0]
            if int(row[2]) > 0:
                temp[row[0]][0] += 1
            else:
                temp[row[0]][1] += 1
    res.append(temp)

tsa = ["IOTA","G-IOTA","E-IOTA"]

for i in range(0,3):
    print("*" + tsa[i] + ":")
    TSA = res[i]
    if not TSA:
        print("No result")
    else:
        for k,v in TSA.items():
            sr = float(v[0])
            fr = float(v[1])
            if sr == 0.0:
                print("PropComputingPower value: {}, Success rate: 0.0%, Fail rate: 100.0%".format(k))
            elif fr == 0.0:
                print("PropComputingPower value: {}, Success rate: 100.0%, Fail rate: 0.0%".format(k))
            else:
                print("PropComputingPower value: {}, Success rate: {:.2f}%, Fail rate: {:.2f}%".format(k,100.0*sr/(sr+fr),100.0*fr/(sr+fr)))
    print('\n')
        


            












