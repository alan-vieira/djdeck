
from PIL import Image
from collections import Counter
p=r"C:/Users/alsvi/AppData/Local/hermes/cache/images/computer_use_db4327c805f34e8a94ef5bfbdef0a13c.png"
im=Image.open(p).convert("RGB")
w,h=im.size
print("size",w,h)
target=(21,96,130)
for deck_x in (0,781):
    pts=[]
    for y in range(130,190):
      for x in range(deck_x,deck_x+120):
        if im.getpixel((x,y))==target: pts.append((x,y))
    if pts:
      print("artwork",deck_x,min(x for x,y in pts),min(y for x,y in pts),max(x for x,y in pts),max(y for x,y in pts),len(pts))
    else: print("artwork",deck_x,"not found")
# OCR from fresh release screenshot
from rapidocr_onnxruntime import RapidOCR
res,_=RapidOCR()(p)
for box,text,score in (res or []):
  if text in {"SYNC","MASTER"} or "Freed From Desire" in text or "It" in text:
    xs=[int(v[0]) for v in box]; ys=[int(v[1]) for v in box]
    print(f"{min(xs)},{min(ys)} {max(xs)},{max(ys)} {text}")
