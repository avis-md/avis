import numpy as np


#out list(1d)
res = np.zeros(4000)

#in int
def Execute(ff):
    global res
    P = np.load("/Users/chokopan/t_All.npy")[:,:800]
    #print( P.shape )
    PMIN,PMAX = np.min(P) , np.max(P)
    PWID = PMAX - PMIN
    P = (P - PMIN)/PWID * 1.
    #print ( np.min(P) , np.max(P) )
    
    res[:] = 1.
    f = ff - (904-32*8)
    if (f >= 0 and ff < 4*800):
        res[0:800*4:4] = P[f,:]
        res[1:800*4:4] = P[f,:]
        res[2:800*4:4] = P[f,:]
        res[3:800*4:4] = P[f,:]
    #res[904-32*8:904-32*8+3592,0:800*4:4] = P
    #res[904-32*8:904-32*8+3592,1:800*4:4] = P
    #res[904-32*8:904-32*8+3592,2:800*4:4] = P
    #res[904-32*8:904-32*8+3592,3:800*4:4] = P
    