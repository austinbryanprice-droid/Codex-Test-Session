"""Original wolf head: smooth SDF union/subtraction, meshed with marching cubes."""
import numpy as np
from skimage.measure import marching_cubes
from pathlib import Path
root=Path(__file__).resolve().parents[1];step=.018
x=np.arange(-.55,.56,step);y=np.arange(-1.,.51,step);z=np.arange(2.40,3.81,step)
X,Y,Z=np.meshgrid(x,y,z,indexing='ij')
def ell(c,r):return (np.sqrt(((X-c[0])/r[0])**2+((Y-c[1])/r[1])**2+((Z-c[2])/r[2])**2)-1)*min(r)
def union(a,b,k=.08):
 h=np.clip(.5+.5*(b-a)/k,0,1);return b*(1-h)+a*h-k*h*(1-h)
f=ell((0,.035,3.2),(.315,.34,.35))
for c,r in [((0,-.38,3.3),(.22,.44,.13)),((-.24,-.18,3.13),(.165,.22,.21)),((.24,-.18,3.13),(.165,.22,.21)),((0,-.3,2.8),(.225,.40,.082))]:f=union(f,ell(c,r))
f=union(f,ell((0,.19,2.82),(.34,.30,.43)))
f=np.maximum(f,-ell((0,-.48,3.02),(.245,.52,.185)))
for s in [-1,1]:f=np.maximum(f,-ell((s*.25,-.08,3.36),(.068,.065,.052)))
v,faces,_,_=marching_cubes(f,0,spacing=(step,step,step),gradient_direction='descent');v+=np.array([x[0],y[0],z[0]])
with (root/'art-source/head.obj').open('w') as out:
 for p in v:out.write('v %f %f %f\n'%tuple(p))
 for q in faces:out.write('f %d %d %d\n'%tuple(q+1))
print('SDF head:',len(v),'vertices')
