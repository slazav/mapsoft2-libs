#include "vmap2io.h"
#include "geo_data/geo_io.h"

/****************************************************************************/

void
gpx_to_vmap2(const std::string & ifile, VMap2 & vmap2, const Opt & opts){

  if (!opts.exists("type")) throw Err() <<
    "use --type option for importing gpx data";
  auto type = VMap2obj::make_type(opts.get("type"));

  GeoData data;
  read_gpx(ifile, data, opts);

  for (auto const & tr:data.trks){
    // make object
    VMap2obj o1(type);

    for (auto const & l:(dMultiLine)tr){
      if (l.size()==0) continue;
      o1.clear();
      o1.push_back(l);
      vmap2.add(o1);
    }
  }
}

/****************************************************************************/

void
vmap2_to_gpx(VMap2 & vmap2, const std::string & ofile, const Opt & opts){

  if (!opts.exists("type")) throw Err() <<
    "use --type option for exporting gpx data";
  auto type = VMap2obj::make_type(opts.get("type"));

  dMultiLine ml;

  // Loop through VMap2 objects of the given type:
  for (const auto i: vmap2.find(type)){
    auto o = vmap2.get(i);
    ml.insert(ml.end(), o.begin(), o.end());
  }

  GeoData data;
  data.trks.push_back(GeoTrk(ml));

  write_gpx(ofile, data, opts);
}

