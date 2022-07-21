import numpy as np
import math
import cv2
from matplotlib import pyplot as plt


def get_angles_rads(rotations: int, from_rads: float, to_rads: float):
    return np.linspace(from_rads, to_rads, rotations)


def generate(rotations: int, from_rads: float, to_rads: float) -> np.ndarray:
    angles_rads = get_angles_rads(rotations, from_rads, to_rads)
    sins = np.sin(angles_rads)
    coss = np.cos(angles_rads)
    return sins, coss


def generate_and_print_rotations():
    sins, coss = generate(NUM_ROTATIONS, ANGLE_RADS_FROM, ANGLE_RADS_TO)

    for rot in range(NUM_ROTATIONS):
        print(f"{{ {coss[rot]}, {sins[rot]} }},")


def get_diagonal_size(height, width):
    diagonal = math.ceil(math.sqrt(height ** 2 + width ** 2))
    # Make the diagonal even
    if (diagonal % 2) != 0:
        diagonal += 1
    return diagonal


def pad_image(image: np.ndarray) -> np.ndarray:
    img_width = image.shape[1]
    img_height = image.shape[0]

    # Pad the image before rotation, so that the whole image is visible
    diagonal = get_diagonal_size(img_height, img_width)
    vertical_pad = diagonal - img_height
    vertical_pad_half = vertical_pad // 2
    horizontal_pad = diagonal - img_width
    horizontal_pad_half = horizontal_pad // 2

    pad_width = [[vertical_pad_half, vertical_pad_half],
                 [horizontal_pad_half, horizontal_pad_half]]
    padded = np.pad(image, pad_width, constant_values=0)
    return padded


def rotate_image(image: np.ndarray, angle_degs: float) -> np.ndarray:
    # Rotate around the centre of the image
    (h, w) = image.shape[:2]
    (cX, cY) = (w // 2, h // 2)

    M = cv2.getRotationMatrix2D((cX, cY), angle_degs, 1.0)
    rotated_image = cv2.warpAffine(image, M, (w, h))

    return rotated_image


def print_col_sum(col_sum: np.ndarray) -> None:
    col_sum_list = col_sum.astype(str).tolist()
    line = ", ".join(col_sum_list)
    print("{ ", line, " },")


def print_nonzero_pixels(width, height) -> None:
    angles_rads = get_angles_rads(NUM_ROTATIONS, ANGLE_RADS_FROM, ANGLE_RADS_TO)
    angles_degs = angles_rads / np.pi * 180
    acc_size = get_diagonal_size(height, width)

    with open("src/scripts/columnPixelCounts.dat", 'wb') as file:
        # Write height and width of the image (determining the number of values)
        num_rotations_bytes = np.array([NUM_ROTATIONS]).astype(np.uint16).tobytes()
        acc_size_bytes = np.array([acc_size]).astype(np.uint16).tobytes()
        file.write(num_rotations_bytes)
        file.write(acc_size_bytes)

        # Write all the values
        for i in range(NUM_ROTATIONS):
            img = np.ones([height, width], dtype=np.uint8)
            padded_img = pad_image(img)
            rotated_img = rotate_image(padded_img, angles_degs[i])

            # Each value can be represented in 16 bits.
            col_sum = np.sum(rotated_img, axis=0).astype(np.uint16)
            # Avoiding 0 values (comes in handy when normalizing by the number of pixels)
            col_sum[col_sum == 0] = 1
            assert (col_sum.size == acc_size)
            col_sum_bytes = col_sum.tobytes()
            file.write(col_sum_bytes)


def resize_image(img_path: str, save_path: str, width: int, height: int) -> None:
    img = cv2.imread(img_path)
    resized = cv2.resize(img, (width, height), interpolation=cv2.INTER_AREA)
    cv2.imwrite(save_path, resized)


NUM_ROTATIONS = 180
ANGLE_RADS_FROM = -0 / 180 * np.pi
ANGLE_RADS_TO = 79 / 180 * np.pi
WIDTH = 1920
HEIGHT = 1080

# resize_image('data/olympus/_6210070.JPG', 'data/olympus/_6210070_fullhd.JPG', WIDTH, HEIGHT);
generate_and_print_rotations()
print_nonzero_pixels(width=WIDTH, height=HEIGHT)
