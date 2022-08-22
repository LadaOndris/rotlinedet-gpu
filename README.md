# rotlinedet-gpu

The repository contains an implementation of the rotlinedet algorithm in C++
optimized for GPU using OpenACC. The algorithm finds lines in the given image by rotating it by 
many angles and finding drastic changes in intensity of its columns.

## Compilation

Use cmake to compile. 

Use **g++** to compile for CPU.

Use **nvc++** to compile for GPU for significant speed up. Install NVIDIA HPC SDK.

## Execution

Run `rotlinedet_run` executable after compilation. The image is read from the standard input
so that images can be pipelined to this program real-time without saving them to a disk.

The output is printed to standard output. Each line in the image (a laser candidate) is represented 
by two points on the image in the following format `{x1},{y1},{x2},{y2}\n`.

The program supports the following arguments:
* --filterSize 
    The width of the running average window. A typical width is around 30. The best value depends
    on the image resolution and laser line width.
* --slopeThreshold
    Only spikes at local extremes are considered (slope == 0). However, the slope is not precisely 0
    but usually very close to 0. A typical threshold is 0.1 or 0.2.
* --minPixelsThreshold
    The minimum number of pixels in a column that can be considered statistically significant to
    say that there is a spike in that column. Columns with lower number of pixels than this threshold
    are ignored. A typical value is 200 or 300.
* --pixelCountFile
    File path to a file with saved pixel counts in columns of a rotated image. 
    Generation of this file is discussed further.
* --candidates
    Number of candidate lines the program should produce.

Optimal parameters can be determined using `bestparams.py` script in **apaler** repository.

### One-time Execution
The image is read from the standard input:
```
python3 loadimg.py ./data/olympus/_6210070_fullhd.JPG | ./src/rotlinedet_run --filterSize 30 --slopeThreshold 0.2 --minPixelsThreshold 300 --candidates 5
```
The candidates are written to the standard output:
```
0,804,1107,0
22,0,1101,1079
1919,580,1497,0
1884,0,805,1079
0,831,507,1079
```

## Number of rotations

The number of rotations is a significant parameter, which determines the precision of the 
algorithm. More rotations means smaller rotational difference, thus higher precision.
Increasing the number of rotations linearly increases the processing time. It is advisory
to keep this parameter as small as possible while achieving sufficient accuracy.

The following steps must be made to change the number of rotations:
* Change parameters (ANGLE_RADS_FROM, ANGLE_RADS_TO, ANGLE_STEP) in `src/scripts/rotations.py`
* Regenerate the `src/scripts/columnPixelCounts.dat`file by running the `src/scripts/rotations.py` script
* Copy generated array from stdout to `linedet.cpp` 
* Change the NUM_ROTATIONS in `linedet.hpp` and compile the source code

## Algorithm speed

Speed was evaluated on Full HD and 4K image. 
Only the most computationally intensive part of the algorithm—the `sumColumns` function—was evaluated.
The other parts of the algorithm are too insignificant to consider for optimization. 

Evaluation used 180 rotations. Note that the current implementation uses 281 rotations. The speed 
scales linearly with the increasing number of rotations.

**1920x1080 - Full HD**  
g++ 		1950 ms  
g++ -O3 	600 ms  
GPU 		440 ms  

**3840x2160 - 4K**  
g++ 		7900 ms  
g++ -O3 	2390 ms  
GPU 		895 ms  

## Algorithm accuracy

The algorithm can be evaluated using the `evaluate.py` script in **apaler** repository.
The evaluation produced 143/250 successful detections (57.2%). The script produces statistics 
per intensity, per laser width, per laser length, and per image.

An important finding emerged from per image statistics:
```
image
0   1  2  3 4
10 40 17 37 3
```
The first row shows image index and below that index on the second line is the number of failed
detections out of 50. Detection accuracies for these images are then 80%, 20%, 66%, 26% and 94%.

The finding is that for some background the algorithm works better than for other background. 
After careful exploration of the images, it is clear that the detection accuracy depends on the complexity
of the image background (or rather foreground). If there are trees whose peaks cover part of the 
lighter background, then the intensities of the columns are jagged.
Possible solution: image could be divided into quadrants in which the line would be detected separately.

### Notes

* 317 rotations result in a step of a half a degree.
* Increasing number of rotations from 317 to 475 did not
  improve accuracy.

## Tests

Tests are written using the **GoogleTest** library. The library should be downloaded 
into `google_tests/lib` directory.

## To do

* Modify the algorithm to accept a continuous stream of images instead of a single image
  (no memory allocations will be required for the processing of each image)
* Strong single point light sources may cause drastic changes in intensity. Thus, consider incorporating
replacement of extreme values (too far away from median) with the median.
