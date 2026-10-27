"""Blender: export original benchmark geometry and licensed scenery for Unreal.
Run from repository root: blender -b -t 2 -P unreal/Scripts/prepare-art.py
Does not download anything. Units in Blender are metres; FBX exports centimetres.
"""
import bpy, json, math, random, shutil
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'unreal'/'ArtSource'; OUT.mkdir(exist_ok=True)
manifest=json.loads((ROOT/'client/src/asset-manifest.json').read_text())
records={m['id']:m for m in manifest['materials']}
materials={}
def stage_material(name):
 if name in materials: return
 maps={}
 for channel in ['color','normal','orm']:
  candidates=[ROOT/'art-source/scenery/converted'/f'{name}-{channel}.png',ROOT/'art-source/scenery/converted'/f'{name}-{channel}.jpg',ROOT/'art-source/polyhaven/converted'/f'{name}-{channel}.png',ROOT/'art-source/polyhaven/converted'/f'{name}-{channel}.jpg',ROOT/'art-source/textures'/f'{name}-{channel}.png',ROOT/'art-source/textures'/f'{name}-{channel}.jpg']
  source=next((p for p in candidates if p.exists()),None)
  if not source: raise RuntimeError(f'Missing required source {name}/{channel}')
  dest=OUT/(name+'-'+channel+source.suffix);shutil.copy2(source,dest);maps[channel]=dest.name
 materials[name]=maps

def reset():
 bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
def export(name):
 bpy.ops.object.select_all(action='SELECT')
 bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',apply_unit_scale=True,global_scale=1,bake_anim=False,add_leaf_bones=False,path_mode='STRIP')
def material(name):
 m=bpy.data.materials.get(name) or bpy.data.materials.new(name);return m

for record in manifest['scenery']:
 if record['id'] not in ['tree','conifer','shrub','grass']: continue
 reset();bpy.ops.import_scene.gltf(filepath=str(ROOT/'client/public'/record['lods'][0]['url'].lstrip('/')))
 for ob in bpy.context.scene.objects:
  if ob.type!='MESH': continue
  for slot in ob.material_slots:
   if not slot.material: continue
   name=slot.material.name.split('.')[0]; binding=record['materialBindings'].get(name)
   if not binding: raise RuntimeError('Unbound vegetation material '+name)
   target=binding['material'];stage_material(target)
   # Bake the glTF texture-coordinate selection/transform into the FBX UV0.
   uv_index=binding.get('texCoord',0);src=ob.data.uv_layers[uv_index];uvs=[tuple(v.uv) for v in src.data]
   tr=binding.get('transform',{});scale=tr.get('scale',[1,1]);offset=tr.get('offset',[0,0])
   index=ob.data.materials.find(slot.material.name)
   for poly in ob.data.polygons:
    if poly.material_index==index:
     for li in poly.loop_indices:
      u,v=uvs[li];ob.data.uv_layers[0].data[li].uv=(u*scale[0]+offset[0],1-((1-v)*scale[1]+offset[1]))
   slot.material=material(target.replace('-','_'))
 export(record['id'])

reset()
def height(x,y):
 q=math.hypot(x/11,(y-4)/8);base=.4+math.sin(x*.13)*math.cos(y*.12)*.4+max(0,math.hypot(x,y)-32)*.1
 t=max(0,min(1,(q-.9)/.45));return -.95+(base+.95)*t*t*(3-2*t)
N=192;verts=[(-90+x*180/N,-90+y*180/N,height(-90+x*180/N,-90+y*180/N)) for y in range(N+1) for x in range(N+1)]
faces=[(y*(N+1)+x,y*(N+1)+x+1,(y+1)*(N+1)+x+1,(y+1)*(N+1)+x) for y in range(N) for x in range(N)]
m=bpy.data.meshes.new('Sculpted ground');m.from_pydata(verts,[],faces);m.update();ob=bpy.data.objects.new('Ground',m);bpy.context.collection.objects.link(ob);m.materials.append(material('meadow'));uv=m.uv_layers.new()
for poly in m.polygons:
 poly.use_smooth=True
 for li in poly.loop_indices:
  p=m.vertices[m.loops[li].vertex_index].co;uv.data[li].uv=(p.x/3,p.y/3)
