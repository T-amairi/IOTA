#generate a network with a specific topology (into a CSV file)
import matplotlib.pyplot as plt
import networkx as nx
import os

############################################################################
#                             PARAMETERS                                   #
############################################################################

#### FOR ALL TOPO ####
name = "WattsStrogatz" #name of the wanted topo : FullGraph or Grid or Torus or WattsStrogatz (case sensitive !)
n = 10 #number of nodes

#### 2D GRID & TORUS ####
rows = 2 #number of rows
columns = 5 #number of columns

#### WATTS STROGATZ ####
p = 0.05 #the probability of rewiring each edge
k = 5 #number of adj nodes
save = True #to save the output as a SVG file

############################################################################
#                             FUNCTIONS                                    #
############################################################################

def FullGraph():
    top = {key:list() for key in range(0,n)}

    for i in range(0,n): 
        for j in range(0,n): 
            if i != j:
                top[i].append(j)

    return top

def Grid():
    top = {key:list() for key in range(0,n)}

    for i in range(0,rows): 
        for j in range(0,columns): 
            if i != rows - 1:
                top[i*columns+j].append((i+1)*columns+j)
                top[(i+1)*columns+j].append(i*columns+j)
            if j != columns - 1:
                top[i*columns+j].append(i*columns+j+1)
                top[i*columns+j+1].append(i*columns+j)

    return top

def Torus():
    top = {key:list() for key in range(0,n)}

    for i in range(0,rows): 
        for j in range(0,columns): 
            if i != rows - 1:
                top[i*columns+j].append(((i+1)%rows)*columns+j)
                top[((i+1)%rows)*columns+j].append(i*columns+j)
            if j != columns - 1:
                top[i*columns+j].append((i*columns+(j+1))%columns)
                top[(i*columns+(j+1))%columns].append(i*columns+j)

    return top

def toDict(network):
    top = {key:list() for key in range(0,n)}

    for (u,v,c) in network.edges.data():
        top[u].append(v)
        top[v].append(u)

    return dict(sorted(top.items()))

def toSave(network):
    fig = plt.figure(figsize=(40,40)) 
    nx.draw(network)
    os.chdir(r"./topologies/SVG")
    fig.savefig(name + ".svg")
    os.chdir(r"../..")

def getTopo():
    if(name == "FullGraph"):
        return FullGraph()
    elif(name == "Grid"):
        return Grid()
    elif(name == "Torus"):
        return Torus()
    elif(name == "WattsStrogatz"):
        network = nx.watts_strogatz_graph(n, k, p)
        if save:
            toSave(network)
        return toDict(network)
    else:
        raise ValueError("Topologie name not recognized")

top = getTopo()
os.chdir(r"./topologies/CSV")

with open(name + '.csv', 'w+') as csvfile:
    for site,neibs in top.items():
        csvfile.write(str(site) + ',')
        for neib in neibs:
            if neib == neibs[-1]:
                csvfile.write(str(neib) + '\n')
            else:
                csvfile.write(str(neib) + ',')    