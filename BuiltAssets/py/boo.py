import numpy as np

#out float
meow = 1.0

#out list(float)
bark = []

#in float
def Execute(f):
    global bark
    for i in range(0, 1000000):
        bark = (np.power(np.random.rand(4), f)).tolist()
    print("done!")