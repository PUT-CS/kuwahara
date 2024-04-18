# Kuwahara Filter
This is a C++/CUDA/OpenMP implementation of the [Kuwahara filter](https://en.wikipedia.org/wiki/Kuwahara_filter)

# Implementations
There are three branches with three separate implementations of the same algorithm. 
This project aims to measure execution time differences between sequential and parallel code, including GPU parellelization.

# Usage
## Compile
```bash
cmake .
make
```
## Run
```bash
kuwahara [INPUT_IMAGE_PATH] [OUTPUT_IMAGE_PATH]
```

To run:
# Showcase
Numbers indicate the window dimensions (it's always square for now)
From the left: Original, 5, 13, 19

<p align="middle">
  <img src="img/lena.jpg" width="100" />
  <img src="img/lena5.jpg" width="100" />
  <img src="img/lena9.jpg" width="100" />
  <img src="img/lena13.jpg" width="100" />
  <img src="img/lena17.jpg" width="100" />
</p>

# Measurements
**TODO**

# Author
Michał Miłek
