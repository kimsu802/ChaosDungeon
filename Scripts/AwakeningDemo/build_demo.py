"""Build an isolated, editable Unreal Engine feasibility demo. No gameplay assets are modified."""
import unreal as u
import math, os, json, random

ROOT = '/Game/AwakeningDemo'
OUT = os.path.join(u.Paths.project_dir(), 'Saved', 'AwakeningDemo')
os.makedirs(OUT, exist_ok=True)
FPS, END = 24, 192
tools = u.AssetToolsHelpers.get_asset_tools()
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
world = u.EditorLoadingAndSavingUtils.new_blank_map(False)
assert u.EditorLoadingAndSavingUtils.save_map(world,ROOT+'/L_AwakeningDemo')
world.get_world_settings().set_editor_property('default_game_mode', u.GameModeBase)

def asset(name, cls, factory):
    old = u.load_asset(ROOT + '/' + name)
    if old:
        return old
    return tools.create_asset(name, ROOT, cls, factory)

def material(name, rgb, glow=0.0, metal=0.0, rough=.45):
    m = asset(name, u.Material, u.MaterialFactoryNew())
    lib=u.MaterialEditingLibrary
    lib.delete_all_material_expressions(m)
    c=lib.create_material_expression(m,u.MaterialExpressionConstant3Vector,-300,0)
    c.set_editor_property('constant',u.LinearColor(*rgb,1))
    lib.connect_material_property(c,'',u.MaterialProperty.MP_BASE_COLOR)
    r=lib.create_material_expression(m,u.MaterialExpressionConstant,-300,150)
    r.set_editor_property('r',rough)
    lib.connect_material_property(r,'',u.MaterialProperty.MP_ROUGHNESS)
    if glow:
        e=lib.create_material_expression(m,u.MaterialExpressionConstant3Vector,-300,-150)
        e.set_editor_property('constant',u.LinearColor(*(v*glow for v in rgb),1))
        lib.connect_material_property(e,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    if metal:
        e=lib.create_material_expression(m,u.MaterialExpressionConstant,-300,300)
        e.set_editor_property('r',metal)
        lib.connect_material_property(e,'',u.MaterialProperty.MP_METALLIC)
    lib.recompile_material(m)
    u.EditorAssetLibrary.save_loaded_asset(m)
    return m

stone=material('M_Arena',(0.045,.068,.09),0,.35,.33)
dark=material('M_Obsidian',(.012,.019,.028),0,.6,.28)
gold=material('M_SolarGold',(1,.38,.055),12)
cyan=material('M_AetherBlue',(.06,.58,1),9)
white=material('M_ImpactCore',(.65,.85,1),24)
trim=material('M_Trim',(.035,.2,.25),1,.5)

# Import a lightweight, deterministic ring mesh (XY plane, radius 100cm).
ring_path=os.path.join(OUT,'SM_AwakeningRing.obj')
with open(ring_path,'w') as f:
    f.write('o AwakeningRing\n')
    for i in range(129):
        a=i*2*math.pi/128
        for r in (98.5,101.5):
            f.write('v %.6f %.6f 0\n' % (math.cos(a)*r,math.sin(a)*r))
    for i in range(128):
        n=i*2+1
        f.write('f %d %d %d\nf %d %d %d\n' % (n,n+1,n+3,n,n+3,n+2))
task=u.AssetImportTask()
task.filename=ring_path
task.destination_path=ROOT
task.destination_name='SM_AwakeningRing'
task.automated=True
task.replace_existing=True
task.save=True
if not u.EditorAssetLibrary.does_asset_exist(ROOT+'/SM_AwakeningRing'):
    tools.import_asset_tasks([task])
ring=u.load_asset(ROOT+'/SM_AwakeningRing')
cube=u.load_asset('/Engine/BasicShapes/Cube')
cylinder=u.load_asset('/Engine/BasicShapes/Cylinder')
sphere=u.load_asset('/Engine/BasicShapes/Sphere')

def spawn(cls,name,loc=(0,0,0),rot=(0,0,0)):
    a=actors.spawn_actor_from_class(cls,u.Vector(*loc),u.Rotator(*rot))
    a.set_actor_label(name)
    return a

def mesh(name, model, loc, scale, mat, rot=(0,0,0)):
    a=spawn(u.StaticMeshActor,name,loc,rot)
    c=a.static_mesh_component
    c.set_mobility(u.ComponentMobility.MOVABLE)
    c.set_static_mesh(model)
    c.set_material(0,mat)
    c.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
    if mat in (gold,cyan,white,trim):
        c.set_cast_shadow(False)
    a.set_actor_scale3d(u.Vector(*scale))
    return a

seq=asset('LS_AwakeningDemo',u.LevelSequence,u.LevelSequenceFactoryNew())
# Rebuild sequence contents when rerunning after a script adjustment.
for b in seq.get_bindings():
    b.remove()
for t in seq.get_tracks():
    seq.remove_track(t)
seq.set_display_rate(u.FrameRate(FPS,1))
seq.set_playback_start(0)
seq.set_playback_end(END)

def binding(obj):
    return seq.add_possessable(obj)

def transform(a, keys, bound=None):
    b=bound or binding(a)
    section=b.add_track(u.MovieScene3DTransformTrack).add_section()
    section.set_range(0,END)
    channels=section.get_all_channels()
    for frame,loc,rot,scale in keys:
        values=(*loc,rot[2],rot[0],rot[1],*scale)
        for ch,v in zip(channels,values):
            ch.add_key(u.FrameNumber(frame),float(v),interpolation=u.MovieSceneKeyInterpolation.LINEAR)
    return b

def animate_scale(a, values, loc=None, rot=None):
    p=loc or tuple(a.get_actor_location().to_tuple())
    r=rot or (0,0,0)
    return transform(a,[(fr,p,r,sc if isinstance(sc,tuple) else (sc,sc,sc)) for fr,sc in values])

# Circular basalt arena, radial inlays and evenly spaced ruined obelisks.
mesh('Foundation',cylinder,(0,0,-65),(25,25,1.2),dark)
mesh('Arena',cylinder,(0,0,-12),(22,22,.24),stone)
for radius in (360,720,1020):
    mesh('Aether floor inlay',ring,(0,0,2),(radius/100,)*3,trim)
for i in range(24):
    ang=i*math.tau/24
    x,y=850*math.cos(ang),850*math.sin(ang)
    mesh('Radial stone joints',cube,(x,y,1.5),(.022,3.3,.018),dark,(0,math.degrees(ang)-90,0))
for i in range(10):
    ang=i*math.tau/10
    x,y=1190*math.cos(ang),1190*math.sin(ang)
    mesh('Obelisk base',cube,(x,y,35),(1.5,1.5,.7),dark,(0,math.degrees(ang),0))
    mesh('Obelisk',cube,(x,y,230),(.7,.7,3.3),stone,(0,math.degrees(ang),0))
    mesh('Obelisk crown',sphere,(x,y,420),(.15,.15,.42),cyan)
    light=spawn(u.PointLight,'Blue shrine light',(x,y,400))
    light.point_light_component.set_intensity(18000)
    light.point_light_component.set_light_color(u.LinearColor(.06,.45,1,1))
    light.point_light_component.set_attenuation_radius(520)

key=spawn(u.DirectionalLight,'Moon key',(0,0,500),(-38,-35,0))
key.get_component_by_class(u.DirectionalLightComponent).set_intensity(3.0)
key.get_component_by_class(u.DirectionalLightComponent).set_light_color(u.LinearColor(.5,.7,1,1))
for name,loc,col,intensity in [('Warm hero key',(240,-240,330),(1,.65,.28),95000),('Cool rim',(-200,100,300),(.1,.6,1),110000)]:
    a=spawn(u.PointLight,name,loc)
    a.point_light_component.set_intensity(intensity)
    a.point_light_component.set_light_color(u.LinearColor(*col,1))
    a.point_light_component.set_attenuation_radius(1100)
pp=spawn(u.PostProcessVolume,'Cinematic exposure')
pp.set_editor_property('unbound',True)
settings=pp.get_editor_property('settings')
for k,v in {'override_auto_exposure_method':True,'auto_exposure_method':u.AutoExposureMethod.AEM_MANUAL,'override_auto_exposure_bias':True,'auto_exposure_bias':3.0,'override_bloom_intensity':True,'bloom_intensity':.65,'override_vignette_intensity':True,'vignette_intensity':.3,'override_motion_blur_amount':True,'motion_blur_amount':.12}.items():
    settings.set_editor_property(k,v)
pp.set_editor_property('settings',settings)

report=json.load(open(os.path.join(OUT,'asset_report.json')))
hero=spawn(u.SkeletalMeshActor,'PLAYER - Greystone Dragonlord',(0,0,0),(0,-90,0))
hero.skeletal_mesh_component.set_skeletal_mesh_asset(u.load_asset(report['player_mesh']))
hero.skeletal_mesh_component.set_editor_property('visibility_based_anim_tick_option',u.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
hero.skeletal_mesh_component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
hb=transform(hero,[(0,(0,0,0),(0,-90,0),(1,1,1)),(37,(0,0,0),(0,-90,0),(1,1,1)),(61,(0,0,160),(0,-90,0),(1,1,1)),(76,(0,0,210),(0,-90,0),(1,1,1)),(82,(0,0,0),(0,-90,0),(1,1,1)),(END,(0,0,0),(0,-90,0),(1,1,1))])
anim=hb.add_track(u.MovieSceneSkeletalAnimationTrack)
base='/Game/Character/Player/ParagonGreystone/Characters/Heroes/Greystone/Animations/'
for name,start,end in [('Idle',0,15),('Ultimate_Entry',15,38),('Ultimate_Part1',38,78),('Ultimate_Part2',78,132),('Idle',132,END)]:
    sec=anim.add_section()
    sec.set_range(start,end)
    params=sec.get_editor_property('params')
    params.animation=u.load_asset(base+name)
    sec.set_editor_property('params',params)

for j,(radius,z,mat) in enumerate([(195,5,gold),(255,9,cyan),(140,190,gold)]):
    a=mesh('Charging sigil %d'%j,ring,(0,0,z),(.001,)*3,mat)
    animate_scale(a,[(0,.001),(14,.001),(29,radius/100),(66,radius/110),(78,radius/140),(82,.001),(END,.001)],(0,0,z))

random.seed(7)
for i in range(32):
    ang=i*math.tau/32
    r=250+random.random()*200
    z=60+random.random()*240
    a=mesh('Converging aether mote %02d'%i,sphere,(0,0,-100),(.001,)*3,gold if i%3 else cyan)
    start=19+i%13
    transform(a,[(0,(0,0,-100),(0,0,0),(.001,)*3),(start,(r*math.cos(ang),r*math.sin(ang),z),(0,0,0),(.001,)*3),(start+6,(r*math.cos(ang),r*math.sin(ang),z),(0,0,0),(.045,.045,.18)),(75,(35*math.cos(ang+2),35*math.sin(ang+2),260),(0,0,0),(.03,.03,.12)),(82,(0,0,260),(0,0,0),(.001,)*3),(END,(0,0,0),(0,0,0),(.001,)*3)])

# Impact shockwaves are independently editable mesh tracks; timing is in sequence frames.
for j,mat in enumerate((white,gold,cyan)):
    a=mesh('Expanding shockwave %d'%j,ring,(0,0,12+j*4),(.001,)*3,mat)
    start=81+j*4
    animate_scale(a,[(0,.001),(start-1,.001),(start,.25),(start+12,6),(start+25,11.5),(start+30,.001),(END,.001)],(0,0,12+j*4))
beam=mesh('Heavenfall pillar',cylinder,(0,0,900),(.001,)*3,white)
animate_scale(beam,[(0,(.001,)*3),(78,(.001,)*3),(81,(.22,.22,22)),(85,(.95,.95,22)),(89,(.1,.1,22)),(94,(.001,)*3),(END,(.001,)*3)],(0,0,900))
for i in range(36):
    ang=i*math.tau/36
    a=mesh('Impact streak %02d'%i,cube,(0,0,-100),(.001,)*3,gold if i%2 else cyan)
    rot=(0,math.degrees(ang),0)
    transform(a,[(0,(0,0,-100),rot,(.001,)*3),(81,(40*math.cos(ang),40*math.sin(ang),20),rot,(.001,)*3),(84,(250*math.cos(ang),250*math.sin(ang),30+i%4*16),rot,(3.2,.022,.022)),(96,(850*math.cos(ang),850*math.sin(ang),70+i%4*26),rot,(1.6,.012,.012)),(109,(1050*math.cos(ang),1050*math.sin(ang),85),rot,(.001,)*3),(END,(0,0,-100),rot,(.001,)*3)])

# Reuse the actual project's ultimate Niagara system.
n=spawn(u.NiagaraActor,'PROJECT Niagara - NS_Skill_Ultimate',(0,0,10))
nc=n.get_component_by_class(u.NiagaraComponent)
nc.set_asset(u.load_asset('/Game/Character/Player/Abilities/Niagaraes/Ultimate/NS_Skill_Ultimate'))
nc.set_editor_property('auto_activate',False)
n.set_actor_scale3d(u.Vector(1.4,1.4,1.4))
nb=binding(nc)
ns=nb.add_track(u.MovieSceneNiagaraSystemTrack).add_section()
ns.set_range(82,171)
ns.set_age_update_mode(u.NiagaraAgeUpdateMode.DESIRED_AGE)

camera=spawn(u.CineCameraActor,'Awakening cinematic camera',(1150,-1400,1250))
cc=camera.get_cine_camera_component()
cc.set_editor_property('current_focal_length',30)
focus=cc.get_editor_property('focus_settings')
focus.focus_method=u.CameraFocusMethod.DISABLE
cc.set_editor_property('focus_settings',focus)
cc.set_editor_property('current_aperture',5.6)
camera_keys=[]
for fr,p,target in [(0,(1150,-1400,1250),(0,0,80)),(18,(1030,-1250,1050),(0,0,100)),(19,(320,-400,200),(0,0,130)),(43,(370,-340,245),(0,0,170)),(74,(420,-260,390),(0,0,285)),(77,(1150,-1450,1350),(0,0,90)),(81,(1080,-1360,1260),(0,0,80)),(83,(1105,-1320,1285),(0,0,75)),(85,(1065,-1375,1230),(0,0,90)),(88,(1100,-1350,1250),(0,0,80)),(103,(1220,-1510,1410),(0,0,80)),(142,(1180,-1450,1300),(0,0,75)),(191,(1100,-1350,1200),(0,0,70))]:
    r=u.MathLibrary.find_look_at_rotation(u.Vector(*p),u.Vector(*target))
    camera_keys.append((fr,p,(r.pitch,r.yaw,r.roll),(1,1,1)))
cb=transform(camera,camera_keys)
cuts=seq.add_track(u.MovieSceneCameraCutTrack)
cut=cuts.add_section()
cut.set_range(0,END)
camera_id=u.MovieSceneObjectBindingID()
camera_id.set_editor_property('guid',cb.get_id())
cut.set_camera_binding_id(camera_id)

# A persistent sequence actor lets the user inspect and replay the shot in the demo map.
sa=spawn(u.LevelSequenceActor,'PLAY - LS_AwakeningDemo')
sa.set_sequence(seq)
play=sa.get_editor_property('playback_settings')
play.auto_play=False
sa.set_editor_property('playback_settings',play)
u.EditorAssetLibrary.save_loaded_asset(seq)
assert u.EditorLoadingAndSavingUtils.save_map(world,ROOT+'/L_AwakeningDemo')
assert seq.locate_bound_objects(cb,world), 'Cinematic camera binding must resolve before rendering'

config=asset('MRQ_AwakeningDemo',u.MoviePipelinePrimaryConfig,u.MoviePipelinePrimaryConfigFactory())
output=config.find_or_add_setting_by_class(u.MoviePipelineOutputSetting)
output.output_directory=u.DirectoryPath(os.path.join(OUT,'Frames').replace('\\','/'))
output.output_resolution=u.IntPoint(1280,720)
output.file_name_format='Awakening.{frame_number}'
output.zero_pad_frame_numbers=4
output.override_existing_output=True
config.find_or_add_setting_by_class(u.MoviePipelineDeferredPassBase)
config.find_or_add_setting_by_class(u.MoviePipelineImageSequenceOutput_PNG)
config.find_or_add_setting_by_class(u.MoviePipelineMP4EncoderOutput).set_editor_property('include_audio',False)
aa=config.find_or_add_setting_by_class(u.MoviePipelineAntiAliasingSetting)
aa.spatial_sample_count=1
aa.temporal_sample_count=1
aa.engine_warm_up_count=24
game=config.find_or_add_setting_by_class(u.MoviePipelineGameOverrideSetting)
game.soft_game_mode_override=u.GameModeBase
game.cinematic_quality_settings=False
cv=config.find_or_add_setting_by_class(u.MoviePipelineConsoleVariableSetting)
for name,value in {'r.ScreenPercentage':100.0,'r.MotionBlurQuality':2.0,'r.Streaming.PoolSize':1500.0}.items():
    cv.add_or_update_console_variable(name,value)
u.EditorAssetLibrary.save_loaded_asset(config)
u.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
unreal_report={'map':ROOT+'/L_AwakeningDemo','sequence':seq.get_path_name(),'render_preset':config.get_path_name(),'duration_seconds':END/FPS,'fps':FPS,'impact_frame':82,'notes':'Presentation-only feasibility demo using the current player mesh and project Niagara. No gameplay damage/input/cooldown integration.'}
with open(os.path.join(OUT,'demo_manifest.json'),'w') as f:
    json.dump(unreal_report,f,indent=2)
u.log('AWAKENING_BUILD_COMPLETE')
