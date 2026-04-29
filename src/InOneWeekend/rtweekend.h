#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>

using std::make_shared;
using std::shared_ptr;

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

#pragma acc routine seq
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

// Single LCG RNG — works on both host and GPU.
// Host uses a global seed; GPU uses a per-thread seed passed by reference.

static unsigned int host_seed = 12345u;

#pragma acc routine seq
inline double random_double_gpu(unsigned int& seed) {
    seed = seed * 1664525u + 1013904223u;
    return (seed >> 1) / (double)0x7FFFFFFF;
}

#pragma acc routine seq
inline double random_double_gpu(unsigned int& seed, double min, double max) {
    return min + (max - min) * random_double_gpu(seed);
}

// Host wrappers — same LCG, use global host_seed
inline double random_double() {
    return random_double_gpu(host_seed);
}
inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif