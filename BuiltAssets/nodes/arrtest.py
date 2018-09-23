import math
import numpy as np

#out list(1d)
res = np.zeros(1)

a = 0

#in double int
def Execute(f, i):
    global res, a
    res = np.sin(a + 0.1 * np.arange(100))
    a = a + 1