
from PIL import Image
p=r"C:/Users/alsvi/AppData/Local/hermes/cache/documents/native-inputs/73a7a38804c425bbd5e07495edc0b0bb9cacff48eea0c928f273dcd71b175173/dashboard_20261120_190707_9c6a461b_image_0b4599.png"
im=Image.open(p).convert("RGB")
for y in range(0,41,2):
  s=''
  for x in range(24,68,1):
    r,g,b=im.getpixel((x,y))
    if r<20 and g<55 and b<70: ch='#'
    elif b>110 and g>70 and r<60: ch='T'
    elif max(r,g,b)>245: ch=' '
    elif max(r,g,b)<100: ch='x'
    else: ch='.'
    s+=ch
  print(f'{y:02d} {s}')
