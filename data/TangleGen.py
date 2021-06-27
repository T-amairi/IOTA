#modules
from graphviz import Digraph 
from PIL import Image
import os
import csv
import glob

#paths
path = r".\data\Tracking"
os.chdir(path)
#files
tangles = glob.glob("Tracker*")
#dict for sites 
NodeModules = dict()
count = -1

#generate the dict
for tangle in tangles:
    with open(tangle, 'r',) as file:
        reader = csv.reader(file, delimiter = ';')
        count += 1
        NodeModules[count] = []
        for row in reader:
            site_name = row[0]
            site_neib = row[1]
            NodeModules[count].append((site_name,site_neib.split(",")))

#compute the number of tips and return the list of Tips 
def Tips(listSite):
    numberTips = 0
    listTips = []
    for node in listSite:
        if node[1][0] == '':
            listTips.append(node[0])
            numberTips += 1
    return numberTips,listTips

#compute the number of tips that approve each sites 
def TipsApp(listSite,listTips):
    listSite.reverse()
    dictApp = {}
    for node in listSite:
        dictApp[node[0]] = []
    for node in listSite:
        if not node[0] in listTips : 
            for neib in node[1]:
                if neib in listTips:
                    dictApp[node[0]].append(neib)
                else:    
                    dictApp[node[0]] = list(set(dictApp[node[0]] + dictApp[neib]))
    return dictApp

#colors of the node for graphviz 
colors = ['green','orange','red']
#confidence 
conf = 0.5

path = r"..\image"
os.chdir(path)

#generating the Tangle 
for nodeID, sites in NodeModules.items():
    numberTips,listTips = Tips(sites)
    dictApp = TipsApp(sites,listTips)
    g = Digraph(comment='Tangle NodeModule[' + str(nodeID) + ']',format='png',node_attr={'shape': 'box','style': 'filled'})
    g.attr(rankdir='LR')
    g.attr(label=r'Tangle NodeModule[' + str(nodeID) + ']',labelloc='t')

    for node in sites:
        if node[1][0] == '':
            g.node(node[0],color=colors[2])
        else:
            if len(dictApp[node[0]]) >= int(numberTips*conf):
                g.node(node[0],color=colors[0])
            else:
                g.node(node[0],color=colors[1])
                
    for node in sites:
            for neib in node[1]:
                if neib != '':
                    g.edge(node[0],neib,dir="back")

    g.render('TangleNodeModule[' + str(nodeID) + ']')
    os.remove('TangleNodeModule[' + str(nodeID) + ']')