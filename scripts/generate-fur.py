"""Original calm directional fur detail, no photographic or external inputs."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFilter
import numpy as np,random
root=Path(__file__).resolve().parents[1]/'public/assets';w=h=1024;r=random.Random(2719)
image=Image.new('L',(w,h),220);draw=ImageDraw.Draw(image)
for i in range(2100):
 x=r.randrange(w);y=r.randrange(h);length=r.randrange(18,110);bend=r.uniform(-12,12);shade=r.randrange(120,235)
 points=[(int(x+bend*((j/7)**2)),y+j*length//7) for j in range(8)]
 for offset in [-h,0,h]:draw.line([(px,py+offset) for px,py in points],fill=shade,width=r.choice([2,2,3]))
image=image.filter(ImageFilter.GaussianBlur(.45));a=np.asarray(image,dtype=float)/255
ny,nx=np.gradient(a);normal=np.stack((-nx*9,ny*9,np.ones_like(a)),axis=-1);normal/=np.linalg.norm(normal,axis=-1,keepdims=True)
Image.fromarray((np.clip(normal*.5+.5,0,1)*255).astype('uint8'),'RGB').save(root/'fur-normal.png',optimize=True)
Image.merge('RGB',(image,image,image)).save(root/'fur-detail.png',optimize=True)
print('Original directional fur detail generated')

# A tapered, fibrous cutout for broad geometric clumps, not thin spike meshes.
mask=Image.new('L',(256,512),0);d=ImageDraw.Draw(mask)
d.polygon([(32,511),(224,511),(214,360),(190,190),(128,8),(66,190),(42,360)],fill=255)
for i in range(100):
 x=r.uniform(15,241);tip=r.uniform(100,156);height=r.randrange(30,160)
 points=[(x,511),(x*.75+tip*.25,350),(x*.4+tip*.6,180),(tip,height)]
 d.line(points,fill=r.randrange(180,256),width=r.choice([1,2,3]))
mask=mask.filter(ImageFilter.GaussianBlur(.55));mask.save(root/'fur-cutout.png',optimize=True)
