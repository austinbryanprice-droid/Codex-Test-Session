"""Original procedural character. Blender 4.3; MakeHuman CC0 body input.
No hand-sculpted models, animation packs or proprietary content.
"""
import bpy,math,random,json
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'public/assets';rng=random.Random(45)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
parts=[];binds={}
def mat(name,color,rough=.55,metal=0):
 m=bpy.data.materials.new(name);m.diffuse_color=(*color,1);m.use_nodes=True;p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(*color,1);p.inputs['Roughness'].default_value=rough;p.inputs['Metallic'].default_value=metal;return m
skin=mat('01 • midnight blue hide',(.035,.095,.14));fur=mat('02 • slate blue fur',(.065,.15,.21));tips=mat('03 • silver blue mane',(.16,.28,.34));dark=mat('04 • fur roots and socket shadow',(.012,.028,.043));mouth=mat('05 • mouth and tinted lips',(.022,.004,.008),.7);ivory=mat('06 • warm ivory teeth and claws',(.8,.73,.52),.34);gold=mat('07 • worn ancient bronze',(.48,.25,.07),.28,.85);leather=mat('08 • charcoal woven loincloth',(.055,.05,.055),.95);eye=mat('09 • amber iris',(.96,.37,.035),.18);black=mat('10 • wet nose and pupils',(.008,.012,.015),.22);steel=mat('11 • blued armor',(.08,.12,.16),.37,.75)
def register(o,m,bone='chest'):
 o.data.materials.clear();o.data.materials.append(m)
 for p in o.data.polygons:p.use_smooth=True
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
 fs.extend([tuple(reversed(range(sides))),tuple((len(points)-1)*sides+j for j in range(sides))]);mesh=bpy.data.meshes.new(name);mesh.from_pydata(vs,[],fs);mesh.update();o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o);return register(o,m,bone)
# Read only the licensed body group, excluding all MakeHuman helper geometry.
v=[];f=[];g=''
for l in (ROOT/'art-source/makehuman-base.obj').read_text().splitlines():
 if l.startswith('v '):v.append(tuple(map(float,l.split()[1:4])))
 elif l.startswith('g '):g=l.split()[1]
 elif l.startswith('f ') and g=='body':f.append([int(x.split('/')[0])-1 for x in l.split()[1:]])
verts=[]
for x,y,z in v:
 h=(y+8.1676)*.18;x*=.18;front=-z*.18
 chest=math.exp(-((h-2.1)/.38)**2);x*=1+chest*1.05;front*=1+chest*.9
 # Thick limbs and enlarged wrists; preserve the source body topology.
 if abs(x)>.35 and h>1.6:front*=1.45
 if h<1.3:x*=1.18
 verts.append((x,front,h))
faces=[q for q in f if max(verts[i][2] for i in q)<2.63]
mesh=bpy.data.meshes.new('MakeHuman heroic derivative');mesh.from_pydata(verts,[],faces);mesh.update();
# Inflate source limb surfaces along their actual normals, retaining topology.
for vertex in mesh.vertices:
 p=vertex.co
 if (abs(p.x)>.46 and 1.6<p.z<2.35) or (.12<abs(p.x) and p.z<1.2):vertex.co+=vertex.normal*.045
mesh.update();body=bpy.data.objects.new('Heroic MakeHuman body',mesh);bpy.context.collection.objects.link(body);register(body,skin,'AUTO')
# Surface-derived armor: copy torso/forearm polygons, push along source normals,
# solidify and bevel. Skin weights use the same shared skeleton as the body.
for side in [-1,1]:
 selected=[]
 for p in mesh.polygons:
  c=p.center
  if (side*c.x>.43 and 1.65<c.z<2.15 and c.y<.08) or (side*c.x>.26 and 2.2<c.z<2.38 and c.y<.01):selected.append(tuple(p.vertices))
 used=sorted({i for q in selected for i in q});lookup={v:i for i,v in enumerate(used)}
 mm=bpy.data.meshes.new('Surface armor');mm.from_pydata([tuple(mesh.vertices[i].co+mesh.vertices[i].normal*.025) for i in used],[],[tuple(lookup[i] for i in q) for q in selected]);mm.update();o=bpy.data.objects.new('Fitted armor '+str(side),mm);bpy.context.collection.objects.link(o);register(o,steel,'AUTO');bpy.context.view_layer.objects.active=o
 mod=o.modifiers.new('Plate thickness','SOLIDIFY');mod.thickness=.018;bpy.ops.object.modifier_apply(modifier=mod.name)
 mod=o.modifiers.new('Plate bevel','BEVEL');mod.width=.012;mod.segments=2;bpy.ops.object.modifier_apply(modifier=mod.name)
