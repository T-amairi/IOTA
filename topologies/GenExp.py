#generate an expander graph (into a CSV file for omnetpp template)
import networkx as nx
import os

path = r".\Expander_Watts_Strogatz\exp_CSV"
os.chdir(path)

n = 10 #Number of nodes
graph_num = 0 #number of the csv file 

network = nx.chordal_cycle_graph(n)
with open('expander' + str(graph_num) + '.csv', 'w+') as csvfile:
    edge=[]
    for(u, v, c) in network.edges.data() :
        if(u,v) not in edge and u!=v:
            edge.append((u,v))
    for (u, v) in edge :
        csvfile.write('Node' + str(u) + ',Node' + str(v) + '\n')
