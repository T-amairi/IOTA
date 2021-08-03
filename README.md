# IOTA Simulation using OMNeT++

This git is a decentralized simulation of [IOTA](https://www.iota.org/) which is a crypto-currency  using [OMNeT++](https://omnetpp.org/). 

The source files for the simulation are **NodeModule.h** which is the header containing the structures and classes of the **NodeModule.cc** file. This one contains the code executed by the simulation. Finally **iota.ini & iota.ned** are the files to configure the simulation (topology used, number of nodes, communication delay between nodes etc...). It is also possible to set **random** delays between nodes in the iota.ned file. 

This simulation can use several topologies: 
- **Complete graph**
- **2D grid**
- **Torus**
- **Expander**
- **Watts-Strogatz**

As OMNeT++ does not implement natively the last two, you have to use the **topologies** folder which contains two **python scripts** allowing each to generate an Expander or Watts-Strogatz network. The simulation will then read the .CSV created by the scripts to build the network. It is also necessary to install the **networkx and numpy modules.**

The **data** folder contains the tracking files allowing to : 
- generate an image of the Tangle via the script **TangleGen.py** (you need to install the modules : **graphviz, csv, glob and glob**). Moreover, the figure will be saved in the **image** folder.
- know the **number of tips** at the end of each simulation and the simulation **execution time**. The script **Stats.py** allows to make an empirical average of these results.

These different metrics will be in .txt format in two different folders: 
- **log** for the execution time which will be given directly by OMNeT++.
- **tracking** which will contain all the files created by the simulation directly (i.e. by the code in C++). 

Finally, the **Results.py** script allows to create a histogram in the **image** folder of these different results. 

The simulation implements three different **TSA** (Tips Selection Algorithm): 
- **IOTA**: www.descryptions.com/Iota.pdf
- **G-IOTA**: https://ieeexplore.ieee.org/document/8845163
- **E-IOTA**: https://ieeexplore.ieee.org/document/9223294

And two types of **attacks** explained in the IOTA [paper](www.descryptions.com/Iota.pdf): 
- **Splitting Attack**
- **Parasite Chain Attack**

I did this project while I was an intern at [LIP6](https://www.lip6.fr/). 

I want to thank **Richard Gardner** (@richardg93) who helped me a lot during the beginning thanks to his [git](https://github.com/richardg93/TangleSim) but also **Gewu Bu** (@GewuBU) who guided me during my internship.   
