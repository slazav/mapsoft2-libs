MOD_HEADERS := colors.h image.h image_r.h image_t.h\
               image_colors.h image_cache.h\
               io.h io_gif.h io_jpeg.h io_png.h io_tiff.h\

MOD_SOURCES := colors.cpp image.cpp image_t.cpp\
               image_colors.cpp\
               io.cpp io_gif.cpp io_jpeg.cpp io_png.cpp io_tiff.cpp\

SIMPLE_TESTS := colors image_r image_t image_colors\
                io io_gif io_jpeg io_png io_tiff\
                image_reduce image_cache

PKG_CONFIG = libjpeg libpng libtiff-4
LDLIBS = -lgif

include ../Makefile.inc
