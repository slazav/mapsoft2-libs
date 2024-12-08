MOD_HEADERS := colors.h image.h image_r.h\
               image_colors.h image_cache.h image_test.h\
               io.h io_gif.h io_jpeg.h io_png.h io_tiff.h io_pnm.h\

MOD_SOURCES := colors.cpp image.cpp\
               image_colors.cpp image_test.cpp\
               io.cpp io_gif.cpp io_jpeg.cpp io_png.cpp io_tiff.cpp io_pnm.cpp\

SIMPLE_TESTS := colors image_r image_colors\
                io io_gif io_jpeg io_png io_tiff io_pnm\
                image_reduce image_cache image_crop

PKG_CONFIG = libjpeg libpng libtiff-4
LDLIBS = -lgif

include ../Makefile.inc
