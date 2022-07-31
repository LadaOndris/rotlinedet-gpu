    

# Compilation

Use cmake to compile. Makefile was used in the first stages of development to compile a single source file.

Use **g++** to compile for CPU.

Use **nvc++** to compile for GPU, increasing the algorithm's speed twice for Full HD images.

# Number of rotations

The number of rotations is a significant parameter, which determines the precision of the 
algorithm. More rotations means smaller rotational difference, thus higher precision.
Increasing the number of rotations linearly increases the processing time. It is advisory
to keep this parameter as small as possible while achieving sufficient accuracy.

The following steps must be made to change the number of rotations:
* Change the NUM_ROTATIONS variable in `rotations.py`
* Regenerate the `columnPixelCounts.dat`file by running the `rotations.py` script
* Copy generated array from stdout to `linedet.cpp` 
* Change the NUM_ROTATIONS in `linedet.hpp` and compile the source code

## Notes

Increasing number of rotations from 317 to 475 did not
improve accuracy. 317 rotations results in a step of a half a degree.