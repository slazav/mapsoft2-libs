#ifndef SHAPE_H
#define SHAPE_H

#include <memory>
#include <string>
#include "geom/multiline.h"
#include "opt/opt.h"

// INterface to shp/shx/dbf files, wrapper for libshape

class ShapeDB {

  private: // PIMPL structures
    class Impl;
    std::unique_ptr<Impl> impl;

  public:

    enum type_t {
      POINT = 0,
      LINE = 1,
      POLYGON = 2
    };

    // Open shp/shx files. Open dbf file if it exists.
    // Shape database contains a few files: <name>.shp, <name>.shx, <name>.dbf ...
    // Parameter fname could be a path to .shp file, path without ".shp" extension,
    // or path to a zip archive with the database.
    // If create == 1, create new database, using specified type.
    // If create == 0, existing db is opened, type parameter is ignored.
    ShapeDB(const std::string & fname, const bool create, const type_t type = LINE);
    ~ShapeDB();

    /********************************************/
    // SHP interface

    // Get type of objects in the shape file
    int shp_type() const;

    // Get number of records in the shape file
    int shp_num() const;

    // Get X-Y bbox (from the shape file header)
    dRect shp_bbox() const;

    // Add new object (only x-y)
    int add(const dMultiLine &ml);

    // Get an object, id = 0..shp_num()
    dMultiLine get(const int id);


    /********************************************/
    // DBF interface

    // number of records
    int dbf_num() const;

    // number of fields
    int dbf_field_num() const;

    // type of the field:
    //'C' (String), 'D' (Date), 'F' (Float)
    // 'N' (Numeric, with or without decimal)
    // 'L' (Logical), 'M' (Memo: 10 digits .DBT block ptr),
    // ' ' (field out of range).
    char dbf_field_type(int fid) const;

    // name of the field
    std::string dbf_field_name(int fid) const;

    // width of the field
    int dbf_field_width(int fid) const;

    // number of decimal places of the field
    int dbf_field_decimals(int fid) const;

    // find field ID by name (-1 if not found)
    int dbf_field_find(const char * name) const;


  // add string field, return field ID or -1 on failure
  int dbf_field_add_str(const char *name, int size);

  // write string attribute
  bool dbf_put_str(int id, int fid, const char * val);

  // get attribute (string representation of any type)
  // or empty string in case of error
  std::string dbf_get_str(int id, int fid);


  Opt get_opts(size_t i){
    Opt o;
    for (size_t j = 0; j < dbf_field_num(); ++j)
      o.put<std::string>(dbf_field_name(j), dbf_get_str(i,j));
    return o;
  }

};

#endif