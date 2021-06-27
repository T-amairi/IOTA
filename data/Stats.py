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
    
t = float(sum(t)/len(t))
print("Average time of execution:",t,"seconds")

path = r".\data\tracking"
os.chdir(path)

NbTips = 0
TipsFile = glob.glob("Number*.txt")
NbModule = 0

for file in TipsFile:
    NbTips = 0
    f = open(file,'r')
    for line in f.readlines():
        NbTips += int(line)

    NbTips = NbTips/Nbrun
    print("Average number of tips for NodeModule[" + str(NbModule) + "]:",NbTips)
    NbModule += 1
    f.close()
