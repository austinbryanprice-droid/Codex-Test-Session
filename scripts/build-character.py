"""Original procedural character. Blender 4.3; MakeHuman CC0 body input.
No hand-sculpted models, animation packs or proprietary content.
"""
import bpy,math,random,json
from pathlib import Path
from mathutils import Vector, Quaternion
from mathutils.bvhtree import BVHTree
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'public/assets';rng=random.Random(45)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
parts=[];binds={}
def mat(name,color,rough=.55,metal=0):
 furry=any(word in name for word in ['hide','fur','mane','roots'])
 if furry:color=tuple(c*.4 for c in color)
 m=bpy.data.materials.new(name);m.diffuse_color=(*color,1);m.use_nodes=True;p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(*color,1);p.inputs['Roughness'].default_value=rough;p.inputs['Metallic'].default_value=metal
 if furry:
  nodes=m.node_tree.nodes;links=m.node_tree.links
  coord=nodes.new('ShaderNodeTexCoord');mapping=nodes.new('ShaderNodeMapping');mapping.inputs['Scale'].default_value=(2.5,2.5,2.5);links.new(coord.outputs['UV'],mapping.inputs['Vector'])
  detail=nodes.new('ShaderNodeTexImage');detail.image=bpy.data.images.load(str(OUT/'fur-detail.png'),check_existing=True);detail.image.colorspace_settings.name='Non-Color';links.new(mapping.outputs['Vector'],detail.inputs['Vector'])
  multiply=nodes.new('ShaderNodeMixRGB');multiply.blend_type='MULTIPLY';multiply.inputs[0].default_value=1;multiply.inputs[1].default_value=(*color,1);links.new(detail.outputs['Color'],multiply.inputs[2])
  normal=nodes.new('ShaderNodeTexImage');normal.image=bpy.data.images.load(str(OUT/'fur-normal.png'),check_existing=True);normal.image.colorspace_settings.name='Non-Color';links.new(mapping.outputs['Vector'],normal.inputs['Vector'])
  bump=nodes.new('ShaderNodeNormalMap');bump.inputs['Strength'].default_value=.38;links.new(normal.outputs['Color'],bump.inputs['Color']);links.new(bump.outputs['Normal'],p.inputs['Normal'])
 return m

skin=mat('01 • midnight blue hide',(.035,.095,.14),.76);fur=mat('02 • slate blue fur',(.065,.15,.21),.77);tips=mat('03 • silver blue mane',(.21,.34,.43),.75);dark=mat('04 • fur roots and socket shadow',(.012,.028,.043));mouth=mat('05 • mouth and tinted lips',(.022,.004,.008),.7);ivory=mat('06 • warm ivory teeth and claws',(.8,.73,.52),.34);gold=mat('07 • worn ancient bronze',(.48,.25,.07),.28,.85);leather=mat('08 • charcoal woven loincloth',(.035,.038,.042),.95);eye=mat('09 • amber iris',(.96,.37,.035),.18);black=mat('10 • wet nose and pupils',(.008,.012,.015),.22);steel=mat('11 • blued armor',(.08,.12,.16),.37,.75)
def register(o,m,bone='chest'):
 o.data.materials.clear();o.data.materials.append(m)
 for p in o.data.polygons:p.use_smooth=True
 if not o.data.uv_layers:
  layer=o.data.uv_layers.new(name='FurFlowUV')
  for poly in o.data.polygons:
   for li in poly.loop_indices:
    p=o.data.vertices[o.data.loops[li].vertex_index].co;layer.data[li].uv=(p.x*.6+p.y*.25,p.z*.5)
 o.data.uv_layers[0].name='CharacterUV'
 parts.append(o);binds[o.name]=bone;return o
def ball(name,pos,scale,m,bone='head'):
 bpy.ops.mesh.primitive_uv_sphere_add(segments=16,ring_count=10,location=pos);o=bpy.context.object;o.name=name;o.scale=scale;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);return register(o,m,bone)
