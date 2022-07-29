import cv2
import sys

if len(sys.argv) == 1:
    raise RuntimeError("No argument - image file path is missing")

filename = sys.argv[1]

bytes = cv2.imread(filename).tobytes()

sys.stdout.buffer.write(bytes)
