#include <vector>
#include <shapefil.h>

#include "shape_db.h"
#include "err/err.h"
#include "filename/filename.h"
#include "tmpdir/tmpdir.h"

/**********************************************************/
// Implementation class
class ShapeDB::Impl {

  // SHP and DBF Interfaces
  std::shared_ptr<void> shp;
  std::shared_ptr<void> dbf;

  // Information about SHP file
  int type; // file type
  int num;  // number of records
  double cmin[4], cmax[4]; // coordinate bounds X,Y,Z,M

  // Update information; should be done after reading and modifications
  void shp_update_info();

  // for working with zip files
  std::unique_ptr<TmpDir> tmp_dir; // should be before shp,dbf
  bool modified;
  std::string fname;  // original filename
  std::string dbpath; // path to DB files (without extensions)
  std::string zbase;  // basename in zip archive

  void dbf_create();

  public:

    Impl(const std::string & fname, const bool create, const type_t type);
    ~Impl();

    type_t shp_type() const {return sh2type(type);}
    int shp_num() const {return num;}
    dRect shp_bbox() const;

    int add(const dMultiLine & ml);
    dMultiLine get(const int id);

    static ShapeDB::type_t sh2type(const int t);
    static int type2sh(const ShapeDB::type_t t);

    int dbf_field_num() const;
    char dbf_field_type(int id) const;

    int dbf_num() const;
    std::string dbf_field_name(int fid) const;
    int dbf_field_width(int fid) const;
    int dbf_field_decimals(int fid) const;
    int dbf_field_find(const char * name) const;
    int dbf_field_add_str(const char *name, int size);
    bool dbf_put_str(int id, int fid, const char * val);
    std::string dbf_get_str(int id, int fid);

};

/**********************************************************/
// Main class methods
ShapeDB::ShapeDB(const std::string & fname, const bool create, const ShapeDB::type_t type):
  impl(std::unique_ptr<Impl>(new Impl(fname, create, type))) {}

int ShapeDB::shp_type() const {return impl->shp_type();}
int ShapeDB::shp_num() const {return impl->shp_num();}
dRect ShapeDB::shp_bbox() const {return impl->shp_bbox();}

int ShapeDB::add(const dMultiLine & ml) {return impl->add(ml);}
dMultiLine ShapeDB::get(const int id) {return impl->get(id);}

// dbf
int ShapeDB::dbf_num() const {return impl->dbf_num();}
int ShapeDB::dbf_field_num() const {return impl->dbf_field_num();}
char ShapeDB::dbf_field_type(int fid) const {return impl->dbf_field_type(fid);}
std::string ShapeDB::dbf_field_name(int fid) const {return impl->dbf_field_name(fid);}
int ShapeDB::dbf_field_width(int fid) const {return impl->dbf_field_width(fid);}
int ShapeDB::dbf_field_decimals(int fid) const {return impl->dbf_field_decimals(fid);}
int ShapeDB::dbf_field_find(const char * name) const { return impl->dbf_field_find(name);}

int ShapeDB::dbf_field_add_str(const char *name, int size) {
  return impl->dbf_field_add_str(name, size);}
bool ShapeDB::dbf_put_str(int id, int fid, const char * val) {
  return impl->dbf_put_str(id, fid, val);}
std::string ShapeDB::dbf_get_str(int id, int fid){
  return impl->dbf_get_str(id, fid);}

ShapeDB::~ShapeDB(){}

/**********************************************************/
// Implementation class methods

ShapeDB::type_t
ShapeDB::Impl::sh2type(const int t){
  switch (t){
    case SHPT_POINT:   return POINT;
    case SHPT_ARC:     return LINE;
    case SHPT_POLYGON: return POLYGON;
  }
  throw Err() << "ShapeDB: unknown type: " << t;
}

int
ShapeDB::Impl::type2sh(const ShapeDB::type_t t){
  switch (t){
    case POINT:   return SHPT_POINT;
    case LINE:    return SHPT_ARC;
    case POLYGON: return SHPT_POLYGON;
  }
  throw Err() << "ShapeDB: unknown type: " << t;
}

void
ShapeDB::Impl::shp_update_info() {
  SHPGetInfo((SHPHandle)shp.get(), &num, &type, cmin, cmax);
}


