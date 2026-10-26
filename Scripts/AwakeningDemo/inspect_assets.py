import unreal, json, os

root = unreal.Paths.project_dir()
report = {}
bp = unreal.load_object(None, '/Game/Character/Player/BP_CDCharacter.BP_CDCharacter_C')
if bp:
    cdo = unreal.get_default_object(bp)
    mesh = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
    report['player_mesh'] = mesh.get_editor_property('skeletal_mesh_asset').get_path_name()
    report['player_materials'] = [m.get_path_name() if m else None for m in mesh.get_materials()]
    report['mesh_transform'] = str(mesh.get_relative_transform())
for path in ['/Game/Character/Player/Abilities/Niagaraes/Ultimate', '/Game/Character/Player/ParagonGreystone/Characters/Heroes/Greystone/Animations']:
    entries=[]
    for p in unreal.EditorAssetLibrary.list_assets(path, recursive=True):
        obj=unreal.load_asset(p)
        item={'path':p,'class':obj.get_class().get_name() if obj else None}
        if isinstance(obj, unreal.AnimSequence):
            item['length']=obj.get_editor_property('sequence_length')
        entries.append(item)
    report[path]=entries
report['APIs']={name:hasattr(unreal,name) for name in ['MoviePipelinePIEExecutor','MoviePipelineQueueSubsystem','MoviePipelineMP4EncoderOutput','MovieSceneNiagaraSystemTrack','MovieSceneParticleTrack']}
os.makedirs(os.path.join(root,'Saved','AwakeningDemo'),exist_ok=True)
with open(os.path.join(root,'Saved','AwakeningDemo','asset_report.json'),'w') as f:
    json.dump(report,f,indent=2)
unreal.log('AWAKENING_INSPECTION_COMPLETE')
