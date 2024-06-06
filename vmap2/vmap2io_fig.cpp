#include "vmap2io.h"
#include "fig_geo/fig_geo.h"
#include "fig_opt/fig_opt.h"
#include "fig/fig_utils.h"
#include "geo_data/conv_geo.h"
#include "geom/poly_tools.h"

/****************************************************************************/


// find vmap2 type for a fig object
uint32_t
fig_to_type (const FigObj & o, const VMap2types & types) {

  // we are interested only in poilyline, spline, and text objects:
  if (!o.is_polyline() && !o.is_spline() && !o.is_text()) return 0;

  for (const auto & type:types){
    if (type.second.fig_mask == "") continue;
    if (fig_match_template(o, type.second.fig_mask)) return type.first;
  }
  return 0;
}

// check typeinfo fig templates
void
fig_check_typeinfo(const VMap2types & types,
                   const int min_depth, const int max_depth){

  // Check that typeinfo is not empty.
  if (types.empty()) throw Err()
    << "typeinfo is empty, nothing can be converted (use -t option)";

  for (const auto & type1:types){
    if (type1.second.fig_mask == "") continue;

    FigObj o = figobj_template(type1.second.fig_mask);

    // Check that fig_mask contains fig object of correct type
    auto cl = VMap2obj::get_class(type1.first);
    if ((cl==VMAP2_POINT   && (o.type!=2 || o.sub_type!=1)) ||
        (cl==VMAP2_LINE    && (o.type!=2 || (o.sub_type!=1 && o.sub_type!=3))) ||
        (cl==VMAP2_POLYGON && (o.type!=2 || o.sub_type!=3)) ||
        (cl==VMAP2_TEXT    && o.type!=4) )
      throw Err() << "typeinfo: wrong type of fig template for object "
                  << VMap2obj::print_type(type1.first)
                  << ": " << type1.second.fig_mask;

    // Check that fig_mask can not be matched by any other fig_mask.
    // (object can not match two types).
    for (const auto & type2:types){
      if (type2.first <= type1.first) continue; // each pair only once
      if (type2.second.fig_mask == "") continue;
      if (fig_match_template(o, type2.second.fig_mask)) throw Err()
        << "typeinfo: fig templates for two types can match one object: "
        << VMap2obj::print_type(type1.first) << " and "
        << VMap2obj::print_type(type2.first);
    }

    // Check that fig_mask contains object with depth inside
    // min_depth - max_depth range.
    if (o.depth < min_depth) throw Err()
        << "typeinfo: fig depth is smaller then min_depth setting: "
        << VMap2obj::print_type(type1.first)
        << ": " << o.depth << " < " << min_depth;
    if (o.depth > max_depth) throw Err()
        << "typeinfo: fig depth is bigger then max_depth setting: "
        << VMap2obj::print_type(type1.first)
        << ": " << o.depth << " > " << max_depth;

    // Check that label_type contains existing type number.
    if (type1.second.label_type>=0){
      auto t = VMap2obj::make_type(VMAP2_TEXT, type1.second.label_type);
      if (types.count(t)==0) throw Err()
        << "typeinfo: " << VMap2obj::print_type(type1.first)
        << " has label_type = " << type1.second.label_type
        << " but " << VMap2obj::print_type(t) << " does not exists";
    }
  }
}


// Map parts to be removed when rewriting fig file
bool
fig_is_map_part(const FigObj & o, const int min_depth, const int max_depth){
  // keep compounds
  if (o.type==6 || o.type==-6) return false;

  // Keep reference and border?
  // This is not needed if these objects are outside min_depth..max_depth.
  // If they are inside some other troubles can happen...
  //  if (o.comment.size()>0 && o.comment[0].substr(0,3) == "REF") return false;
  //  if (o.comment.size()>0 && o.comment[0].substr(0,3) == "BRD") return false;

  // remove everything between min_depth and max_depth
  if (o.depth >= min_depth && o.depth <= max_depth) return true;

  // remove all objects with MapType parameter
  Opt oo = fig_get_opts(o);
  if (oo.exists("MapType")) return true;

  // keep everything else
  return false;
}

/****************************************************************************/

