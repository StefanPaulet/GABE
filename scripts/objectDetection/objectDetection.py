import time

import numpy as np
import sys
from ultralytics import YOLO
import cv2



model = YOLO("../scripts/objectDetection/weights.pt")

def read_image():
    imageWidth = 1920
    imageHeight = 1080 - 64
    totalImageSize = imageWidth * imageHeight * 3
    byte_data = sys.stdin.buffer.read(totalImageSize)
    image_array = np.frombuffer(byte_data, dtype=np.uint8)
    image_array = image_array.reshape((imageHeight, imageWidth, 3))
    return image_array

data = sys.stdin.readline().strip()
response = "response:" + data
sys.stdout.write(response)
sys.stdout.flush()

time.sleep(1)

sys.stdout.write("**finishedsetup**")
sys.stdout.flush()

while True:
    image = read_image()
    results = model(cv2.cvtColor(image, cv2.COLOR_RGB2BGR), conf=0.7, max_det=5, verbose=False)
    boxes = []
    for result in results:
        for box in result.boxes:
            x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
            x1 = int(x1)
            x2 = int(x2)
            y1 = int(y1)
            y2 = int(y2)
            boxes.append([x1, y1, x2, y2])
    sys.stdout.write(str(boxes))
    sys.stdout.flush()
