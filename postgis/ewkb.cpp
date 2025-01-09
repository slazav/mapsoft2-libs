#include "ewkb.h"
#include "geo_data/conv_geo.h"
#include "err/err.h"

// Decode WKB/EWKB.. byte streams into dMultiLine
// WKB https://www.ibm.com/docs/en/informix-servers/12.10?topic=geometry-description-wkbgeometry-byte-streams
// EWKB https://github.com/postgis/postgis/blob/2.1.0/doc/ZMSgeoms.txt
// postgis/liblwgeom/lwin_wkb.c

enum wkbGeometryType {
  // standard WKB type
  wkbPoint                  = 1,
  wkbLineString             = 2,
  wkbPolygon                = 3,
  wkbMultiPoint             = 4,
  wkbMultiLineString        = 5,
  wkbMultiPolygon           = 6,
  wkbGeometryCollection     = 7,
  // additional types (see postgis/liblwgeom/liblwgeom.h.in)
  ewkbCircString            = 8,
  ewkbCompound              = 9,
  ewkbCurvePoly             = 10,
  ewkbMultiCurve            = 11,
  ewkbMultiSurf             = 12,
  ewkbPolyhedralSurf        = 13,
  ewkbTriangle              = 14,
  ewkbTin                   = 15,
  // EWKB flags
  ewkbZ    = 0x80000000,
  ewkbM    = 0x40000000,
  ewkbSRID = 0x20000000,
  // WKB flags (dec!)
  wkbZ    = 1000,
  wkbM    = 2000,
};

uint8_t
wkb_dec_byte(const std::string & str, size_t & pos){
  if (pos+1>=str.size())
    throw Err() << "wkb_dec_byte: not enough data: " << str << ": " << pos;
  auto c1 = str[pos++];
  auto c2 = str[pos++];
  if   (c1>='0'&&c1<='9') c1=c1-'0';
  else if (c1>='A'&&c1<='F') c1=c1-'A'+10;
  else throw Err() << "wkb_dec_byte: bad symbol: " << str << ": " << pos << ": " << c1;

  if   (c2>='0'&&c2<='9') c2=c2-'0';
  else if (c2>='A'&&c2<='F') c2=c2-'A'+10;
  else throw Err() << "wkb_dec_byte: bad symbol: " << str << ": " << pos << ": " << c2;
  return c1*16 + c2;
}

int
wkb_dec_int(const std::string & str, size_t & pos, const int order){
  int ret=0;
  for (int i=0; i<4; ++i ){
    int c = wkb_dec_byte(str, pos);
    if (order) ret += c<<(8*i);
    else ret += c<<(8*(3-i));
  }
  return ret;
}

double
wkb_dec_dbl(const std::string & str, size_t & pos, const int order){
  int64_t ret=0;
  for (int i=0; i<8; ++i ){
    int64_t c = wkb_dec_byte(str, pos);
    if (order) ret += c<<(8*i);
    else ret += c<<(8*(7-i));
  }
  void *x = &ret;
  return *(double*)x;
}

// decode wkb points (without M field)
dPoint
wkb_dec_pt(const std::string & str, size_t & pos, const int order, const int type){
  dPoint ret;
  ret.x = wkb_dec_dbl(str, pos, order);
  ret.y = wkb_dec_dbl(str, pos, order);
  if (type&ewkbZ || type/1000==1 || type/1000==3)
    ret.z = wkb_dec_dbl(str, pos, order); // read z field
  if (type&ewkbM || type/1000==2 || type/1000==3 )
    wkb_dec_dbl(str, pos, order); // skip M field
  return ret;
}


dMultiLine
ewkb_decode(const std::string & str, const bool docnv, size_t & pos){
  dMultiLine ret;
  if (str == "") return ret;
  int order = wkb_dec_byte(str, pos);
  if (order!=0 && order!=1) throw Err() << "wkb_decode: order byte is wrong: " << order;
  int type = wkb_dec_int(str, pos, order);
  int srid = 0;
  if (type & ewkbSRID) srid = wkb_dec_int(str, pos, order);

  switch (type & 0xF){

    case wkbPoint:
      ret.add_point(wkb_dec_pt(str, pos, order, type));
      break;

    case ewkbCircString: // should be similar to line; 3 points min; odd npoints?!
    case wkbLineString: {
      int npts = wkb_dec_int(str, pos, order);
      for (size_t i=0; i<npts; i++)
        ret.add_point(wkb_dec_pt(str, pos, order, type));
      break;
    }

    case ewkbTriangle: // should be like a polygon with 1 ring
    case wkbPolygon: {
      int nseg = wkb_dec_int(str, pos, order);
      for (size_t i=0; i<nseg; i++){
        int npts = wkb_dec_int(str, pos, order);
        ret.add_segment();
        for (size_t j=0; j<npts; j++)
          ret.add_point(wkb_dec_pt(str, pos, order, type));
      }
      break;
    }

    // multi objects
    case wkbMultiPoint:
    case wkbMultiLineString:
    case wkbMultiPolygon:
    case ewkbCurvePoly:
    case ewkbMultiCurve:
    case ewkbCompound:
    case ewkbMultiSurf:
    case ewkbTin:
    case wkbGeometryCollection: {
      int npts = wkb_dec_int(str, pos, order);
      for (size_t i=0; i<npts; i++){
        auto l = ewkb_decode(str, docnv, pos);
        ret.insert(ret.end(), l.begin(), l.end());
      }
      break;
    }
    case ewkbPolyhedralSurf:
    default: throw Err()
      << "ewkb_decode: unsupported type: " << (type & 0xF);
  }

  // convert to wgs
  if (docnv) {
    ConvGeo cnv(std::string("ESPG:") + type_to_str(srid));
    ret = cnv.frw_acc(ret);
  }
  return ret;
}

dMultiLine
ewkb_decode(const std::string & str, const bool docnv){
  size_t pos = 0;
  auto ret = ewkb_decode(str, docnv, pos);
  if (pos!=str.size()) throw Err() << "ewkb_decode: extra data: " << str.substr(pos);
  return ret;
}

