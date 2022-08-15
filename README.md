    

## Compilation

Use cmake to compile. Makefile was used in the first stages of development to compile a single source file.

Use **g++** to compile for CPU.

Use **nvc++** to compile for GPU, increasing the algorithm's speed twice for Full HD images.

## Execution

Run `rotlinedet_run` executable after compilation. The image is read from the standard input
so that images can be pipelined to this program real-time without saving them to a disk.

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
    File path to a file with saved pixel counts in columns of a rotated image. Generation of this file is discussed further.
* --candidates
    Number of candidate lines the program should produce.

Optimal parameters can be determined using `bestparams.py` script in **apaler** repository.

## Number of rotations

The number of rotations is a significant parameter, which determines the precision of the 
algorithm. More rotations means smaller rotational difference, thus higher precision.
Increasing the number of rotations linearly increases the processing time. It is advisory
to keep this parameter as small as possible while achieving sufficient accuracy.

The following steps must be made to change the number of rotations:
* Change parameters (ANGLE_RADS_FROM, ANGLE_RADS_TO, ANGLE_STEP) in `rotations.py`
* Regenerate the `columnPixelCounts.dat`file by running the `rotations.py` script
* Copy generated array from stdout to `linedet.cpp` 
* Change the NUM_ROTATIONS in `linedet.hpp` and compile the source code

### Notes

* Increasing number of rotations from 317 to 475 did not
improve accuracy. 
* 317 rotations result in a step of a half a degree.

## Tests

Tests are written using the **GoogleTest** library. The library should be downloaded 
into `google_tests/lib` directory.