void
fig_to_vmap2(const std::string & ifile, const VMap2types & types,
             VMap2 & vmap2,  const Opt & opts){

  Fig fig;
  read_fig(ifile, fig, opts);

  GeoMap ref = fig_get_ref(fig);
  if (ref.empty()) throw Err()
    << "no reference in FIG file: " << ifile;
  ConvMap cnv(ref);

  size_t min_depth = opts.get("min_depth", 40);
  size_t max_depth = opts.get("max_depth", 200);

  fig_check_typeinfo(types, min_depth, max_depth);

  std::vector<std::string> cmp_comm;
  for (const auto & o:fig){

    // keep compound comments:
    if (o.is_compound())  { cmp_comm = o.comment; continue; }
    if (o.is_compound_end()) { cmp_comm.clear(); continue; }

    // Select depth range
    if (o.depth < min_depth || o.depth > max_depth) continue;

    // get object type
    auto type = fig_to_type(o, types);
    if (type==0) continue;

    // fig comment without options
    auto comm = cmp_comm.size()>0? cmp_comm : o.comment;
    Opt o_opts = fig_get_opts(comm);
    fig_del_opts(comm);

    VMap2obj o1(type);

    // coordinates
    if (o.size()<1) continue; // skip empty objects
    dLine pts(o);
    cnv.frw(pts); // point-to-point conversion
    dPoint pt0 = pts[0];

    // Look at arrows, invert line if needed
    if (o.forward_arrow==0 && o.backward_arrow==1) pts.invert();

    o1.push_back(pts);

    // for polygons try to find holes
    if (o1.get_class() == VMAP2_POLYGON) {
      for (auto const i:vmap2.find(type, pts.bbox())){
        auto o2 = vmap2.get(i);
        if (o2.size()<1) continue; // we will use o2[0] below

        // If o1 has no name and comment and inside
        // first loop of some other object o2, merge it to o2. Note that
        // name and comments are not transfered to
        // o1 yet, we should check comm.size()
        if (comm.size()==0 && check_hole(o2[0], pts)){
         o2.push_back(pts);
         vmap2.put(i, o2);
         o1.clear();
         break;
        }

        // If all loops of other object o2 with empty name and comment
        // are inside o1, merge it to o1:
        if (o2.name != "" || o2.comm != "") continue;
        bool all_in = true;
        for (const auto & pts2: o2) {
          if (!check_hole(pts, pts2)){
            all_in = false;
            break;
          }
        }
        if (all_in){
          o1.insert(o1.end(), o2.begin(), o2.end());
          vmap2.del(i);
        }
      }
    }
    if (o1.size()==0) continue;

    // tags - space-separated list of words
    if (o_opts.exists("Tags"))
      o1.add_tags(o_opts.get("Tags"));

    // old-style Source option - add as a tag
    if (o_opts.exists("Source"))
      o1.tags.insert(o_opts.get("Source"));

    // scale
    o1.scale = o_opts.get("Scale", 1.0);

    // == Labels ==
    if (o_opts.get("MapType")=="label" &&
        o_opts.exists("RefType") && o.is_text()){

      // Default reference point - text position
      o1.ref_pt   = o_opts.get("RefPt", pt0); cnv.frw(o1.ref_pt);
      o1.ref_type = VMap2obj::make_type(o_opts.get("RefType"));

      // Get alignment from text orientation.
      switch (o.sub_type){
        case 0: o1.align = VMAP2_ALIGN_SW; break; // left: 0 -> SW;
        case 1: o1.align = VMAP2_ALIGN_S;  break; // center: 1 -> S;
        case 2: o1.align = VMAP2_ALIGN_SE; break; // right: 2 -> SE;
      }

      // name -- from label text
      o1.name = o.text;
      for (size_t i=0; i<comm.size(); i++)
        o1.comm += (i>0?"\n":"") + comm[i];

      // Text angle in fig, rad, CCW
      if (o.angle!=0){
        dPoint pt1(pt0), pt2(pt0+dPoint(0,1e-4)); // up direction, deg
        cnv.bck(pt1); cnv.bck(pt2);  pt1-=pt2;
        double da = atan2(pt1.y, pt1.x) - M_PI/2;
        o1.angle = - 180/M_PI*(o.angle - da);
      }
      else o1.angle = std::nan("");
    }
    // == Other objects ==
    else {
      // name and comment
      if (comm.size()>0){
        o1.name = comm[0];
        for (size_t i = 1; i<comm.size(); i++)
          o1.comm += (i>1?"\n":"") + comm[i];
      }
      // Angle and Align for non-text objects
      if (o_opts.exists("Angle")) o1.angle = o_opts.get<float>("Angle");
      if (o_opts.exists("Align")) o1.align = VMap2obj::parse_align(o_opts.get("Align"));
    }

    vmap2.add(o1);
  }
}

/****************************************************************************/