def tube(name,points,radii,m,bone='head',sides=7):
 vs=[];fs=[]
 for i,p in enumerate(points):
  tangent=Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)]);tangent.normalize();axis=tangent.cross(Vector((0,0,1)))
  if axis.length<.1:axis=tangent.cross(Vector((1,0,0)))
  axis.normalize();other=tangent.cross(axis).normalized()
  for j in range(sides):vs.append(tuple(Vector(p)+radii[i]*(axis*math.cos(j*math.tau/sides)+other*math.sin(j*math.tau/sides))))
 for i in range(len(points)-1):
  for j in range(sides):a=i*sides+j;b=i*sides+(j+1)%sides;fs.append((a,b,b+sides,a+sides))
 fs.extend([tuple(reversed(range(sides))),tuple((len(points)-1)*sides+j for j in range(sides))]);mesh=bpy.data.meshes.new(name);mesh.from_pydata(vs,[],fs);mesh.update();layer=mesh.uv_layers.new(name='FurFlowUV')
 for poly in mesh.polygons:
  for li in poly.loop_indices:
   idx=mesh.loops[li].vertex_index;layer.data[li].uv=((math.cos((idx%sides)*math.tau/sides)+1)*.5,(idx//sides)/max(1,len(points)-1))
 o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o);return register(o,m,bone)
# Flattened, curved clumps hide their narrow roots beneath the skin.
def fur_lock(name,points,widths,normal,material,bone='AUTO'):
 vs=[];fs=[];normal=Vector(normal).normalized();sides=8
 for i,p in enumerate(points):
  tangent=(Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)])).normalized()
  side=normal.cross(tangent)
  if side.length<.05:side=Vector((1,0,0)).cross(tangent)
  side.normalize();out=tangent.cross(side).normalized()
  for j in range(sides):
   a=j*math.tau/sides;vs.append(tuple(Vector(p)+side*math.cos(a)*widths[i]+out*math.sin(a)*widths[i]*.24))
 for i in range(len(points)-1):
  for j in range(sides):a=i*sides+j;b=i*sides+(j+1)%sides;fs.append((a,b,b+sides,a+sides))
 fs.append(tuple(reversed(range(sides))));fs.append(tuple((len(points)-1)*sides+j for j in range(sides)))
 data=bpy.data.meshes.new(name);data.from_pydata(vs,[],fs);data.update();card=data.attributes.new('_furcard','FLOAT','POINT')
 for val in card.data:val.value=1.
 layer=data.uv_layers.new(name='CharacterUV')
 for poly in data.polygons:
  for li in poly.loop_indices:
   idx=data.loops[li].vertex_index;layer.data[li].uv=((math.cos((idx%sides)*math.tau/sides)+1)*.5,(idx//sides)/max(1,len(points)-1))
 ob=bpy.data.objects.new(name,data);bpy.context.collection.objects.link(ob);return register(ob,material,bone)

# Read only the licensed body group, excluding all MakeHuman helper geometry.
v=[];f=[];face_uv=[];texcoords=[];g=''
for l in (ROOT/'art-source/makehuman-base.obj').read_text().splitlines():
 if l.startswith('v '):v.append(tuple(map(float,l.split()[1:4])))
 elif l.startswith('vt '):texcoords.append(tuple(map(float,l.split()[1:3])))
 elif l.startswith('g '):g=l.split()[1]
 elif l.startswith('f ') and g=='body':
  f.append([int(x.split('/')[0])-1 for x in l.split()[1:]]);face_uv.append([int(x.split('/')[1])-1 for x in l.split()[1:]])
verts=[]
for x,y,z in v:
 h=(y+8.1676)*.18;x*=.18;front=-z*.18
 chest=math.exp(-((h-2.1)/.38)**2);x*=1+chest*1.05;front*=1+chest*.9
 # Thick limbs and enlarged wrists; preserve the source body topology.
 if abs(x)>.35 and h>1.6:front*=1.45
 if h<1.3:x*=1.18
 if front<-.015 and abs(x)<.48 and 1.4<h<2.3:
  front-=.07*math.exp(-((abs(x)-.18)/.17)**2-((h-2.10)/.15)**2)
 if h>2.05 and front>0:
  hump=math.exp(-((h-2.29)/.20)**2-(x/.44)**2);front+=.15*hump;h+=.035*hump
 if h<.24: h+=.13*max(0,min(1,(front+.05)/.18))
 verts.append((x,front,h))
eligible=[i for i,q in enumerate(f) if max(verts[j][2] for j in q)<2.63 and not any(abs(verts[j][0])>1.16 and verts[j][1]<-.71 and verts[j][2]<1.96 for j in q)];faces=[f[i] for i in eligible]
mesh=bpy.data.meshes.new('MakeHuman heroic derivative');mesh.from_pydata(verts,[],faces);mesh.update();layer=mesh.uv_layers.new(name='MakeHumanUV')
for poly,face_id in zip(mesh.polygons,eligible):
 for li,ti in zip(poly.loop_indices,face_uv[face_id]):layer.data[li].uv=texcoords[ti]
# Inflate source limb surfaces along their actual normals, retaining topology.
for vertex in mesh.vertices:
 p=vertex.co
 if (abs(p.x)>.46 and 1.6<p.z<2.35) or (.12<abs(p.x) and p.z<1.2):vertex.co+=vertex.normal*.065
mesh.update();body=bpy.data.objects.new('Heroic MakeHuman body',mesh);bpy.context.collection.objects.link(body);register(body,skin,'AUTO')
# Clothing and narrow wrist guards are extracted from the actual body surface.
def surface_patch(name,polygons,material,offset,thickness):
 used=sorted({i for q in polygons for i in q});lookup={v:i for i,v in enumerate(used)}
 mm=bpy.data.meshes.new(name);mm.from_pydata([tuple(mesh.vertices[i].co+mesh.vertices[i].normal*offset) for i in used],[],[tuple(lookup[i] for i in q) for q in polygons]);mm.update();o=bpy.data.objects.new(name,mm);bpy.context.collection.objects.link(o);register(o,material,'AUTO');bpy.context.view_layer.objects.active=o
 mod=o.modifiers.new('Fitted surface thickness','SOLIDIFY');mod.thickness=thickness;bpy.ops.object.modifier_apply(modifier=mod.name)
 return o
short_faces=[tuple(p.vertices) for p in mesh.polygons if .88+.025*math.sin(p.center.x*35)<p.center.z<1.36]
surface_patch('Fitted torn shorts',short_faces,leather,.028,.009)
for side in [-1,1]:
 selected=[tuple(p.vertices) for p in mesh.polygons if 1.15<side*p.center.x<1.30 and 1.98<p.center.z<2.09 and -.7<p.center.y<-.4]
 if selected:surface_patch('Fitted wrist guard',selected,gold,.018,.012)
# Body fur sampled by real surface area, with strands flowing toward extremities.
polys=[p for p in mesh.polygons if p.area>.00002 and .22<p.center.z<2.55 and not (.87<p.center.z<1.38)]
weights=[p.area for p in polys]
for i,poly in enumerate(rng.choices(polys,weights=weights,k=260)):
 center=poly.center.copy();normal=poly.normal.normalized()
 if abs(center.x)>1.16 and center.z>1.5:continue
 # Keep the chest's muscular planes readable beneath shorter nap.
 front=center.y<-.05 and abs(center.x)<.52
 if front:continue
 length=rng.uniform(.045,.11) if front else rng.uniform(.09,.21)
 flow=Vector((0,.12,-1)).normalized()
 if abs(center.x)>.6 and center.z>1.6:flow=Vector((math.copysign(.3,center.x),-.7,-.5)).normalized()
 start=center-normal*.023;middle=center+normal*.006+flow*length*.45;end=center+normal*.005+flow*length
 width=rng.uniform(.015,.033) if front else rng.uniform(.024,.046)
 fur_lock('Surface fur clump',[tuple(start),tuple(middle),tuple(end)],[width*.20,width,.002],normal,fur if i%9 else tips)
hv=[];hf=[]
for line in (ROOT/'art-source/head.obj').read_text().splitlines():
 if line.startswith('v '):hv.append(tuple(map(float,line.split()[1:])))
 if line.startswith('f '):hf.append(tuple(int(i)-1 for i in line.split()[1:]))
hmesh=bpy.data.meshes.new('SDF wolf head');hmesh.from_pydata(hv,[],hf);hmesh.update();head=bpy.data.objects.new('SDF wolf skull and open jaw',hmesh);bpy.context.collection.objects.link(head);register(head,fur,'head')
# Dominant-axis head UVs keep fur flow longitudinal instead of concentric.
for poly in head.data.polygons:
 side=abs(poly.normal.x)>abs(poly.normal.y)
 for li in poly.loop_indices:
  p=head.data.vertices[head.data.loops[li].vertex_index].co
  head.data.uv_layers[0].data[li].uv=((p.y+p.z)*.6,(p.y-p.z)*.6) if side else (p.x*.9,p.z*.8)


head.data.materials.append(mouth)
for poly in head.data.polygons:
 c=poly.center
 if abs((c.x/.245)**2+((c.y+.48)/.52)**2+((c.z-3.02)/.185)**2-1)<.22 and c.y<.1:poly.material_index=1
# Short sideburn clumps follow actual cheek surfaces, preserving eye/mouth openings.
cheeks=[p for p in head.data.polygons if p.material_index==0 and 2.83<p.center.z<3.29 and abs(p.center.x)>.16 and p.normal.x*p.center.x>0 and p.center.y>-.4]
if cheeks:
 for i,p in enumerate(rng.choices(cheeks,weights=[p.area for p in cheeks],k=210)):
  n=p.normal.normalized();c=p.center.copy();length=rng.uniform(.07,.14);flow=Vector((0,.8,-.5)).normalized()
  start=c-n*.009;middle=c+n*.008+flow*length*.4;end=c+n*.005+flow*length
  fur_lock('Cheek sideburn',[tuple(start),tuple(middle),tuple(end)],[.006,rng.uniform(.018,.031),.002],n,tips if i%9==0 else fur,'head')

ball('Tongue',(0,-.49,2.87),(.12,.21,.025),mouth)
nose_mesh=bpy.data.meshes.new('Canine nose')
nose_mesh.from_pydata([(-.12,-.88,3.35),(.12,-.88,3.35),(0,-.9,3.23),(-.10,-.76,3.34),(.10,-.76,3.34),(0,-.78,3.24)],[],[(0,2,1),(0,1,4,3),(1,2,5,4),(2,0,3,5),(3,4,5)])
nose_mesh.update();nose=bpy.data.objects.new('Triangular canine nose',nose_mesh);bpy.context.collection.objects.link(nose);register(nose,black,'head')
bpy.context.view_layer.objects.active=nose;bevel=nose.modifiers.new('Soft nose cartilage','BEVEL');bevel.width=.018;bevel.segments=3;bpy.ops.object.modifier_apply(modifier=bevel.name)
head_bvh=BVHTree.FromPolygons(hv,hf)
for s in [-1,1]:
 ball('Socket shadow',(s*.253,-.075,3.36),(.063,.042,.042),dark)
 globe=ball('Recessed eye globe',(s*.260,-.09,3.36),(.041,.025,.030),black);globe.rotation_euler.z=s*.85
 iris=ball('Amber iris',(s*.279,-.106,3.36),(.026,.008,.023),eye);iris.rotation_euler.z=s*.85
 pupil=ball('Vertical pupil',(s*.283,-.110,3.36),(.006,.005,.021),black);pupil.rotation_euler.z=s*.85
 ball('Eye catchlight',(s*.278,-.112,3.371),(.005,.003,.005),ivory)
 fur_lock('Heavy fur brow',[(s*.16,-.16,3.4),(s*.29,-.16,3.43),(s*.35,-.07,3.42)],[.015,.05,.006],(s*.5,-.8,.4),dark,'head')
 tube('Swept wolf ear',[(s*.23,.04,3.43),(s*.37,.20,3.54),(s*.45,.36,3.60)],[.14,.105,.008],fur)
 tube('Ear inner velvet',[(s*.27,-.04,3.47),(s*.37,.12,3.55),(s*.42,.29,3.58)],[.07,.048,.005],mouth)
 upper_lip=[];lower_lip=[]
 for i in range(7):
  yy=-.70+i*.064;xx=s*(.11+.045*i/6)
  top,_,_,_=head_bvh.ray_cast(Vector((xx,yy,3.0)),Vector((0,0,1)),.5)
  bottom,_,_,_=head_bvh.ray_cast(Vector((xx,yy,3.0)),Vector((0,0,-1)),.4)
  if top is None or bottom is None:continue
  uz=top.z+.008;lz=bottom.z-.006;length=.17 if i==1 else .035+rng.random()*.025
  upper_lip.append((xx,yy,uz));lower_lip.append((xx,yy,lz))
  tube('Anchored upper tooth',[(xx,yy,uz),(xx,yy-.008,uz-length*.6),(xx*.95,yy+.007,uz-length)],[.026 if i==1 else .018,.013,.001],ivory)
  tube('Anchored lower tooth',[(xx,yy,lz),(xx,yy-.008,lz+.06)],[.016,.001],ivory)
 if len(upper_lip)>2:tube('Upper gingival margin',upper_lip,[.014]*len(upper_lip),mouth)
 if len(lower_lip)>2:tube('Lower gingival margin',lower_lip,[.012]*len(lower_lip),mouth)
ball('Dense collar undercoat',(0,.24,2.52),(.35,.29,.43),fur,'chest')
# Overlapping swept mane flows backward from the skull, not straight down.
for i in range(520):
 a=rng.uniform(-math.pi,math.pi);z=rng.uniform(2.17,3.49);width=rng.uniform(.022,.040);r=.30+(3.45-z)*.17
 x=math.cos(a)*r;y=.08+math.sin(a)*r*.72
 if y<.01 and z>2.65:continue
 length=rng.uniform(.16,.29)
 lock=fur_lock('Swept layered mane',[(x,y-.015,z),(x*1.12,y+length*.3,z-.04),(x*1.23,y+length*.75,z-.13),(x*1.22,y+length,z-.20)],[width*.25,width,width*.62,.003],(x,y-.08,.12),tips if i%5==0 else fur,'head' if z>2.73 else 'chest')
 if i%3==0:
  for val in lock.data.attributes['_furcard'].data:val.value=0.
# Lathed bronze wrist cuffs, oriented along the forearm.
for s in [-1,1]:
 bpy.ops.mesh.primitive_torus_add(major_radius=.102,minor_radius=.018,major_segments=24,minor_segments=6,location=(s*1.23,-.54,2.03));o=bpy.context.object;o.name='Lathed bronze wrist cuff';o.rotation_euler=Vector((s*.3,-.75,-.4)).to_track_quat('Z','Y').to_euler();register(o,gold,'forearm.L' if s<0 else 'forearm.R')
# Authored palms and articulated swept digits meet the retained body at the wrist.
horn=mat('12 • dark keratin claws',(.065,.078,.072),.32)
for side,label in [(-1,'L'),(1,'R')]:
 handbone='hand.'+label
 ball('Wolf palm',(side*1.28,-.85,1.87),(.16,.13,.15),fur,handbone)
 for finger in range(4):
  x=side*(1.16+finger*.08)
  points=[(x,-.92,1.91-finger*.018),(x+side*.015,-1.09,1.91),(x+side*.025,-1.13,2.04),(x+side*.025,-1.06,2.11)]
  tube('Articulated wolf digit',points,[.047,.044,.034,.024],fur,'finger%d.'%finger+label,10)
  tip=points[-1];tube('Curved keratin talon',[tip,(tip[0],tip[1]+.01,tip[2]+.09),(tip[0]-side*.025,tip[1]+.09,tip[2]+.15)],[.026,.019,.001],horn,'finger%d.'%finger+label,9)
 thumb=[(side*1.14,-.84,1.82),(side*1.04,-.96,1.9),(side*1.02,-.99,2.04)]
 tube('Opposable wolf thumb',thumb,[.06,.045,.025],fur,'finger4.'+label,10)
 tube('Thumb talon',[thumb[-1],(side*1.04,-.97,2.13),(side*1.10,-.92,2.16)],[.026,.018,.001],horn,'finger4.'+label,9)
 for j in range(3):tube('Foot talon',[(side*.23+(j-1)*.065,-.04,.07),(side*.23+(j-1)*.065,-.13,.06),(side*.23+(j-1)*.065,-.2,.07)],[.04,.03,.003],horn,'foot.'+label)
tube('Heavy wolf tail',[(0,.1,1.05),(0,.42,.9),(.24,.65,.55),(.5,.78,.48),(.64,.85,.6)],[.12,.14,.12,.08,.01],fur,'pelvis')
# Shared original CC0 humanoid rig, used by all skinned parts.
bpy.ops.object.armature_add();rig=bpy.context.object;rig.name='Moonfang Shared Humanoid';bpy.ops.object.mode_set(mode='EDIT');rig.data.edit_bones.remove(rig.data.edit_bones[0])
bones={}
def bone(name,a,b,parent=None):
 e=rig.data.edit_bones.new(name);e.head=a;e.tail=b
 if parent:e.parent=bones[parent]
 bones[name]=e
bone('root',(0,0,0),(0,0,.2));bone('pelvis',(0,0,1.15),(0,0,1.5),'root');bone('chest',(0,0,1.5),(0,0,2.4),'pelvis');bone('head',(0,0,2.4),(0,0,3.4),'chest')
for s,label in [(-1,'L'),(1,'R')]:
 bone('upperarm.'+label,(s*.4,0,2.3),(s*1.05,-.12,2.13),'chest');bone('forearm.'+label,(s*1.05,-.12,2.13),(s*1.28,-.7,1.95),'upperarm.'+label);bone('hand.'+label,(s*1.28,-.7,1.95),(s*1.3,-.9,1.8),'forearm.'+label)
 for finger in range(5):
  bone('finger%d.'%finger+label,(s*(1.2+finger*.035),-.88-finger*.026,1.84-finger*.015),(s*(1.24+finger*.035),-1.04-finger*.026,1.78-finger*.015),'hand.'+label)
 bone('thigh.'+label,(s*.18,0,1.25),(s*.21,0,.65),'pelvis');bone('shin.'+label,(s*.21,0,.65),(s*.21,0,.18),'thigh.'+label);bone('foot.'+label,(s*.21,0,.18),(s*.21,-.22,.08),'shin.'+label)
bpy.ops.object.mode_set(mode='OBJECT')
def dist(p,a,b):
 ab=b-a;t=max(0,min(1,(p-a).dot(ab)/max(ab.length_squared,1e-6)));return (p-a-ab*t).length
for ob in parts:
 groups={n:ob.vertex_groups.new(name=n) for n in bones};ob.parent=rig;mod=ob.modifiers.new('Shared skeleton','ARMATURE');mod.object=rig
 for v in ob.data.vertices:
  p=ob.matrix_world@v.co
  if binds[ob.name]!='AUTO':groups[binds[ob.name]].add([v.index],1.,'REPLACE');continue
  candidates=[(dist(p,b.head_local,b.tail_local),b.name) for b in rig.data.bones if b.name!='root'];candidates.sort();a,b=candidates[:2];w=1/(.001+a[0])**4;w2=1/(.001+b[0])**4;groups[a[1]].add([v.index],w/(w+w2),'REPLACE');groups[b[1]].add([v.index],w2/(w+w2),'REPLACE')
# Convert deliberate world-axis posing into each bone's rest basis.
def local_pose(name,axis,angle):
 basis=rig.data.bones[name].matrix_local.to_quaternion()
 return (basis.inverted() @ Quaternion(axis,angle) @ basis).to_euler('XYZ')
BASE={
 'chest':local_pose('chest',(1,0,0),.12),
 'head':local_pose('head',(1,0,0),-.48),
 'upperarm.L':local_pose('upperarm.L',(0,1,0),.23),
 'forearm.L':local_pose('forearm.L',(1,0,0),-.50),
 'hand.L':local_pose('hand.L',(1,0,0),-.16),
 'upperarm.R':local_pose('upperarm.R',(0,1,0),.24),
 'forearm.R':local_pose('forearm.R',(1,0,0),-.25),
 'thigh.L':local_pose('thigh.L',(0,1,0),.14),
 'thigh.R':local_pose('thigh.R',(0,1,0),-.14),
}
# Bake original shared clips, no simulated third-party retargeting claim.
for action,length in [('Idle',72),('Run',24),('Cast',48),('Attack',32),('Hit',20),('Death',60)]:
 rig.animation_data_create();rig.animation_data.action=bpy.data.actions.new(action)
 for frame in range(0,length+1,4):
  t=frame/length;wave=math.sin(t*math.tau)
  for p in rig.pose.bones:p.rotation_mode='XYZ';p.rotation_euler=BASE.get(p.name,(0,0,0));p.location=(0,0,0)
  rig.pose.bones['chest'].rotation_euler.x+=.035*wave
  if action=='Run':
   for label,s in [('L',1),('R',-1)]:
    rig.pose.bones['thigh.'+label].rotation_euler.x=wave*s*.65;rig.pose.bones['shin.'+label].rotation_euler.x=max(0,-wave*s)*.7;rig.pose.bones['upperarm.'+label].rotation_euler.x=-wave*s*.45
   rig.pose.bones['pelvis'].location.z=abs(wave)*.05
  if action in ['Cast','Attack','Hit','Death']:
   pulse=math.sin(t*math.pi);rig.pose.bones['chest'].rotation_euler.x+=(-.15 if action=='Cast' else .25)*pulse
   rig.pose.bones['upperarm.R'].rotation_euler.y=-pulse*(1.1 if action=='Attack' else .6);rig.pose.bones['head'].rotation_euler.x+=(-.18 if action=='Cast' else .20)*pulse
  if action=='Death':
   for name in ['head','chest','forearm.L','forearm.R','hand.L','hand.R']:
    rig.pose.bones[name].rotation_euler=tuple(a*(1-t) for a in BASE.get(name,(0,0,0)))
   rig.pose.bones['root'].rotation_euler.x=-min(1,t*1.6)*1.35;rig.pose.bones['root'].location.z=-t*.15
  for p in rig.pose.bones:p.keyframe_insert('rotation_euler',frame=frame,group=p.name);p.keyframe_insert('location',frame=frame,group=p.name)
 track=rig.animation_data.nla_tracks.new();track.name=action;track.strips.new(action,0,rig.animation_data.action);rig.animation_data.action=None
# Merge by material into one skinned character: material slots remain physically distinct.
bpy.ops.object.select_all(action='DESELECT')
for o in parts:o.select_set(True)
bpy.context.view_layer.objects.active=body;bpy.ops.object.join();body.name='Moonfang skinned character';body.modifiers.clear();dec=body.modifiers.new('Camera-scale topology budget','DECIMATE');dec.ratio=.72;bpy.ops.object.modifier_apply(modifier=dec.name);mod=body.modifiers.new('Shared skeleton','ARMATURE');mod.object=rig
from mathutils.bvhtree import BVHTree
bvh=BVHTree.FromPolygons([v.co for v in body.data.vertices],[tuple(p.vertices) for p in body.data.polygons])
ao=body.data.attributes.new('_ao','FLOAT','POINT')
for v in body.data.vertices:
 n=v.normal.normalized();t=n.cross(Vector((0,0,1)))
 if t.length<.1:t=n.cross(Vector((1,0,0)))
 t.normalize();b=n.cross(t);clear=0
 for sample in range(12):
  z=.15+.85*(sample+.5)/12;r=math.sqrt(1-z*z);angle=sample*2.3999632297
  ray=(n*z+t*math.cos(angle)*r+b*math.sin(angle)*r).normalized()
  hit,_,_,distance=bvh.ray_cast(v.co+n*.014,ray,.18)
  if hit is None or distance<.025:clear+=1
 ao.data[v.index].value=.5+.5*clear/12
bpy.ops.object.select_all(action='DESELECT');body.select_set(True);rig.select_set(True);bpy.context.view_layer.objects.active=rig
bpy.context.scene.frame_set(0)
bpy.ops.export_scene.gltf(filepath=str(OUT/'moonfang.glb'),export_format='GLB',use_selection=True,export_animations=True,export_animation_mode='NLA_TRACKS',export_skins=True,export_yup=True,export_attributes=True)
print('MOONFANG GENERATED',OUT/'moonfang.glb')
