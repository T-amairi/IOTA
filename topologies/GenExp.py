#generate an expander graph (into a CSV file for omnetpp template)
import networkx as nx
import numpy as np
import os

path = r".\topologies\exp_CSV"
os.chdir(path)

n = 10 #Number of nodes
rep = 1 #number of files (always > 0)

for graph_num in range(0,rep):
    top = dict()
    network = nx.chordal_cycle_graph(n)
    edge = []

    for(u,v,c) in network.edges.data():
        if(u,v) not in edge and u!=v:
            edge.append((u,v))
    for (u,v) in edge :
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

    with open('expander' + str(graph_num) + '.csv', 'w+') as csvfile:
        for site,neibs in top.items():
            csvfile.write(str(site) + ',')
            for neib in neibs:
                if neib == neibs[-1]:
                    csvfile.write(str(neib) + '\n')
                else:
                    csvfile.write(str(neib) + ',')