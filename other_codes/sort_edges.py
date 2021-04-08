ulyses=[[1,3],[1,16],[2,3],[2,4],[4,8],[5,6],[5,11],[6,15],[7,10],[7,12],[8,14],[9,10],[9,11],[12,13],[13,16],[14,15]]
edges=ulyses
print(edges)
N=16
ans=[]


def fun(i,sl,sr):
    if i>=len(edges):return True
    L,R=edges[i]

    #Stay like this
    stay=False
    if not sl[L] and not sr[R]:
        newSL=sl[:]
        newSR=sr[:]
        newSL[L]=True
        newSR[R]=True
        stay=fun(i+1,newSL,newSR)
        if stay:
            ans.append([L,R])
            print(" "*i,L,R)
            return True

    #oposite
    opposite=False
    L,R=R,L
    if not sl[L] and not sr[R]:
        newSL=sl[:]
        newSR=sr[:]
        newSL[L]=True
        newSR[R]=True
        opposite=fun(i+1,newSL,newSR)
        if opposite:
            ans.append([L,R])
            print(" "*i,L,R)
            return True

    return stay or opposite


fun(0,[False]*(N+1),[False]*(N+1))

ans.sort()
print(ans)



#TODO test cases: generate random edges