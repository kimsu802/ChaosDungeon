# 초각성 연출 가능성 검증

현재 ChaosDungeon 프로젝트의 실제 플레이어 모델과 애니메이션, 궁극기 Niagara를 사용하는 8초짜리 Unreal Engine 연출 시연입니다. AI 생성 영상이나 로스트아크 게임 화면을 합성한 영상이 아닙니다.

## 판단

구현 가능합니다. 이미 다음 기반이 존재합니다.

- `CDSkillAbility`: GAS 시전, 쿨타임, 타격 및 GameplayCue 처리.
- `CDSkillData`: 몽타주, 타격 도형, 계수, 넉백 및 효과 목록.
- `CDPlayerCharacter`: 쿼터뷰 카메라와 SpringArm.
- 현재 `BP_CDCharacter`의 `Greystone_Dragonlord` 모델.
- `Ultimate_Entry`, `Ultimate_Part1`, `Ultimate_Part2` 애니메이션과 `NS_Skill_Ultimate` Niagara.

따라서 시전 동작, 카메라 전환, 충전, 빛기둥, 충격파를 동기화하는 방식의 초각성풍 연출을 만들 수 있습니다. 로스트아크의 특정 직업 기술을 그대로 재현하는 작업은 별도의 전용 애니메이션, VFX, 사운드 제작이 필요합니다.

## 이번 시연 범위

- 0.0–0.8초: 전투 시점에서 시전 시작.
- 0.8–3.2초: 근접 카메라, 공중 부유, 에너지 입자 수렴, 다중 문양.
- 약 3.4초: 착지와 프로젝트 궁극기 Niagara, 빛기둥, 방사형 섬광, 세 겹 충격파.
- 4.5–8.0초: 잔광이 사라지며 쿼터뷰로 복귀.

신규 검증용 아레나에서 촬영합니다. 기존 던전 맵은 변경하지 않습니다. 시연은 Sequencer로 연출을 검증하며 입력, 피해, 쿨타임, 무적, 멀티플레이까지 연결한 완성 스킬은 아닙니다. 오디오는 포함하지 않습니다.

## 에디터에서 확인

1. `/Game/AwakeningDemo/L_AwakeningDemo` 맵을 엽니다.
2. `/Game/AwakeningDemo/LS_AwakeningDemo`를 엽니다.
3. Sequencer의 카메라 컷 보기를 켜고 재생합니다.

연출을 구성하는 모든 카메라/캐릭터/충격파 트랙은 시퀀스에서 수정할 수 있습니다. 렌더 프리셋은 `/Game/AwakeningDemo/MRQ_AwakeningDemo`입니다.

## 산출물

`Saved/AwakeningDemo/`에 렌더 프레임, MP4, GIF, 에셋 확인 보고서와 렌더 결과를 저장합니다. MP4는 1280×720 / 24fps / 8초이며, GIF는 채팅 미리보기를 위해 768×432 / 12fps로 축소합니다. 생성 스크립트와 재현 절차는 Git에 남고, Saved 폴더의 영상은 프로젝트의 기존 ignore 정책을 따릅니다.

## 실제 스킬로 연결할 때

1. `UCDSkillAbility` 자식 능력과 전용 데이터 에셋을 추가합니다. 기존 구조가 스킬별 능력 클래스 교체를 지원합니다.
2. 시전 시 플레이어 입력 및 이동을 제어하고 로컬 카메라만 시퀀스로 전환합니다.
3. 피해 판정은 기존 `Event.Montage.Hit`/GAS 흐름에 연결합니다. 피해를 카메라나 영상 프레임에 종속시키지 않습니다.
4. 중단, 사망, 레벨 전환, 정상 종료 시 카메라, 입력, 무적 태그, VFX를 복원합니다.
5. 다른 플레이어에게는 전투 VFX를 전달하고 시전자 카메라만 컷신으로 처리합니다. 전체 월드 시간 감속은 별도 정책이 필요합니다.
6. VFX 오버드로와 GPU 비용을 실제 전투 밀집 상황에서 측정하고 사운드 및 전용 동작을 다듬습니다.

## 스크립트 실행

엔진 설치 경로는 이 환경의 UE 5.8 기준입니다. 명령줄의 플러그인 활성화는 실행 세션에만 적용하며 `.uproject`를 변경하지 않습니다.

```powershell
$engine = 'C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$project = 'C:/Users/user/Desktop/UnrealProject/ChaosDungeon/ChaosDungeon.uproject'
$scripts = 'C:/Users/user/Desktop/UnrealProject/ChaosDungeon/Scripts/AwakeningDemo'
& $engine $project '-run=pythonscript' "-script=$scripts/inspect_assets.py" '-EnablePlugins=PythonScriptPlugin,EditorScriptingUtilities,SequencerScripting,MovieRenderPipeline' -unattended -NullRHI -nosound
& $engine $project '-run=pythonscript' "-script=$scripts/build_demo.py" '-EnablePlugins=PythonScriptPlugin,EditorScriptingUtilities,SequencerScripting,MovieRenderPipeline' -unattended -NullRHI -nosound
& $engine $project '/Game/AwakeningDemo/L_AwakeningDemo' "-ExecutePythonScript=$scripts/render_demo.py" '-EnablePlugins=PythonScriptPlugin,EditorScriptingUtilities,SequencerScripting,MovieRenderPipeline' -unattended -nosound -RenderOffscreen
# 렌더가 종료된 뒤 Pillow가 설치된 Python에서 실행합니다.
python "$scripts/export_media.py"
```

`build_demo.py`는 신규 데모 맵의 내용을 다시 생성합니다. 데모 맵을 직접 편집했다면 필요한 변경을 보관한 뒤 재실행해야 합니다.
