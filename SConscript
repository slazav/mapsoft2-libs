##################################################
### lists of files:

# all library source files
library_objects = Split ("""
  opt/opt.cpp
  words/words.cpp
  iconv/iconv.cpp
  conv/conv_base.cpp
  conv/conv_geo.cpp
  conv/conv_aff.cpp
  getopt/getopt.cpp
  geom/json_pt.cpp
  geom/line_walker.cpp
  geo/geo_data.cpp
  geo/io_gpx.cpp
  geo/io_kml.cpp
  time_fmt/time_fmt.cpp
""")

# Programs inside the lib folder
programs=Split("""
  conv/conv_base.acc_test.cpp
""")

# Simple tests: Build and run program, fail if error code is not zero.
simple_tests=Split("""
  err/err.test.cpp
  opt/opt.test.cpp
  geom/point2d.test.cpp
  geom/point3d.test.cpp
  geom/rect.test.cpp
  geom/line2d.test.cpp
  geom/line3d.test.cpp
  geom/line_walker.test.cpp
  geom/multiline.test.cpp
  iconv/iconv.test.cpp
  conv/conv_base.test.cpp
  conv/conv_multi.test.cpp
  conv/conv_aff.test.cpp
  conv/conv_geo.test.cpp
  cache/cache.test.cpp
  cache/sizecache.test.cpp
  geo/geo_data.test.cpp
  time_fmt/time_fmt.test.cpp
  ozi/ozi.test.cpp
""")

# Script tests: Build a program <prog>, then run a script
# <prog>.script with program path as argument. Fail if error
# code is not zero.
script_tests=Split("""
  getopt/getopt.test.cpp
  words/words.test.cpp
""")
#  geo/io.test.cpp

##################################################
##################################################
# import and configure environments
import os
Import ('env')

# We need three environments:
# - for building programs, with -lmapsoft and other libs
# - for bilding libraries, without -lmapsoft

env_lib=env.Clone()

env_lib.UseLibs('jansson libproj libxml-2.0')

# env_lib.UseLibs('libxml-2.0 libzip libgif libjpeg libpng libtiff-4 libcurl zlib yaml-0.1 shp jansson')
# env_lib.UseLibs('glibmm-2.4 gtkmm-2.4 gthread-2.0')
# env_lib.UseLibs('cairomm-1.0 pixman-1 freetype2 libusb-1.0')

env_prg=env_lib.Clone()
env_prg.Prepend(LIBS = "-lmapsoft-static")


##################################################
## build swig

# from distutils.sysconfig import get_python_inc, get_config_var
# swig = env_lib.Clone()
# swig.Append(CPPPATH = [get_python_inc()])
# swig.Replace(SWIGFLAGS = ['-c++', '-python'])
# swig.Replace(SHLIBPREFIX = "")
# swig.SharedLibrary("_mapsoft.so", ["swig.i"])

##################################################
## build mapsoft library

sh_lib=env_lib.SharedLibrary('mapsoft', library_objects)
st_lib=env_lib.Library('mapsoft-static', library_objects)

env.Install(env.libdir, sh_lib)
env.Install(env.incdir, 'mapsoft.h')

## build programs inside the lib folder
for x in programs:
  prg=env_prg.Program(x)
  env_prg.Depends(prg, st_lib)

## build and run simple tests: fail if error code is not zero
for x in simple_tests:
  prg = env_prg.Program(x);
  res = str(prg[0]) + '.passed'
  env_prg.Command(res, prg,\
         "${SOURCE} && echo PASSED > $TARGET")
  env_prg.Depends(prg, st_lib)

## script tests: build a program, then run a script

for x in script_tests:
  prg = env_prg.Program(x);
  res = str(prg[0]) + '.passed'
  scr = str(prg[0]) + ".script"
  env_prg.Command(res, [prg, scr],\
         "${SOURCE}.script $SOURCE && echo PASSED > $TARGET")
  env_prg.Depends(prg, st_lib)

##################################################
