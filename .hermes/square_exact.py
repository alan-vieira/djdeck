
from PIL import Image
p=r"C:/Users/alsvi/AppData/Local/hermes/cache/documents/native-inputs/73a7a38804c425bbd5e07495edc0b0bb9cacff48eea0c928f273dcd71b175173/dashboard_20261120_190707_9c6a461b_image_0b4599.png"
im=Image.open(p).convert("RGB")
for y in range(2,38):
  s=''
  for x in range(28,64):
    r,g,b=im.getpixel((x,y))
    if r<12 and g<60 and b<75: ch='#'
    elif b>110 and g>70 and r<60: ch=' '
    else: ch='.'
    s+=ch
  print(f'{y:02d}|{s}|')
