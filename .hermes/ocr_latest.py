
from PIL import Image
from rapidocr_onnxruntime import RapidOCR
p=r"C:/Users/alsvi/AppData/Local/hermes/cache/documents/native-inputs/5dc842cdd991f357769788bac3908a86adaef6b0d28a586c6013f15047cf6aa6/dashboard_20261120_192049_ababc397_image_542ba0.png"
im=Image.open(p)
print("IMAGE", im.size, im.mode)
engine=RapidOCR()
result, elapsed=engine(p)
print("ELAPSED", elapsed)
if result:
  for box,text,score in result:
    xs=[int(v[0]) for v in box]; ys=[int(v[1]) for v in box]
    print(f"{min(xs):4},{min(ys):4} {max(xs):4},{max(ys):4} {score:.3f} {text}")
else:
  print("NO_OCR")
