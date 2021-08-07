import matplotlib.pyplot as plt
import collections
import numpy as np
import os

path = r".\data\image"
os.chdir(path)

TSA = ["IOTA","G-IOTA","E-IOTA"]
n = 1000

tips = [658.23,473.43,209.64]
t = [133.77,222.35,72.93]
app = [n - i for i in tips]

def autolabel(rects):
    for p in rects:
        height = p.get_height()
        ax.annotate('{}'.format(int(height)),xy=(p.get_x() + p.get_width() / 2, height),xytext=(0, 3),textcoords="offset points",ha='center', va='bottom')

x = np.arange(len(TSA)) 
width = 0.2  
fig, ax = plt.subplots(figsize=(12,8))
rects1 = ax.bar(x - width, tips, width, label='Number of tips')
rects2 = ax.bar(x, app, width, label='Number of approuved transactions')
rects3 = ax.bar(x + width, t, width, label='Execution time in sec')

ax.set_xlabel('Tip Selection Algorithm')
ax.set_xticks(x)
ax.set_xticklabels(TSA)
ax.legend()

fig.tight_layout()
autolabel(rects1)
autolabel(rects2)
autolabel(rects3)

#fig.savefig('TSA.png',bbox_inches='tight')

##########################################

y = [46.0,74.5,88.2,98.7,147.4]
x = np.arange(0.02,0.12,0.02)

plt.figure(figsize=(8,5))
plt.scatter(x,y)
plt.xlabel(r"$\alpha$ value")
plt.ylabel(r"Average tips")

for i, txt in enumerate(y):
    plt.annotate(txt, (x[i] - 0.0025, y[i] + 2.5))

plt.savefig('alphaIOTA.png',bbox_inches='tight')