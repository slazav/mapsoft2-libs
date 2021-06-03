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
  Multi-segment borders are supported by using multiple BRD objects.
  Map projection is stored in `map_proj` option, as libproj string.

- Waypoints are stored in objects (normally points) with comment started with
  `WPT <name>`. Point labels have first comment `WPL` (this is new in mapsoft2)

- Tracks are stored in objects (normally polylines) with comment started with
  `TRK <name>`

- Raster maps are stored in image objects with comment started with
  `MAP <name>`. If an object with comment `BRD <name>` exists, then
  it defines border for this map.

## Difference between mapsoft1 and mapsoft2

- In mapsoft1 it was possible to write coordinates of reference points
in various projections (this was controlled by proj, datum,... options).
In mapsoft2 it is not supported, all coordinates are in WGS84.

- Map projection is set differently, now it's a libproj string instead
of a set of parameters (map_proj, lon0, etc.) Some old-style projections
are supported (tmerc, lonlat). This feature makes files written by mapsoft2
incompatable with mapsoft1.

- In mapsoft1 many point and track parameters were written as options
in fig objects. Now it's not supported, only point/track name and
coordinates are stored in fig file.

- In mapsoft2 Multi-segment borders are supported (by using a few polygons
with same `BRD <name>` comment.

- In mapsoft2 waypoint labels are marked with `WPL` comment and can be
removed together with points.

-----------
## Changelog:

2021.06.03 V.Zavjalov 1.0:
- First version
