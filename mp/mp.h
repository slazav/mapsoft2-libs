#ifndef MP_H
#define MP_H

#include <list>
#include <vector>
#include <string>
#include <iostream>

#include "geom/line.h"
#include "geom/multiline.h"
#include "geom/rect.h"
#include "opt/opt.h"

#define MP_POINT    0
#define MP_LINE     1
#define MP_POLYGON  2

// format description can be found in cGPSmapper-UsrMan-v02.4.pdf

struct MPObj {
    int  Class; // 0:POI, 1:POLYLINE, 2:POLYGON
    int  Type;  // mp type
    int  EndLevel;
    int  Direction;
    std::string    Label;
    Opt            Opts;  // other options
    std::vector<std::string> Comment;
    std::vector<dMultiLine> Data; // Data for different levels

    MPObj(){
      Class=MP_POINT; Type=-1; EndLevel=0; Direction=0;
    }

    bool operator== (const MPObj & o) const{
      if (Data.size()!=o.Data.size()) return false;
      std::vector<dMultiLine>::const_iterator i,j;
      for (i=Data.begin(), j=o.Data.begin(); (i!=Data.end()) && (j!=o.Data.end()) ; i++, j++)
        if (*i!=*j) return false;
      return (
        (Class==o.Class) && (Type==o.Type) && (EndLevel == o.EndLevel) &&
        (Direction == o.Direction) &&
        (Label==o.Label) && (Comment==o.Comment) && (Opts==o.Opts) );
    }
    MPObj & operator= (const dLine & l){
      Data.clear();
      dMultiLine ml;
      ml.push_back(l);
      Data.push_back(ml);
      return *this;
    }
};

struct MP : std::list<MPObj>{
    int         ID;
    std::string Name;

    int LblCoding;           // 6,9,10
    std::string Codepage;    // "1251" etc.
    //std::string Datum;     // W84, or Custom, or other.
    //bool Transparent;
    //bool MG;               // for routing maps
    //bool Numbering;
    //bool Routning;         // for routing maps
    //bool Lock;
    //int ProductCode;       // 1..255, for locked files
    //std::string CopyRight; // visible on GPS welcome page
    char Elevation;          // 'M' or 'F'
    //bool POIOnly;
    //bool POIIndex;
    //bool POINumberFirst;
    //bool POIZipFirst;
    //std::string DefaultCityCountry;   // 80 chars max
    //std::string DefaultRegionCountry; // 80 chars max
    //float TreMargin; // Not found in format description but appeared in old files.
    int TreSize;       // Max region size ~1000-~10000
    int RgnLimit;      // Max elements per region ~50..1024
    char PreProcess;   // G, F, N: type of generalization
    std::vector<int> Levels; // levels
    //std::vector<int> Zoom; // ??
    //bool Preview;     // GPS/Preview map
    //int DrawPriority; // 0..31
    //bool Marine;      // marine map?

    std::vector<std::string> Comment;
    Opt Opts;  // other options

    MP(){
      ID=0;
      LblCoding               = 9;
      Codepage                = "1251";
      Elevation               = 'M';
      PreProcess              = 'F';
      Opts["Transparent"]     = "N";
      Opts["PolygonEvaluate"] = "Y";
      Opts["MG"]              = "Y";
      Levels.push_back(22);
      Levels.push_back(21);
      Levels.push_back(19);
      Levels.push_back(17);
      TreSize=6000;
      RgnLimit=1024;
    }
    dRect bbox() const{
      dRect r;
      for (auto o:*this)
        for (auto l:o.Data) r.expand(l.bbox());
      return r;
    }
};

/******************************************************************/
// reading/writing

#include "getopt/getopt.h"

// add MP group of options
void ms2opt_add_mp(GetOptSet & opts);

/// Read mp-file from a stream <f> into <data>. If some data
/// already exists in <data> then objects are appended and the header
/// is overwrited...
/// Text is converted to UTF8 encoding using CodePage parameter in the
/// file header or "mp_in_enc" option.

void read_mp(std::istream & f, MP & data, const Opt & opts = Opt());

/// Write <data> to stream <f> in MP format.
/// Text is converted from UTF8 encoding to the encoding specified
/// by CodePage parameter or "mp_out_enc" option.
void write_mp(std::ostream & f, const MP & data, const Opt & opts = Opt());


#endif