ShapeDB::Impl::Impl(const std::string & fn, bool create, const ShapeDB::type_t t){
  // init shape info
  type = type2sh(t);
  num = 0;
  for (int i = 0; i<4; ++i) cmin[i] = cmax[i] = 0.0; // should we do none?
  modified = false;
  fname = fn;

  // Unzip archive if needed, calculate base name
  if (file_ext_check(fname, ".zip")){
    zbase = file_get_basename(fname, ".zip");
    if (!create && !file_exists(fname)) throw Err()
      << "ShapeDB: can't open zip file: " << fname;
    tmp_dir.reset(new TmpDir("shp2vmap_XXXXXX"));
    if (!create  && file_exists(fname)) tmp_dir->unzip(fname);
    dbpath = tmp_dir->get_dir() + "/" + zbase;
  }
  else if (file_ext_check(fname, ".shp")){
    dbpath = file_ext_repl(fname, "");
  }
  else  dbpath = fname;

  // create database if needed
  if (create) {

    // this is needed to keep file list in tmp_dir and zip/delete it correctly:
    if (tmp_dir) {
      tmp_dir->add(zbase+".shp");
      tmp_dir->add(zbase+".shx");
    }

    // create shp
    shp = std::shared_ptr<void>(SHPCreate(dbpath.c_str(), type), SHPClose);
    if (shp == NULL) throw Err() <<
        "ShapeDB: can't create shape files: " << dbpath << ".{shp,shx}";

    // delete dbf
    if (file_exists(dbpath + ".dbf")) file_remove(dbpath + ".dbf");
    if (file_exists(dbpath + ".DBF")) file_remove(dbpath + ".DBF");

    modified = true;
    return;
  }

  // open existing database
  if (!file_exists(dbpath + ".shp") && !file_exists(dbpath + ".SHP"))
    throw Err() << "ShapeDB: can't open shp file: " << dbpath << ".shp";

  if (!file_exists(dbpath + ".shx") && !file_exists(dbpath + ".SHX"))
    throw Err() << "ShapeDB: can't open shx file: " << dbpath << ".shx";

  shp = std::shared_ptr<void>(SHPOpen(dbpath.c_str(), "rb+"), SHPClose);
  if (shp == NULL) throw Err() <<
    "ShapeDB: fail in SHPOpen: " << dbpath << ".{shp,shx}";

  // open dbf if it exists
  if (file_exists(dbpath + ".dbf")){
    dbf = std::shared_ptr<void>(DBFOpen(dbpath.c_str(), "rb+"), DBFClose);
    if (dbf == NULL) throw Err() <<
      "Dbf: can't open dbf file: " << dbpath << ".dbf" ;
  }

  shp_update_info();
}

ShapeDB::Impl::~Impl(){
  shp.reset();
  dbf.reset();
  if (tmp_dir && modified) tmp_dir->zip(fname);
}

void
ShapeDB::Impl::dbf_create(){
  if (tmp_dir) tmp_dir->add(zbase+".dbf");
  dbf = std::shared_ptr<void>(DBFCreate(dbpath.c_str()), DBFClose);
  if (dbf == NULL) throw Err() <<
    "Dbf: can't create dbf file: " << dbpath << ".dbf" ;
  modified = true;
}


dRect
ShapeDB::Impl::shp_bbox() const{
  if (num==0) return dRect();
  return dRect(dPoint(cmin[0],cmin[1]), dPoint(cmax[0], cmax[1]));
}

int
ShapeDB::Impl::add(const dMultiLine & ml) {
  int nparts = ml.size();
  std::vector<int> parts;
  std::vector<double> x, y;
  int nverts = 0;
  for (auto l:ml){
    parts.push_back(nverts);
    nverts+=l.size();
    for (auto p:l) {
      x.push_back(p.x);
      y.push_back(p.y);
    }
  }
  SHPObject *o = SHPCreateObject(type, -1,
      nparts, parts.data(), NULL,
      nverts, x.data(), y.data(), NULL, NULL);
  if (o==NULL) throw Err() << "ShapeDB: can't create object";

  // It's not clear from the manual, is SHPRewindObject is called
  // if SHPCreateObject
  if (type==SHPT_POLYGON) SHPRewindObject((SHPHandle)shp.get(), o);

  int id = SHPWriteObject((SHPHandle)shp.get(), -1, o);
  SHPDestroyObject(o);
  if (id==-1) throw Err() << "ShapeDB: can't write object";

  shp_update_info();
  modified = true;
  return id;
}

