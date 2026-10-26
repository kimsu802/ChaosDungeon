"""Encode a looping GIF from actual Unreal-rendered frames and validate the render."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageStat
import hashlib, json, shutil

root=Path(__file__).resolve().parents[2]
output=root/'Saved'/'AwakeningDemo'
frames=sorted((output/'Frames').glob('Awakening.*.png'))
assert len(frames)==192, f'Expected 192 frames, got {len(frames)}'
assert (output/'render_result.txt').read_text()=='SUCCESS'
means=[]
hashes=set()
for path in frames:
    with Image.open(path) as image:
        assert image.size==(1280,720)
        means.append(sum(ImageStat.Stat(image.convert('RGB')).mean)/3)
        hashes.add(hashlib.sha256(image.tobytes()).hexdigest())
assert max(means)>12, 'Rendered movie must contain a visible scene'
assert len(hashes)>150, 'Animation frames must change over time'

clips=sorted((output/'Frames').glob('*.mp4'),key=lambda p:p.stat().st_mtime)
assert clips, 'Native Unreal MP4 output missing'
shutil.copy2(clips[-1],output/'ChaosDungeon_Awakening.mp4')

samples=[0,35,65,83,96,155]
sheet=Image.new('RGB',(1152,480),(8,13,22))
draw=ImageDraw.Draw(sheet)
for i,index in enumerate(samples):
    with Image.open(frames[index]) as im:
        im=im.convert('RGB').resize((384,216),Image.Resampling.LANCZOS)
        x,y=(i%3)*384,(i//3)*240
        sheet.paste(im,(x,y))
        draw.text((x+10,y+220),f'{index/24:.2f}s  |  UNREAL ENGINE RENDER',fill=(170,190,210))
sheet.save(output/'Awakening_ContactSheet.jpg',quality=94)

palette=sheet.quantize(colors=128,method=Image.Quantize.MEDIANCUT)
gif=[]
for path in frames[::2]:
    with Image.open(path) as im:
        im=im.convert('RGB').resize((768,432),Image.Resampling.LANCZOS)
        gif.append(im.quantize(palette=palette,dither=Image.Dither.NONE))
gif[0].save(output/'ChaosDungeon_Awakening.gif',save_all=True,append_images=gif[1:],duration=[80,80,90]*32,loop=0,optimize=False,disposal=2)
summary={'source':'Unreal Engine Movie Render Queue, project player assets','frame_count':len(frames),'fps':24,'duration_seconds':8,'size':[1280,720],'unique_frame_count':len(hashes),'min_mean_brightness':min(means),'max_mean_brightness':max(means),'gif_frame_count':96,'gif_duration_ms':8000,'mp4_bytes':(output/'ChaosDungeon_Awakening.mp4').stat().st_size,'gif_bytes':(output/'ChaosDungeon_Awakening.gif').stat().st_size}
(output/'media_validation.json').write_text(json.dumps(summary,indent=2))
print(json.dumps(summary,indent=2))
