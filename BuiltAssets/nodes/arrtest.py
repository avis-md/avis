import math
import numpy as np

#out list(1f)
res = np.zeros(1)

a = 0

#in float int
def Execute(f, i):
    global res, a
    res = np.sin(a + 0.1 * np.arange(100,dtype=np.float32))
    a = a + 1