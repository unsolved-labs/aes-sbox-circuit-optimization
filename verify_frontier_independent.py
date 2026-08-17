#!/usr/bin/env python3
"""Independent Python replay of the R002 local outer-span exclusion certificate."""
from pathlib import Path
from collections import Counter

HERE = Path(__file__).resolve().parent
DATA = HERE / "frontier_certificate.txt"


def load_data():
    section = None
    gens, target = [], []
    for raw in DATA.read_text().splitlines():
        line = raw.strip()
        if not line or line.startswith('#'):
            continue
        if line == '[GENERATORS]': section = 'g'; continue
        if line == '[TARGET]': section = 't'; continue
        if section == 'g':
            a,b = line.split()
            gens.append((int(a,16), int(b,16)))
        elif section == 't':
            target.append(int(line,16))
        else:
            raise AssertionError(f"data outside section: {line}")
    assert len(gens) == 24 and len(target) == 12
    assert all(0 < a < 1<<12 and 0 < b < 1<<12 for a,b in gens)
    assert all(0 <= q < 1<<24 for q in target)
    return gens, target

PAIR = [[-1]*12 for _ in range(12)]
k=0
for i in range(12):
    for j in range(i+1,12):
        PAIR[i][j]=k; k+=1
assert k == 66

WEDGE_BIT = [[0]*12 for _ in range(1<<12)]
for u in range(1<<12):
    for j in range(12):
        w=0
        for i in range(12):
            if i != j and ((u>>i)&1):
                lo,hi=(i,j) if i<j else (j,i)
                w ^= 1 << PAIR[lo][hi]
        WEDGE_BIT[u][j]=w

def wedge(u,v):
    w=0
    while v:
        bit=v & -v
        j=bit.bit_length()-1
        v ^= bit
        w ^= WEDGE_BIT[u][j]
    return w

def tagged_basis(vectors):
    basis=[0]*66
    tags=[0]*66
    rank=0
    for idx,x in enumerate(vectors):
        tag=1<<idx
        while x:
            p=x.bit_length()-1
            if basis[p]:
                x ^= basis[p]; tag ^= tags[p]
            else:
                basis[p]=x; tags[p]=tag; rank+=1; break
        else:
            raise AssertionError("outer generators are not independent")
    return basis,tags,rank

def coordinates(x,basis,tags):
    tag=0
    while x:
        p=x.bit_length()-1
        if not basis[p]: return None
        x ^= basis[p]; tag ^= tags[p]
    return tag

def rank24(xs):
    basis=[0]*24; r=0
    for x in xs:
        while x:
            p=x.bit_length()-1
            if basis[p]: x ^= basis[p]
            else: basis[p]=x; r+=1; break
    return r

def nullspace_basis(rows):
    rr=rows[:]
    piv=[]; r=0
    for col in range(23,-1,-1):
        sel=next((i for i in range(r,len(rr)) if (rr[i]>>col)&1),None)
        if sel is None: continue
        rr[r],rr[sel]=rr[sel],rr[r]
        for i in range(len(rr)):
            if i!=r and ((rr[i]>>col)&1): rr[i] ^= rr[r]
        piv.append(col); r+=1
    assert r == len(rows) == 12
    is_piv=set(piv)
    nb=[]
    for f in range(24):
        if f in is_piv: continue
        x=1<<f
        for i,row in enumerate(rr):
            if (row>>f)&1: x |= 1<<piv[i]
        nb.append(x)
    assert len(nb)==12
    return nb

def main():
    gens,target=load_data()
    pvec=[wedge(a,b) for a,b in gens]
    basis,tags,prank=tagged_basis(pvec)
    assert prank==24
    decomposable=[]; total=0
    for u in range(1,1<<12):
        for v in range(u+1,1<<12):
            z=u^v
            if not (v<z): continue
            total += 1
            co=coordinates(wedge(u,v),basis,tags)
            if co is not None: decomposable.append(co)
    decomposable=sorted(set(decomposable))
    nb=nullspace_basis(target)
    hist=Counter(); best=0
    for mask in range(1,1<<12):
        functional=0
        for i,b in enumerate(nb):
            if (mask>>i)&1: functional ^= b
        inside=[x for x in decomposable if ((x & functional).bit_count() & 1)==0]
        z=rank24(inside); hist[z]+=1; best=max(best,z)
    assert total==2794155, total
    assert len(decomposable)==27, len(decomposable)
    assert sum(hist.values())==4095
    assert best==21, best
    print(f"canonical decomposable bivectors: {total}")
    print(f"published outer-span rank: {prank}")
    print(f"decomposable points in published span: {len(decomposable)}")
    print("target rank: 12")
    print("target-containing hyperplanes checked: 4095")
    print("decomposable-span rank histogram:" + ''.join(f" {k}:{hist[k]}" for k in sorted(hist)))
    print(f"maximum decomposable span rank: {best}")
    print("PASS independent Python frontier replay")

if __name__ == '__main__': main()
