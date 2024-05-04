# Kuwahara Filter
This is a C++/CUDA/OpenMP implementation of the [Kuwahara filter](https://en.wikipedia.org/wiki/Kuwahara_filter)

# Implementations
There are three branches with three separate implementations of the same algorithm. 
This project aims to measure execution time differences between sequential and parallel code, including GPU parellelization.

# Usage
There's a dependency on **OpenCV**, so make sure it's installed.
For the CUDA branch, there's obviously a dependency on the Nvidia libraries and toolkits.
## Compile
```bash
cmake .
make
```
## Run
```bash
./kuwahara INPUT_IMAGE_PATH OUTPUT_IMAGE_PATH [--window WINDOW_SIZE]
```
# Examples

| Original Image | Radius 9 | Radius 17 |
|:--------------:|:---------------------------:|:----------------------------:|
| ![Original Lena](img/lena.jpg) | ![Filtered Lena (Radius=9)](img/lena9.jpg) | ![Filtered Lena (Radius=17)](img/lena17.jpg) |
| ![Original Morskie Oko](img/morskieOko.jpg) | ![Filtered Morskie Oko (Radius=9)](img/morskieOko9.jpg) | ![Filtered Morskie Oko (Radius=17)](img/morskieOko17.jpg) |
| ![Original Jez](img/jez.jpg) | ![Filtered Jez (Radius=9)](img/jez9.jpg) | ![Filtered Jez (Radius=17)](img/jez17.jpg) |
| ![Original NYC](img/nyc.jpg) | ![Filtered NYC (Radius=9)](img/nyc9.jpg) | ![Filtered NYC (Radius=17)](img/nyc17.jpg) |

# Performance Benchmarks
## Benchmarking Script
I've written a script to benchmark code located on different branches of the same repo - `branchbench.py`.
Perhaps one day I'll make it into its own project, but for now it's embedded into this one.
For the script to work your program's only output (to stdout) must be a string parsable into `float` by Python.

**GRAPH HERE**

# Author
Michał Miłek
