#ifndef RTWEEKEND_STB_IMAGE_H
#  define RTWEEKEND_STB_IMAGE_H

// Disable pedantic warnings for this external library.
#  ifdef _MSC_VER
    // Microsoft Visual C++ Compiler
#    pragma warning (push, 0)
#  endif

#  pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wmisleading-indentation"

#    define STB_IMAGE_IMPLEMENTATION
#    include "stb_image.h"
#  pragma GCC diagnostic pop

// Restore warning levels.
#  ifdef _MSC_VER
    // Microsoft Visual C++ Compiler
#    pragma warning (pop)
#  endif
#endif
