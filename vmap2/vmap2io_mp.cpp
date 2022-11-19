#include "vmap2io.h"

/****************************************************************************/

void
mp_to_vmap2(const MP & mp, const VMap2types & types, VMap2 & vmap2){
  int level = 0; // move to options?

  for (auto const & o:mp){

    VMap2objClass cl;
    uint16_t tnum = o.Type & 0xFFFF;
    switch (o.Class){
      case MP_POINT:   cl = VMAP2_POINT;   break;
      case MP_LINE:    cl = VMAP2_LINE;    break;
      case MP_POLYGON: cl = VMAP2_POLYGON; break;
      default:
        throw Err() << "wrong MP class: "<< o.Class;
    }
    VMap2obj o1(cl, tnum);

    // name, comments
    o1.name = o.Label;
    o1.comm = o.Comment;

    // Non-standard fields
    if (o.Opts.exists("Source")) o1.tags.insert(o.Opts.get("Source"));
    if (o.Opts.exists("Angle")) o1.angle = o.Opts.get("Angle", 0.0);
    if (o.Opts.exists("Scale")) o1.scale = o.Opts.get("Scale", 1.0);
    if (o.Opts.exists("Align")) o1.align = (VMap2objAlign)o.Opts.get("Align", 0);

    // choose data level (move to MP?)
    int l = -1;
    if (level < (int)o.Data.size() && o.Data[level].size()>0) l = level;
    if (level <= o.EndLevel){
      for (int i = level; i>0; i--){
        if (i<(int)o.Data.size() && o.Data[i].size()>0) {l=i; break;}
      }
    }
    if (l==-1) continue; // no data for the requested level

    // set coordinates
    o1.dMultiLine::operator=(o.Data[l]);

    // add object
    vmap2.add(o1);
  }
}

/****************************************************************************/

void
vmap2_to_mp(VMap2 & vmap2, const VMap2types & types, MP & mp){

  // Loop through VMap2 objects:
  vmap2.iter_start();
  while (!vmap2.iter_end()){
    VMap2obj o = vmap2.iter_get_next().second;
    MPObj o1;

    // Get object type:
    auto t = o.type>>24;

    // Type info
    VMap2type typeinfo; // default
    if (types.count(o.type))
      typeinfo = types.find(o.type)->second;

    // Process text objects:
    if (t == VMAP2_TEXT) {
      // todo?
      continue;
    }

    // convert type to mp format
    switch (t){
      case VMAP2_POINT:   o1.Class = MP_POINT; break;
      case VMAP2_LINE:    o1.Class = MP_LINE; break;
      case VMAP2_POLYGON: o1.Class = MP_POLYGON; break;
      default: throw Err() << "vmap2_to_mp: unsupported object: " << t;
    }
    o1.Type = o.type & 0xFFFF;

    // name, comments
    o1.Label = o.name;
    o1.Comment = o.comm;

    // Non-standard fields
    if (o.tags.size()>0) o1.Opts.put("Source", *o.tags.begin());
    if (!std::isnan(o.angle)) o1.Opts.put<int>("Angle", o.angle);
    if (o.scale!=1.0) o1.Opts.put<int>("Scale", o.scale);
    if (o.align!=VMAP2_ALIGN_SW) o1.Opts.put<int>("Align", o.align);

    // EndLevel
    o1.EndLevel = typeinfo.mp_end;

    // points
    o1.Data.resize(typeinfo.mp_start+1);
    o1.Data[typeinfo.mp_start] = o;

    mp.push_back(o1);
  }
}