hv=[];hf=[]
for line in (ROOT/'art-source/head.obj').read_text().splitlines():
 if line.startswith('v '):hv.append(tuple(map(float,line.split()[1:])))
 if line.startswith('f '):hf.append(tuple(int(i)-1 for i in line.split()[1:]))
hmesh=bpy.data.meshes.new('SDF wolf head');hmesh.from_pydata(hv,[],hf);hmesh.update();head=bpy.data.objects.new('SDF wolf skull and open jaw',hmesh);bpy.context.collection.objects.link(head);register(head,fur)

ball('Deep mouth',(0,-.19,3.02),(.23,.24,.18),mouth)
ball('Tongue',(0,-.49,2.87),(.12,.21,.025),mouth)
ball('Canine nose',(0,-.78,3.32),(.125,.09,.075),black)
for s in [-1,1]:
 ball('Socket shadow',(s*.275,-.285,3.36),(.102,.08,.077),dark)
 ball('Eye globe',(s*.283,-.333,3.368),(.055,.03,.035),ivory)
 ball('Amber iris',(s*.285,-.368,3.37),(.035,.01,.028),eye)
 ball('Vertical pupil',(s*.285,-.379,3.37),(.009,.005,.025),black)
 ball('Eye catchlight',(s*.273,-.384,3.385),(.009,.004,.009),ivory)
 tube('Heavy brow',[(s*.16,-.30,3.39),(s*.28,-.335,3.435),(s*.38,-.22,3.44)],[.065,.065,.025],dark)
 tube('Swept wolf ear',[(s*.26,.035,3.46),(s*.43,.06,3.67),(s*.49,.10,3.79)],[.14,.105,.008],fur)
 tube('Ear inner velvet',[(s*.30,-.065,3.5),(s*.43,-.012,3.68),(s*.47,.05,3.75)],[.07,.048,.005],mouth)
 for i in range(7):
  yy=-.7+i*.064;xx=s*(.17+.025*math.sin(i));length=.16 if i==1 else .06+rng.random()*.04
  tube('Upper fang',[(xx,yy,3.17),(xx,yy-.01,3.17-length*.65),(xx*.94,yy-.005,3.17-length)],[.035,.02,.002],ivory)
  tube('Lower tooth',[(xx,yy,2.85),(xx,yy-.01,2.93)],[.025,.002],ivory)
# Chunky swept mane, layered with broad tapered locks rather than needles.
for i in range(95):
 a=rng.uniform(-math.pi,math.pi);z=rng.uniform(2.18,3.49);width=rng.uniform(.055,.095);r=.34+(3.45-z)*.10
 x=math.cos(a)*r;y=.08+math.sin(a)*r*.62
 if y<.02 and z>2.55:continue
 tube('Layered mane lock',[(x,y,z),(x*1.22,y+.08,z-.12),(x*1.4,y+.18,z-.30),(x*1.42,y+.28,z-.38)],[width,width*.9,width*.4,.025],tips if i%4==0 else fur,'head' if z>2.73 else 'chest')
# Lathed bronze wrist cuffs, oriented along the forearm.
for s in [-1,1]:
 bpy.ops.mesh.primitive_torus_add(major_radius=.102,minor_radius=.018,major_segments=24,minor_segments=6,location=(s*1.23,-.54,2.03));o=bpy.context.object;o.name='Lathed bronze wrist cuff';o.rotation_euler=Vector((s*.3,-.75,-.4)).to_track_quat('Z','Y').to_euler();register(o,gold,'forearm.L' if s<0 else 'forearm.R')
