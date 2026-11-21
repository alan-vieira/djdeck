
from PIL import Image
from collections import Counter
p=r"C:/Users/alsvi/AppData/Local/hermes/cache/documents/native-inputs/73a7a38804c425bbd5e07495edc0b0bb9cacff48eea0c928f273dcd71b175173/dashboard_20261120_190707_9c6a461b_image_0b4599.png"
im=Image.open(p).convert("RGB")
w,h=im.size
print("size",w,h)
for y in [0,1,2,3,4,10,20,30,37,38,39,40]:
    runs=[]; last=None; start=0
    for x in range(w):
        c=im.getpixel((x,y))
        if c!=last:
            if last is not None: runs.append((start,x-1,last))
            start=x; last=c
    runs.append((start,w-1,last))
    notable=[r for r in runs if r[1]-r[0]>=2 or r[2]!=(255,255,255)]
    print("y",y,notable[:30])
# exact dominant gray grid columns: count (224,224,224) by x
cnt=Counter()
for y in range(h):
  for x in range(w):
    if im.getpixel((x,y))==(224,224,224): cnt[x]+=1
print("gray columns",cnt.most_common(30))
# region x28:64 per-row dominant colors / dark bounds
for y in range(h):
  dark=[x for x in range(20,70) if max(im.getpixel((x,y)))<80]
  if dark: print("square dark row",y,min(dark),max(dark),len(dark))
