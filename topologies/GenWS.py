#generate a watts strogatz graph (into a CSV file for omnetpp template)
import networkx as nx
import os

path = r".\topologies\ws_CSV"
os.chdir(path)

n = 10 #Number of nodes
p = 1 #The probability of rewiring each edge
graph_num = 0 #number of the csv file 
k = 2 #number of adj nodes

network = nx.watts_strogatz_graph(n, k, p)
with open('watts_strogatz' + str(graph_num) + '.csv', 'w+') as csvfile:
    for (u, v, c) in network.edges.data() :
        csvfile.write('Node' + str(u) + ',Node' + str(v) + '\n')