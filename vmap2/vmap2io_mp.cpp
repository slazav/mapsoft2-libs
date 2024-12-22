#include "vmap2io.h"

/****************************************************************************/

void
mp_to_vmap2(const std::string & ifile, const VMap2types & types,
            VMap2 & vmap2, const Opt & opts){

  MP mp;
  read_mp(ifile, mp, opts);

  // be quiet (by default types of skipped objects are printed)
  bool quiet = opts.get("quite", false);

  // skip objects which are not in typeinfo file
  bool skip_unknown = opts.get("skip_unknown", false);

  std::set<uint32_t> skipped_types;

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

    // find type information if any.
    if (types.count(o1.type)){
    }
    else if (skip_unknown) {
      if (!quiet) skipped_types.insert(o1.type);
      continue;
    }

    // name, comments
    o1.name = o.Label;
    o1.comm = o.Comment;

    // Non-standard fields
    if (o.Opts.exists("Tags"))  o1.add_tags(o.Opts.get("Tags"));
    if (o.Opts.exists("Source"))o1.tags.insert(o.Opts.get("Source")); // old
    if (o.Opts.exists("Angle")) o1.angle = o.Opts.get("Angle", 0.0);
    if (o.Opts.exists("Scale")) o1.scale = o.Opts.get("Scale", 1.0);
    if (o.Opts.exists("Align")) o1.align = (VMap2objAlign)o.Opts.get("Align", 0);

    // MP object can contain different data levels.
    // When writing MP file we use typeinfo.mp_start parameter (default:0)
    // and fill only this data level.
    // When reading we use lowest non empty level.
    size_t l;
    for (l=0; l<o.Data.size() && o.Data[l].size()==0; l++) {}
    if (l>=o.Data.size()) continue; // empty object

    // set coordinates
    o1.dMultiLine::operator=(o.Data[l]);

    // add object
    vmap2.add(o1);
  }

  if (skipped_types.size()){
    std::cerr <<
       "Reading MP file: some types were skipped because "
       "skip_unknown parameter is set and there is no information in "
       "the typeinfo file:\n";
    for (const auto & t:skipped_types)
      std::cerr << VMap2obj::print_type(t) << "\n";
  }
}

/****************************************************************************/

void
vmap2_to_mp(VMap2 & vmap2, const VMap2types & types,
            const std::string & ofile, const Opt & opts){
  MP mp;
  if (opts.exists("mp_id"))     mp.ID = opts.get<int>("mp_id");
  if (opts.exists("mp_name"))   mp.Name = opts.get("mp_name");
  if (opts.exists("mp_levels")) mp.Levels = str_to_type_ivec(opts.get("mp_levels"));

  // be quiet (by default types of skipped objects are printed)
  bool quiet = opts.get("quite", false);

  // skip objects which are not in typeinfo file
  bool skip_unknown = opts.get("skip_unknown", false);

  std::set<uint32_t> skipped_unk_types;
  std::set<uint32_t> skipped_bad_types;

  // Loop through VMap2 objects:
  vmap2.iter_start();
  while (!vmap2.iter_end()){
    VMap2obj o = vmap2.iter_get_next().second;
    MPObj o1;

    // Get object class and type:
    auto cl = o.type>>24;
    auto o1.type = o.type & 0xFFFFFF;

    if (o1.type > 0x7FFF){
      skipped_bad_types.insert(o.type);
      continue;
    }

    // Type info
    VMap2type typeinfo; // default
    if (types.count(o.type))
      typeinfo = types.find(o.type)->second;
    else if (skip_unknown) {
      skipped_unk_types.insert(o.type);
      continue;
    }

    // Process text objects:
    if (cl == VMAP2_TEXT) {
      // todo?
      continue;
    }

    // convert object class to mp format
    switch (cl){
      case VMAP2_POINT:   o1.Class = MP_POINT; break;
      case VMAP2_LINE:    o1.Class = MP_LINE; break;
      case VMAP2_POLYGON: o1.Class = MP_POLYGON; break;
      default: throw Err() << "vmap2_to_mp: unsupported object: " << cl;
    }

    // name, comments
    o1.Label = o.name;
    o1.Comment = o.comm;

    // Non-standard fields
    if (o.tags.size()>0) o1.Opts.put("Tags", o.get_tags());
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

  if (skipped_bad_types.size()){
      std::cerr << "Writing MP file: skip object types bigger then 0x7FFF:\n";
      for (const auto & t:skipped_bad_types)
        std::cerr << VMap2obj::print_type(t) << "\n";
  }

  if (!quiet && skipped_unk_types.size()){
    std::cerr <<
       "Writing MP file: some types were skipped because "
       "skip_unknown parameter is set and there is no information in "
       "the typeinfo file:\n";
    for (const auto & t:skipped_unk_types)
      std::cerr << VMap2obj::print_type(t) << "\n";
  }

  write_mp(ofile, mp);
}
