#!/usr/bin/env python3

import re
import glob
import os

path = r".\data\log"
os.chdir(path)

t = []
logs = glob.glob("log*.txt")
Nbrun = len(logs)

for log in logs:
    l = open(log,'r')
    m = re.findall("(?<=Elapsed: )(.*?)(?=s)",l.read())
    if float(m[-1]) > 0:
        t.append(float(m[-1]))
    l.close()

if t:
    t = float(sum(t)/len(t))
    print("Average time of execution:",t,"seconds")

path = r"..\tracking"
os.chdir(path)

NbTips = 0
Nbrun = 0
with open("TipsNumber.csv",'r') as f:
    for line in f.readlines():
        NbTips += int(line)
        Nbrun += 1

    NbTips = NbTips/Nbrun
    print("Average number of tips:",NbTips)
