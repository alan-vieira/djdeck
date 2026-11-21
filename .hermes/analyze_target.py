
from PIL import Image
from collections import Counter
p=r"C:/Users/alsvi/AppData/Local/hermes/cache/documents/native-inputs/73a7a38804c425bbd5e07495edc0b0bb9cacff48eea0c928f273dcd71b175173/dashboard_20261120_190707_9c6a461b_image_0b4599.png"
im=Image.open(p).convert("RGB")
w,h=im.size
print("size",w,h)
# dominant colors
print("dominant", Counter(im.getdata()).most_common(12))
# bounds of teal-ish pixels (B/R/G relationships)
pts=[]
for y in range(h):
  for x in range(w):
    r,g,b=im.getpixel((x,y))
    if b>70 and g>45 and r<80 and (g+b)>r+90:
      pts.append((x,y))
print("teal bounds", min(x for x,y in pts), min(y for x,y in pts), max(x for x,y in pts), max(y for x,y in pts), "count",len(pts))
# dark-pixel bounds in square crop
for name,box in [('square',(20,0,75,h)),('right',(430,0,w,h))]:
  q=[]
  for y in range(box[1],box[3]):
    for x in range(box[0],box[2]):
      r,g,b=im.getpixel((x,y))
      if max(r,g,b)<100: q.append((x,y))
  print(name, (min(x for x,y in q),min(y for x,y in q),max(x for x,y in q),max(y for x,y in q),len(q)) if q else None)
