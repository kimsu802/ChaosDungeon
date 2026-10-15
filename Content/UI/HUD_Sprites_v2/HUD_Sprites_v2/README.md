# 고딕 판타지 RPG HUD 스프라이트 v2

모든 파일은 투명 PNG(RGBA)입니다. 투명 픽셀의 RGB는 주변 색으로 채워 두었습니다(필터링 시 헤일로 방지).

## 폴더 구성

| 폴더 | 내용 |
|---|---|
| 01_base | 시트에서 분리한 원본 요소 95개. 원본 좌표는 `sprites_base_coords.json` |
| 02_no_text | 박힌 텍스트(2/5, 12,480, Lv. 50, 키 라벨)를 지운 빈 버전 |
| 03_bar_frame_fill | 바를 빈 프레임(`frame_empty`)과 채움(`fill_*`)으로 분리. `fill_white_tintable`은 엔진에서 색을 입히는 흰색 버전 |
| 04_color_variants | 채움/완성 바의 red·gold·green·blue·purple 색상 변형 |
| 05_slices | 바·패널을 늘릴 수 있게 자른 파츠와 `slice.json`(마진 값) |
| 06_button_states | 버튼 normal / hover / pressed / disabled. 네 상태 모두 캔버스 크기와 피벗이 같음(사방 8px 여백) |
| 07_flips_rotations | 코너 장식 4방향, 화살표 상하좌우, 세로 장식의 가로 버전 |
| 08_skill_icons | 스킬바에서 뽑은 아이콘 8종(프레임 없는 62px 원본 + 128px 업스케일) |
| 09_generated | 코드로 만든 보조 텍스처(쿨다운, 글로우, 그라데이션, 하이라이트, 마스크) |

## 언리얼(UMG) 사용 팁

- **텍스처 설정**: Texture Group `UI`, Mip Gen `NoMipmaps`, Compression `UserInterface2D (RGBA)`.
- **늘어나는 바/패널**: `05_slices/<이름>/slice.json`의 `umg_box_margin` 값을 Image 브러시의 Draw As = **Box**, Margin에 그대로 넣으면 됩니다(3-slice, 9-slice). 
  - `hp_bar_large_frame_empty`, `hp_bar_frame_small`은 가운데 보석이 있는 **5-slice**라 Box 마진 하나로는 안 됩니다. 파츠(fixed/stretch)를 HorizontalBox에 배치하고, stretch 파츠만 Fill로 두세요.
  - 끝이 투명하게 사라지는 패널(`panel_bar_gold_gem`, `blue_gem`, `red`, `dark_03` 등)은 페이드 구간을 오른쪽 고정 캡에 넣어 두었습니다.
- **HP/MP 바**: ProgressBar의 Background Image에 `frame_empty`, Fill Image에 `fill_*`. 색을 런타임에 바꾸려면 `fill_white_tintable` + Fill Color and Opacity.
- **버튼**: Button 스타일의 Normal / Hovered / Pressed / Disabled에 `06_button_states`의 같은 이름 4장을 각각 넣으세요.
- **스킬 쿨다운**: `cooldown_radial_gradient_128`을 머티리얼에서 `step(Progress, Texture.R)`처럼 비교하면 12시 방향부터 시계방향으로 쓸리는 마스크가 됩니다. 아이콘 위에 `cooldown_dim_overlay_64`를 그 마스크로 덮으면 됩니다.
- **슬롯 선택 표시**: `slot_highlight_gold_96`을 스킬 슬롯 위에 겹쳐 쓰세요.
