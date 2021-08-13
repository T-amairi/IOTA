import matplotlib.pyplot as plt
import numpy as np
import os

path = r".\data\archive\alpha"
os.chdir(path)

y = [94.8,96,99.6,99.5,100.2,103]
x = np.arange(0.5,3.5,0.5)

plt.figure(figsize=(8,5))
plt.scatter(x,y)
plt.xlabel(r"$\alpha$ value")
plt.ylabel(r"Average tips")

for i, txt in enumerate(y):
    plt.annotate(txt, (x[i] - 0.05, y[i] + 0.15))

plt.savefig('alphaE-IOTA.png',bbox_inches='tight')