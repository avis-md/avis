import math
import numpy as np

#out list(1d)
res = np.zeros(1)

#in list(1d)
def Execute(arr):
    global res
    res = np.diff(arr)
    print ("Python says hello!")