dMultiLine
ShapeDB::Impl::get(const int id) {
  dMultiLine ret;
  SHPObject *o = SHPReadObject((SHPHandle)shp.get(), id);
  if (o==NULL) throw Err() << "ShapeDB: can't read object: " << id;

  // if o->nParts == 0 it means 1 part with no panPartStart info!
  if (o->nParts == 0){
    dLine l;
    for (int j=0; j<o->nVertices; j++)
      l.push_back(dPoint(o->padfX[j],o->padfY[j]));
    ret.push_back(l);
  }
  else {
    for (int p = 0; p<o->nParts; p++){
      dLine l;
      // I do not undersand, can it have points before part 0
      // (note the strange trick that both nParts=0 and nParts=1 mean one part!).
      // Let's have a warning message for safety
      if (p==0 && o->panPartStart[p] != 0)
        std::cerr << ">>> points are missing - FIXME!";
      int j1 = o->panPartStart[p];
      int j2 = (p==o->nParts-1 ? o->nVertices : o->panPartStart[p+1]);
      for (int j=j1; j<j2; j++) l.push_back(dPoint(o->padfX[j],o->padfY[j]));
      ret.push_back(l);
    }
  }
  SHPDestroyObject(o);
  return ret;
}

int
ShapeDB::Impl::dbf_num() const {
  if (!dbf) return 0;
  return DBFGetRecordCount((DBFHandle)dbf.get());
}

int
ShapeDB::Impl::dbf_field_num() const {
  if (!dbf) return 0;
  return DBFGetFieldCount((DBFHandle)dbf.get());
}

char
ShapeDB::Impl::dbf_field_type(int fid) const {
  if (!dbf || fid<0 || fid>=dbf_field_num()) throw Err()
    << "ShapeDB: field number out of range: " << fid;
  return DBFGetNativeFieldType((DBFHandle)dbf.get(), fid);
}

std::string
ShapeDB::Impl::dbf_field_name(int fid) const {
  if (!dbf || fid<0 || fid>=dbf_field_num()) throw Err()
    << "ShapeDB: field number out of range: " << fid;
  char fname[12];
  int fwidth, fdec;
  DBFGetFieldInfo((DBFHandle)dbf.get(), fid,
    fname, &fwidth, &fdec);
  return std::string(fname);
}

int
ShapeDB::Impl::dbf_field_width(int fid) const {
  if (!dbf || fid<0 || fid>=dbf_field_num()) throw Err()
    << "ShapeDB: field number out of range: " << fid;
  char fname[12];
  int fwidth, fdec;
  DBFGetFieldInfo((DBFHandle)dbf.get(), fid,
    fname, &fwidth, &fdec);
  return fwidth;
}

int
ShapeDB::Impl::dbf_field_decimals(int fid) const {
  if (!dbf || fid<0 || fid>=dbf_field_num()) throw Err()
    << "ShapeDB: field number out of range: " << fid;
  char fname[12];
  int fwidth, fdec;
  DBFGetFieldInfo((DBFHandle)dbf.get(), fid,
    fname, &fwidth, &fdec);
  return fdec;
}

int
ShapeDB::Impl::dbf_field_find(const char * name) const {
  if (!dbf) return -1;
  return DBFGetFieldIndex((DBFHandle)dbf.get(), name);
}

int
ShapeDB::Impl::dbf_field_add_str(const char *name, int size) {
   if (!dbf) dbf_create();
   modified = true;
   return DBFAddField((DBFHandle)dbf.get(), name, FTString, size, 0);
}

bool
ShapeDB::Impl::dbf_put_str(int id, int fid, const char * val) {
   return DBFWriteStringAttribute((DBFHandle)dbf.get(), id, fid, val);
}

std::string
ShapeDB::Impl::dbf_get_str(int id, int fid){
  const char * str = DBFReadStringAttribute((DBFHandle)dbf.get(), id, fid);
  return std::string(str? str:"");
}
