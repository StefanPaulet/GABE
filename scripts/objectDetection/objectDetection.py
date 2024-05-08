import sys
import numpy as np

from roboflow import Roboflow
import supervision as sv


def read_image():
    byte_data = sys.stdin.buffer.read(640 * 640 * 3)
    image_array = np.frombuffer(byte_data, dtype=np.uint8)
    image_array = image_array.reshape((640, 640, 3))
    return image_array

data = sys.stdin.readline().strip()
response = "response:" + data
sys.stdout.write(response)
sys.stdout.flush()


rf = Roboflow(api_key="xL0E5GuDc339bLtw3p1A")
project = rf.workspace().project("csgo-2-hfa81")
model = project.version(1).model

sys.stdout.write("**finishedsetup**")
sys.stdout.flush()

while True:
    data = read_image()
    result = model.predict(data, confidence=40, overlap=30).json()

    detections = sv.Detections.from_inference(result)
    sys.stdout.write(str(detections.xyxy))
    sys.stdout.flush()