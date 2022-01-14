#generate a watts strogatz graph (into a CSV file)
import matplotlib.pyplot as plt
import networkx as nx
import numpy as np
import os

path = r".\topologies\ws_CSV"
os.chdir(path)

n = 200 #Number of nodes
p = 0.05 #The probability of rewiring each edge
k = 10 #number of adj nodes
rep = 1 #number of files (always > 0)
save = True #to save the output as a SVG file

for graph_num in range(0,rep):
    top = dict()
    network = nx.connected_watts_strogatz_graph(n, k, p)
    
    for(u,v,c) in network.edges.data():
        if u in top:
            top[u].append(v)
        else:
            top[u] = []
            top[u].append(v)

    all_idx = np.arange(0,n,1) 

    for idx in all_idx:
        if not idx in top:
            top[idx] = []
            top[idx].append(-1)

    top = dict(sorted(top.items()))

    with open('watts_strogatz' + str(graph_num) + '.csv', 'w+') as csvfile:
        for site,neibs in top.items():
            csvfile.write(str(site) + ',')
            for neib in neibs:
                if neib == neibs[-1]:
                    csvfile.write(str(neib) + '\n')
                else:
                    csvfile.write(str(neib) + ',')
    
    if(save):
        fig = plt.figure(figsize=(40, 40)) 
        nx.draw(network)
        fig.savefig("ws" + str(graph_num) + ".svg")