# Large hands, curved finger talons, digitigrade feet and a thick tail.
for s in [-1,1]:
 for j in range(4):
  x=s*(1.22+j*.045);y=-1.02-j*.012;z=1.79-j*.018
  tube('Hand talon',[(x,y,z),(x+s*.10,y-.05,z-.03),(x+s*.12,y-.1,z+.05)],[.038,.027,.002],ivory,'hand.L' if s<0 else 'hand.R')
 for j in range(3):tube('Foot talon',[(s*.23+(j-1)*.065,-.04,.07),(s*.23+(j-1)*.065,-.13,.06),(s*.23+(j-1)*.065,-.2,.07)],[.04,.03,.003],ivory,'foot.L' if s<0 else 'foot.R')
tube('Heavy wolf tail',[(0,.1,1.05),(0,.42,.9),(.24,.65,.55),(.5,.78,.48),(.64,.85,.6)],[.12,.14,.12,.08,.01],fur,'pelvis')
# Ragged panelled loincloth: split by thigh and bound to the leg bones.
for s in [-1,1]:
 for i in range(7):
  a=i/7*math.pi+(math.pi if s<0 else 0);x=math.cos(a)*.31;y=math.sin(a)*.22
  tube('Thick cloth strip',[(x,y,1.3),(x*1.2,y*1.3,1.06),(x*1.3,y*1.5,.87+rng.random()*.08)],[.11,.11,.04],leather,'thigh.L' if x<0 else 'thigh.R',4)
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
# Bake original shared clips, no simulated third-party retargeting claim.
for action,length in [('Idle',72),('Run',24),('Cast',48),('Attack',32),('Hit',20),('Death',60)]:
 rig.animation_data_create();rig.animation_data.action=bpy.data.actions.new(action)
 for frame in range(0,length+1,4):
  t=frame/length;wave=math.sin(t*math.tau)
  for p in rig.pose.bones:p.rotation_mode='XYZ';p.rotation_euler=(0,0,0);p.location=(0,0,0)
  rig.pose.bones['chest'].rotation_euler.x=.035*wave
  if action=='Run':
   for label,s in [('L',1),('R',-1)]:
    rig.pose.bones['thigh.'+label].rotation_euler.x=wave*s*.65;rig.pose.bones['shin.'+label].rotation_euler.x=max(0,-wave*s)*.7;rig.pose.bones['upperarm.'+label].rotation_euler.x=-wave*s*.45
   rig.pose.bones['pelvis'].location.z=abs(wave)*.05
  if action in ['Cast','Attack','Hit','Death']:
   pulse=math.sin(t*math.pi);rig.pose.bones['chest'].rotation_euler.x=(-.15 if action=='Cast' else .25)*pulse
   rig.pose.bones['upperarm.R'].rotation_euler.y=-pulse*(1.1 if action=='Attack' else .6);rig.pose.bones['head'].rotation_euler.x=-.2*pulse
  if action=='Death':rig.pose.bones['root'].rotation_euler.x=-min(1,t*1.6)*1.35;rig.pose.bones['root'].location.z=-t*.15
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
 for ray in [n,(n+t*.8).normalized(),(n+b*.8).normalized()]:
  hit,_,_,_=bvh.ray_cast(v.co+n*.006,ray,.22)
  if hit is None:clear+=1
 ao.data[v.index].value=.5+.5*clear/3
bpy.ops.object.select_all(action='DESELECT');body.select_set(True);rig.select_set(True);bpy.context.view_layer.objects.active=rig
bpy.context.scene.frame_set(0)
bpy.ops.export_scene.gltf(filepath=str(OUT/'moonfang.glb'),export_format='GLB',use_selection=True,export_animations=True,export_animation_mode='NLA_TRACKS',export_skins=True,export_yup=True,export_attributes=True)
print('MOONFANG GENERATED',OUT/'moonfang.glb')
