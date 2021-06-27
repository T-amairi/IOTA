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
    #os.remove(log)
    
t = float(sum(t)/len(t))
print("Average time of execution:",t,"seconds")

NbTips = 0
TipsFile = open(glob.glob("Number*.txt")[0],'r')

for line in TipsFile.readlines():
    NbTips += int(line)

NbTips = NbTips/Nbrun
print("Average number of tips:",NbTips)

TipsFile.close()
#os.remove(glob.glob("Number*.txt")[0])