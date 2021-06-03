## fig_geo

An extension of FIG format to keep geo reference and geodata.
(compatible with old mapsoft).

This is a very old format which I use for working with geodata: first awk
scripts for handling FIG files with geodata have been done before year 2000.
In 2002 they were rewritten in Perl.

- The fig file and each object can contain options embedded in the comments
  (see ../fig_opt)

- Reference is stored in objects (normally points) with first comment
  `REF <lon> <lat>`. Border for the reference is stored in objects
  (polygons) with first comment `BRD <name>`, where `<name>` is
  same as in the `name` option of the fig file (and can be empty).
  Multi-segment borders are supported by using multiple BRD objects -- this is
  new in mapsoft2.

- Waypoints are stored in objects (normally points) with comment started with
  `WPT <name>`

- Tracks are stored in objects (normally polylines) with comment started with
  `TRK <name>`

- Raster maps are stored in image objects with comment started with
  `MAP <name>`. If an object with comment `BRD <name>` exists, then
  it defines border for this map.

-----------
## Changelog:

2021.01.01 V.Zavjalov 1.0:
- First version
