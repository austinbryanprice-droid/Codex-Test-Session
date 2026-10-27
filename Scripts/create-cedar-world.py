"""UE 5.8.4 editor Python. Import staged art, build materials, save playable map.
Required imports fail explicitly. Run via Setup-Windows.ps1, after C++ build.
"""
import unreal as u, json, math, random
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];ART=ROOT/'ArtSource'
ASSETS=u.AssetToolsHelpers.get_asset_tools();LIB=u.EditorAssetLibrary
MAT=u.MaterialEditingLibrary;ACTORS=u.get_editor_subsystem(u.EditorActorSubsystem)
LEVEL=u.get_editor_subsystem(u.LevelEditorSubsystem)

def import_asset(filename,name):
 task=u.AssetImportTask();task.set_editor_property('filename',str(ART/filename));task.set_editor_property('destination_path','/Game/Cedar/Imported');task.set_editor_property('destination_name',name);task.set_editor_property('automated',True);task.set_editor_property('replace_existing',True);task.set_editor_property('save',True)
 if filename.endswith('.fbx'):
  options=u.FbxImportUI();options.set_editor_property('automated_import_should_detect_type',False);options.set_editor_property('import_mesh',True);options.set_editor_property('import_materials',False);options.set_editor_property('import_textures',False);options.set_editor_property('import_as_skeletal',False);options.set_editor_property('mesh_type_to_import',u.FBXImportType.FBXIT_STATIC_MESH)
  options.static_mesh_import_data.set_editor_property('combine_meshes',True);options.static_mesh_import_data.set_editor_property('convert_scene',True);options.static_mesh_import_data.set_editor_property('convert_scene_unit',True);options.static_mesh_import_data.set_editor_property('auto_generate_collision',False)
  task.set_editor_property('options',options)
 ASSETS.import_asset_tasks([task]);objects=[LIB.load_asset(p) for p in task.get_editor_property('imported_object_paths')];expected=u.StaticMesh if filename.endswith('.fbx') else u.Texture
 asset=next((a for a in objects if isinstance(a,expected)),None)
 if not asset:raise RuntimeError('Required art import failed: '+filename)
 return asset

def node(m,cls,**props):
 n=MAT.create_material_expression(m,cls)
 for k,v in props.items():n.set_editor_property(k,v)
 return n

def scalar(m,value,prop):
 n=node(m,u.MaterialExpressionConstant,r=value);MAT.connect_material_property(n,'',prop)

def make_material(name,maps=None):
 path='/Game/Cedar/Materials/M_'+name.replace('-','_')
 m=LIB.load_asset(path)
 if m: MAT.delete_all_material_expressions(m)
 else: m=ASSETS.create_asset(path.rsplit('/',1)[1],'/Game/Cedar/Materials',u.Material,u.MaterialFactoryNew())
 if maps:
  textures={channel:import_asset(file,name.replace('-','_')+'_'+channel) for channel,file in maps.items()}
  for channel,t in textures.items():
   t.set_editor_property('srgb',channel=='color')
   if channel=='normal':t.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_NORMALMAP);t.set_editor_property('flip_green_channel',True)
   elif channel=='orm':t.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_MASKS)
   LIB.save_loaded_asset(t)
  base=node(m,u.MaterialExpressionTextureSample,texture=textures['color']);normal=node(m,u.MaterialExpressionTextureSample,texture=textures['normal']);normal.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_NORMAL)
  orm=node(m,u.MaterialExpressionTextureSample,texture=textures['orm']);orm.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_MASKS)
  MAT.connect_material_property(base,'RGB',u.MaterialProperty.MP_BASE_COLOR);MAT.connect_material_property(normal,'RGB',u.MaterialProperty.MP_NORMAL)
  for output,prop in [('R',u.MaterialProperty.MP_AMBIENT_OCCLUSION),('G',u.MaterialProperty.MP_ROUGHNESS),('B',u.MaterialProperty.MP_METALLIC)]:MAT.connect_material_property(orm,output,prop)
  foliage=any(word in name for word in ['tree','shrub','grass','fir'])
  if foliage:
   m.set_editor_property('two_sided',True);m.set_editor_property('blend_mode',u.BlendMode.BLEND_MASKED);m.set_editor_property('opacity_mask_clip_value',.35)
   MAT.connect_material_property(base,'A',u.MaterialProperty.MP_OPACITY_MASK)
 else:
  colors={'wood':(.2,.12,.055),'glass':(.08,.13,.15),'water':(.025,.08,.065)}
  color=node(m,u.MaterialExpressionConstant3Vector,constant=u.LinearColor(*colors.get(name,(.3,.3,.3)),1))
  MAT.connect_material_property(color,'',u.MaterialProperty.MP_BASE_COLOR)
  scalar(m,.07 if name in ['water','glass'] else .8,u.MaterialProperty.MP_ROUGHNESS)
  scalar(m,.15 if name in ['water','glass'] else 0,u.MaterialProperty.MP_METALLIC)
  if name=='water':
   # Two moving detail normals produce fine ripples; Lumen supplies scene reflections.
   source=LIB.load_asset('/Game/Cedar/Imported/meadow_normal')
   tex=node(m,u.MaterialExpressionTextureSample,texture=source);tex.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_NORMAL)
   pan=node(m,u.MaterialExpressionPanner,speed_x=.012,speed_y=.008);uv=node(m,u.MaterialExpressionTextureCoordinate,u_tiling=18.,v_tiling=18.)
   MAT.connect_material_expressions(uv,'',pan,'Coordinate');MAT.connect_material_expressions(pan,'',tex,'UVs');MAT.connect_material_property(tex,'RGB',u.MaterialProperty.MP_NORMAL)
 MAT.recompile_material(m);LIB.save_loaded_asset(m);return m

