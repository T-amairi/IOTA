#modules
from graphviz import Digraph
import os
import csv
import glob

#paths
path = r".\\data\tracking"
os.chdir(path)
#files
tangles = glob.glob("TrackerTangle*")
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

#get a dict of all transaction approving directly or indirectly double spend transaction
chains = glob.glob("Chain*")
dictChain = dict()
for chain in chains:
    with open(chain, 'r',) as file:
        idx = 0
        for line in file:
            if(line[0] != '[' and line[0] != '-'):
                idx = int(line)
                dictChain[idx] = set()
            else:
                dictChain[idx].add(line.strip())

#get a dict of all transaction approving directly or indirectly not double spend transaction
Legitchains = glob.glob("LegitChain*")
dictLegitChain = dict()
for Legitchain in Legitchains:
    with open(Legitchain, 'r',) as file:
        idx = 0
        for line in file:
            if(line[0] != '[' and line[0] != '-'):
                idx = int(line)
                dictLegitChain[idx] = set()
            else:
                dictLegitChain[idx].add(line.strip())

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
colors = ['green','orange','red','blue','purple']
#confidence 
conf = 0.5
#number of Tangle to generate 
NbTangle = 1
assert NbTangle <= len(NodeModules), 'NbTangle should be less or equal to the number of nodes'

path = r"..\\image"
os.chdir(path)

#generating the Tangle 
for nodeID, sites in NodeModules.items():
    if(nodeID == NbTangle):
        break

    numberTips,listTips = Tips(sites)
    dictApp = TipsApp(sites,listTips)
    setChain = set()
    setLegitChain = set()

    if nodeID in dictChain.keys():
        setChain = dictChain[nodeID]
    if nodeID in dictLegitChain.keys():
        setLegitChain = dictLegitChain[nodeID]
    
    g = Digraph(comment='Tangle NodeModule[' + str(nodeID) + ']',format='png',node_attr={'shape': 'box','style': 'filled'})
    g.attr(rankdir='LR')
    g.attr(label=r'Tangle NodeModule[' + str(nodeID) + ']',labelloc='t')

    for node in sites:
        if node[1][0] == '':
            if node[0] in setChain:
                g.node(node[0],color=colors[3]+":"+colors[2])
            elif node[0] in setLegitChain:
                g.node(node[0],color=colors[4]+":"+colors[2])
            else:
                g.node(node[0],color=colors[2])
        else:
            if len(dictApp[node[0]]) >= int(numberTips*conf):
                if node[0] in setChain:
                    g.node(node[0],color=colors[3]+":"+colors[0])
                elif node[0] in setLegitChain:
                    g.node(node[0],color=colors[4]+":"+colors[0])
                else:
                    g.node(node[0],color=colors[0])        
            else:
                if node[0] in setChain:
                    g.node(node[0],color=colors[3]+":"+colors[1])
                elif node[0] in setLegitChain:
                    g.node(node[0],color=colors[4]+":"+colors[1])
                else:
                    g.node(node[0],color=colors[1])        
                
    for node in sites:
            for neib in node[1]:
                if neib != '':
                    g.edge(node[0],neib,dir="back")

    g.render('TangleNodeModule[' + str(nodeID) + ']',format="svg")
    os.remove('TangleNodeModule[' + str(nodeID) + ']') 