export('Ground')
reset()
def box(name,pos,size,mat,rot=0):
 bpy.ops.mesh.primitive_cube_add(size=1,location=pos);o=bpy.context.object;o.name=name;o.dimensions=size;o.rotation_euler.y=rot;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(material(mat));mod=o.modifiers.new('Rounded construction edges','BEVEL');mod.width=.035;mod.segments=3;bpy.ops.object.modifier_apply(modifier=mod.name)
 # Re-project UVs after beveling rather than stretching a whole tile over a wall.
 bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.cube_project(cube_size=2);bpy.ops.object.mode_set(mode='OBJECT')
box('Foundation',(-17,13,.25),(8.6,6.7,1),'concrete')
box('Rear wall',(-17,16,2.3),(8, .3,3.4),'wall')
for x in [-21,-13]: box('Side wall',(x,13,2.3),(.25,6,3.4),'wall')
# Continuous front wall around the door and glazing, with no overlapping openings.
for x,w in [(-20.35,1.3),(-17.55,1.1),(-13.55,1.1)]:box('Front wall',(x,10,2.3),(w,.3,3.4),'wall')
box('Window lower',(-15.45,10,1.15),(2.7,.3,1.1),'wall');box('Window header',(-15.45,10,3.65),(2.7,.3,.7),'wall')
box('Door header',(-18.95,10,3.5),(1.7,.3,1),'wall');box('Timber door',(-18.95,9.96,1.85),(1.7,.12,2.5),'wood')
box('Glazing',(-15.45,10.03,2.45),(2.7,.06,1.5),'glass')
for x in [-16.8,-15.45,-14.1]:box('Window frame',(x,9.8,2.45),(.06,.12,1.65),'wood')
for z in [1.63,3.27]:box('Window frame',(-15.45,9.8,z),(2.8,.12,.08),'wood')
for side in [-1,1]:box('Pitched roof',(-17+side*2.2,13,4.4),(5,.18,7.2),'roof',side*.44)
box('Ridge cap',(-17,13,5.46),(.24,7.25,.18),'roof');box('Chimney',(-19,14,4.9),(.65,.7,2.2),'concrete')
box('Step',(-18.95,9.3,.55),(2.2,1.2,.3),'concrete')
for i in range(34):box('Dock plank',(0,-8+i*.28,.62),(3.1,.265,.12),'wood')
for x in [-1.4,1.4]:
 for y in [-7.8,-3,1.2]:box('Dock post',(x,y,.25),(.17,.17,1.55),'wood')
box('Bench seat',(-12,12,.85),(.7,2.7,.12),'wood')
for y in [11,13]:box('Bench legs',(-12,y,.5),(.55,.16,.7),'wood')
r=random.Random(915)
for i in range(55):
 a=i/55*math.tau;x=math.cos(a)*12.7;y=4+math.sin(a)*9.5
 if abs(x)<2 and y<4:continue
 bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2,radius=1,location=(x,y,height(x,y)+.18));o=bpy.context.object;o.scale=(.35+r.random()*.6,.4+r.random()*.7,.3+r.random()*.5);o.data.materials.append(material('gravel'))
export('Architecture')
reset();bpy.ops.mesh.primitive_circle_add(vertices=128,radius=1,fill_type='NGON',location=(0,4,.04));ob=bpy.context.object;ob.scale=(11.9,8.6,1);ob.data.materials.append(material('water'));export('Pond')
for name in ['meadow','soil','gravel','wall','roof','concrete']:stage_material(name)
(OUT/'materials.json').write_text(json.dumps(materials,indent=2))
shutil.copy2(ROOT/'client/public/textures/overcast.hdr',OUT/'overcast.hdr')
shutil.copy2(ROOT/'docs/ASSET-PROVENANCE.md',OUT/'ASSET-PROVENANCE.md')
print('UNREAL ART READY:',OUT)