// Convert vmap2 object to fig format and add to Fig
void
vmap2_to_fig(VMap2 & vmap2, const VMap2types & types,
             const std::string & ofile, const Opt & opts){

  // be quiet (by default types of skipped objects are printed)
  bool quiet = opts.get("quite", false);

  size_t min_depth = opts.get("min_depth", 40);
  size_t max_depth = opts.get("max_depth", 200);

  // Read fig file (we need reference and non-map objects)
  Fig fig;
  read_fig(ofile, fig, opts);

  fig_check_typeinfo(types, min_depth, max_depth);

  // get fig reference
  GeoMap ref = fig_get_ref(fig);
  if (ref.empty()) throw Err()
    << "no reference in FIG file, create if with "
    << "ms2geofig program before: " << ofile;
  ConvMap cnv(ref); // conversion fig->wgs

  // Cleanup fig: remove objects, labels, pictures
  auto i = fig.begin();
  while (i!=fig.end()){
    if (fig_is_map_part(*i, min_depth, max_depth)) i=fig.erase(i);
    else i++;
  }
  // Remove empty compounds
  fig_remove_empty_comp(fig);

  std::set<uint32_t> skipped_types;

  // Loop through VMap2 objects:
  vmap2.iter_start();
  while (!vmap2.iter_end()){
    VMap2obj o = vmap2.iter_get_next().second;
    if (o.size()==0 || o[0].size()==0) continue; // skip empty objects
    dPoint pt0 = o[0][0];

    if (!types.count(o.type)){
      if (!quiet) skipped_types.insert(o.type);
      continue;
    }
    auto info = types.find(o.type)->second;
    if (info.fig_mask == ""){
      if (!quiet) skipped_types.insert(o.type);
      continue;
    }

    // prepare fig template, collect options
    FigObj o1 = figobj_template(info.fig_mask);
    Opt fig_opts;

    // For non-text objects keep original angle and
    // align in options. Put name and comments to object comments.
    if (o.get_class() != VMAP2_TEXT) {

      if (o.name!="" || o.comm!="")
        o1.comment.push_back(o.name);
      if (o.comm!="")
        o1.comment.push_back(o.comm);

      if (o.align!=0)
        fig_opts.put("Align", VMap2obj::print_align(o.align));

      if (!std::isnan(o.angle))
        fig_opts.put("Angle", type_to_str(o.angle));

    }

    // Tags, space-separated words
    if (o.tags.size()>0)
      fig_opts.put("Tags", o.get_tags());

    // Scale
    if (o.scale!=1.0)
      fig_opts.put("Scale", type_to_str(o.scale));

    fig_set_opts(o1, fig_opts);

    // Convert angle. It will be needed to rotate text and
    // point pictures. Value is CCW, radians.
    // see also GObjVMap2::DrawingStep::convert_coords()
    double angle = o.angle;
    if (!std::isnan(o.angle)){
      dPoint pt1(pt0), pt2(pt0+dPoint(0,1e-4)); // up direction, deg
      cnv.bck(pt1); cnv.bck(pt2);  pt1-=pt2;
      double da = atan2(pt1.y, pt1.x) - M_PI/2;
      angle=-M_PI/180*o.angle + da;
    }

    dMultiLine pts(o);
    cnv.bck(pts);
    cnv.bck(pt0);

    // Lines/Polygons: one fig object for each segment
    if (o.get_class() == VMAP2_LINE ||
        o.get_class() == VMAP2_POLYGON){
      o1.type=2;
      if (o.get_class() == VMAP2_POLYGON)
        o1.close();
      else
        o1.open();
      for (const auto & l:pts){
        o1.clear();
        o1.set_points(l);
        fig.push_back(o1);
        // remove name and comment in all segments
        // except the first one:
        o1.comment.clear();
        fig_set_opts(o1, fig_opts);
      }
      continue;
    }

    // Points
    if (o.get_class() == VMAP2_POINT){
      o1.type=2; o1.sub_type=1;
      o1.push_back(pt0);

      // Pictures
      if (info.fig_pic.size()){
        // Read pictures, remove all compounds
        std::list<FigObj> pic = info.fig_pic;
        fig_remove_comp(pic);
        // Add "MapType=pic" comment
        for (auto & o:pic) fig_add_opt(o, "MapType", "pic");
        // Scale, rotate, shift the picture
        if (o.scale!=1.0) fig_scale(pic, o.scale);
        if (!std::isnan(angle)) fig_rotate(pic, -angle);
        fig_shift(pic, pt0);
        // Add main object, make compound
        pic.push_back(o1);
        fig_make_comp(pic);
        // transfer comment to the compound
        if (o1.comment.size()>0) pic.front().comment = o1.comment;
        // put picture
        fig.insert(fig.end(), pic.begin(), pic.end());
      }
      else{
        fig.push_back(o1);
      }
      continue;
    }

    // Text
    if (o.get_class() == VMAP2_TEXT){
      o1.type=4;
      o1.text = o.name;
      dPoint ref_pt(o.ref_pt); cnv.bck(ref_pt);
      fig_add_opt(o1, "RefPt",   type_to_str(rint(ref_pt)));
      fig_add_opt(o1, "RefType", VMap2obj::print_type(o.ref_type));
      fig_add_opt(o1, "MapType", "label");
      switch (o.align){
        case VMAP2_ALIGN_SW:
        case VMAP2_ALIGN_W:
        case VMAP2_ALIGN_NW: o1.sub_type=0; break;
        case VMAP2_ALIGN_N:
        case VMAP2_ALIGN_S:
        case VMAP2_ALIGN_C:  o1.sub_type=1; break;
        case VMAP2_ALIGN_NE:
        case VMAP2_ALIGN_E:
        case VMAP2_ALIGN_SE: o1.sub_type=2; break;
      }
      o1.angle = std::isnan(angle)? 0:angle;
      // Font size can be adjusted in the fig template
      o1.font_size = o.scale * o1.font_size;
      o1.push_back(pt0);
      fig.push_back(o1);
      continue;
    }
  }

  if (skipped_types.size()){
    std::cerr <<
       "Writing FIG file: some types were skipped because "
       "fig_mask parameter is not set in the typeinfo file:\n";
    for (const auto & t:skipped_types)
      std::cerr << VMap2obj::print_type(t) << "\n";
  }

  write_fig(ofile, fig, opts);
}

/****************************************************************************/