manifest=json.loads((ART/'materials.json').read_text());materials={name.replace('-','_'):make_material(name,maps) for name,maps in manifest.items()}
for name in ['wood','glass','water']:materials[name]=make_material(name)
meshes={name:import_asset(name+'.fbx',name) for name in ['Ground','Architecture','Pond','tree','conifer','shrub','grass']}
MESH=u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
for name,mesh in meshes.items():
 for i,slot in enumerate(mesh.get_editor_property('static_materials')):
  key=str(slot.material_slot_name).split('.')[0]
  if key not in materials:raise RuntimeError(f'Unbound material slot: {name}/{key}')
  mesh.set_material(i,materials[key])
 if name in ['Ground','Architecture']:
  nanite=mesh.get_editor_property('nanite_settings');nanite.set_editor_property('enabled',True)
  MESH.set_nanite_settings(mesh,nanite,True)
  body=mesh.get_editor_property('body_setup');body.set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
 LIB.save_loaded_asset(mesh)

path='/Game/Cedar/Maps/CedarHollow'
if not LEVEL.new_level(path):raise RuntimeError('Could not create '+path)
def spawn(cls,name,pos=(0,0,0),rot=(0,0,0)):
 a=ACTORS.spawn_actor_from_class(cls,u.Vector(*pos),u.Rotator(*rot));a.set_actor_label(name);return a
def shape(mesh,name,pos=(0,0,0),scale=1,yaw=0):
 a=spawn(u.StaticMeshActor,name,pos,(0,yaw,0));c=a.static_mesh_component;c.set_static_mesh(mesh);a.set_actor_scale3d(u.Vector(scale,scale,scale));c.set_editor_property('cast_shadow',True);return a
shape(meshes['Ground'],'Sculpted forest floor');shape(meshes['Architecture'],'Cabin dock and shoreline');water=shape(meshes['Pond'],'Reflective pond');water.static_mesh_component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
r=random.Random(916)
def h(x,y):
 q=math.hypot(x/11,(y-4)/8);base=.4+math.sin(x*.13)*math.cos(y*.12)*.4+max(0,math.hypot(x,y)-32)*.1;t=max(0,min(1,(q-.9)/.45));return -.95+(base+.95)*t*t*(3-2*t)
for i in range(170):
 a=r.random()*math.tau;d=27+r.random()*49;x=math.cos(a)*d;y=math.sin(a)*d
 if x>5 and y<-8 and d<38:continue
 shape(meshes['conifer' if i%3==0 else 'tree'],'Woodland tree',(x*100,y*100,h(x,y)*100),.8+r.random()*.7,r.random()*360)
for i in range(120):
 x=r.uniform(-45,45);y=r.uniform(-45,45)
 if math.hypot(x/11,(y-4)/8)<1.5 or (-23<x<-10 and 7<y<18):continue
 shape(meshes['shrub'],'Understory',(x*100,y*100,h(x,y)*100),.5+r.random()*.5,r.random()*360)
# One instanced grass actor, rather than thousands of independently ticking actors.
grass=spawn(u.CedarGrass,'Instanced meadow')
grass.set_editor_property('grass_mesh',meshes['grass']);grass.rebuild()
sun=spawn(u.DirectionalLight,'Golden hour sun',(0,0,12000),(-18,-32,0));sun.tags=['CedarSun'];c=sun.get_component_by_class(u.DirectionalLightComponent);c.set_mobility(u.ComponentMobility.MOVABLE);c.set_editor_property('intensity',65000.);c.set_editor_property('light_color',u.Color(255,219,174,255));c.set_editor_property('atmosphere_sun_light',True);c.set_editor_property('light_source_angle',1.8)
spawn(u.SkyAtmosphere,'Atmosphere')
sky=spawn(u.SkyLight,'Dynamic sky');c=sky.get_component_by_class(u.SkyLightComponent);c.set_mobility(u.ComponentMobility.MOVABLE);c.set_editor_property('real_time_capture',True)
fog=spawn(u.ExponentialHeightFog,'Woodland height haze');fog.tags=['CedarFog'];c=fog.get_component_by_class(u.ExponentialHeightFogComponent);c.set_editor_property('fog_density',.007);c.set_editor_property('fog_height_falloff',.18);c.set_editor_property('volumetric_fog',True);c.set_editor_property('volumetric_fog_distance',14000.)
light=spawn(u.PointLight,'Cabin lantern',(-1280,965,275));c=light.get_component_by_class(u.PointLightComponent);c.set_mobility(u.ComponentMobility.MOVABLE);c.set_editor_property('intensity',1800.);c.set_editor_property('attenuation_radius',1100.);c.set_editor_property('light_color',u.Color(255,182,91,255))
post=spawn(u.PostProcessVolume,'Filmic exposure');post.set_editor_property('unbound',True);settings=post.get_editor_property('settings')
for k,v in {'override_bloom_intensity':True,'bloom_intensity':.3,'override_vignette_intensity':True,'vignette_intensity':.2,'override_auto_exposure_bias':True,'auto_exposure_bias':0.}.items():settings.set_editor_property(k,v)
post.set_editor_property('settings',settings)
spawn(u.PlayerStart,'Explorer start',(1800,-2100,220),(0,130,0))
if not LEVEL.save_current_level():raise RuntimeError('Could not save benchmark level')
LIB.save_directory('/Game/Cedar',only_if_is_dirty=False,recursive=True)
# Only this marker means the entire required asset/level build succeeded.
(ROOT/'Saved').mkdir(exist_ok=True);(ROOT/'Saved/CedarAssets.ready').write_text('UE 5.8.4 Cedar Hollow complete\n')
u.log('CEDAR HOLLOW READY: /Game/Cedar/Maps/CedarHollow')
