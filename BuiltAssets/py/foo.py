import sys

#out float
rdf = 1.0

#out list(pair(float,float))
msd = []

#in float int
def Execute(f, i):
    global rdf, msd
    rdf = 2.5 + f
    msd = [(1.2 + i, 3.4 - i)]
    print(rdf)