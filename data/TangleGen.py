#!/usr/bin/env python3

#import a Tangle CSV file to a SVG file  
from graphviz import Digraph
import os
import csv

os.chdir(r"./data/tracking")

Tangle = list()
numberTips = 0
listTips = list()
with open("Tangle.csv", 'r',) as file:
    reader = csv.reader(file, delimiter = ';')
    for row in reader:
        temp = row[1].split(",")
        if temp[0] == '':
            Tangle.append((row[0],list()))
            listTips.append(row[0])
            numberTips += 1
        else:
            Tangle.append((row[0],temp))

listNotLegit = list()
if os.path.exists("NotLegit.csv"):
    with open("NotLegit.csv", 'r',) as file:
        reader = csv.reader(file, delimiter = ';')
        for row in reader:
            listNotLegit.append(row[1])
        listNotLegit = list(set(listNotLegit))

listLegit = list()
if os.path.exists("Legit.csv"):
    with open("Legit.csv", 'r',) as file:
        reader = csv.reader(file, delimiter = ';')
        for row in reader:
            listLegit.append(row[1])
        listLegit = list(set(listLegit))
            
dictApp = {tx[0]:list() for tx in Tangle}
Tangle.reverse()
for tx in Tangle:
    if len(tx[1]) != 0: 
        for neib in tx[1]:
            if neib in listTips:
                dictApp[tx[0]].append(neib)
            else:    
                dictApp[tx[0]] = list(set(dictApp[tx[0]] + dictApp[neib]))

os.chdir(r"../image")
colors = ['green','orange','red','blue','purple']
conf = 0.5

g = Digraph(comment='Tangle',format='svg',node_attr={'shape': 'box','style': 'filled'})
g.attr(rankdir='LR')
g.attr(label='Tangle',labelloc='t')

for tx in Tangle:
    if len(tx[1]) == 0:
        if tx[0] in listNotLegit:
            g.node(tx[0],color=colors[3]+":"+colors[2])
        elif tx[0] in listLegit:
            g.node(tx[0],color=colors[4]+":"+colors[2])
        else:
            g.node(tx[0],color=colors[2])
    else:
        if len(dictApp[tx[0]]) >= int(numberTips*conf):
            if tx[0] in listNotLegit:
                g.node(tx[0],color=colors[3]+":"+colors[0])
            elif tx[0] in listLegit:
                g.node(tx[0],color=colors[4]+":"+colors[0])
            else:
                g.node(tx[0],color=colors[0])        
        else:
            if tx[0] in listNotLegit:
                g.node(tx[0],color=colors[3]+":"+colors[1])
            elif tx[0] in listLegit:
                g.node(tx[0],color=colors[4]+":"+colors[1])
            else:
                g.node(tx[0],color=colors[1])        
            
for tx in Tangle:
        for neib in tx[1]:
            g.edge(tx[0],neib,dir="back")

g.render('Tangle',format="svg")
os.remove('Tangle')