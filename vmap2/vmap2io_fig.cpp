#include "vmap2io.h"
#include "fig_geo/fig_geo.h"
#include "fig_opt/fig_opt.h"
#include "fig/fig_utils.h"
#include "geo_data/conv_geo.h"
#include "geom/poly_tools.h"

/****************************************************************************/

uint32_t
fig_to_type (const FigObj & o, const VMap2types & types) {

  // we are interested only in poilyline, spline, and text objects:
  if (!o.is_polyline() && !o.is_spline() && !o.is_text()) return 0;

  for (const auto & type:types){
    if (type.second.fig_mask == "") continue;

    auto t = figobj_template(type.second.fig_mask); // template object
    int c1 = o.pen_color;
    int c2 = t.pen_color;

    // lines
    if ((o.is_polyline() || o.is_spline()) && (o.size()>1)){
      // depth and thickness should match:
      if ((o.depth != t.depth) ||
          (o.thickness != t.thickness)) continue;
      // if thickness > 0 color and line style should match:
      if (o.thickness!=0 &&
          (c1!=c2 || o.line_style!=t.line_style)) continue;

      // fill
      int af1 = o.area_fill;
      int af2 = t.area_fill;
      int fc1 = o.fill_color;
      int fc2 = t.fill_color;
      // there are two types of white fill
      if ((fc1!=0xffffff)&&(af1==40)) {fc1=0xffffff; af1=20;}
      if ((fc2!=0xffffff)&&(af2==40)) {fc2=0xffffff; af2=20;}

      // fill type should match:
      if (af1 != af2) continue;
      // if fill is not transparent, fill color should match:
      if (af1!=-1 && fc1!=fc2) continue;

      // for hatch filling pen_color should match (even if thickness=0)
      if (af1>41 && c1!=c2) continue;

      // After all tests we assume that object has this type!
      return type.first;
    }

    // points
    if ((o.is_polyline() || o.is_spline()) && o.size()==1){
      //depth, thickness, color, cap_style should match:
      if (o.depth != t.depth || o.thickness != t.thickness ||
          c1 != c2 || (o.cap_style%2)!=(t.cap_style%2)) continue;
      return type.first;
    }

    // text
    if (o.type==4){
      // depth, color, font should match:
      // должны совпасть глубина, цвет, и шрифт
      if (o.depth!=t.depth || c1!=c2 || o.font!=t.font) continue;
      return type.first;
    }
  }
  return 0;
}


/****************************************************************************/

void
fig_to_vmap2(const Fig & fig, const VMap2types & types, VMap2 & vmap2){
  GeoMap ref = fig_get_ref(fig);
  ConvMap cnv(ref);

  std::vector<std::string> cmp_comm;
  for (const auto & o:fig){

    // keep compound comments:
    if (o.is_compound())  { cmp_comm = o.comment; continue; }
    if (o.is_compound_end()) { cmp_comm.clear(); continue; }

    // TODO: select depth range!

    // get object type
    auto type = fig_to_type(o, types);
    if (type==0) continue;

    // fig comment without options
    auto comm = o.comment;
    Opt o_opts = fig_get_opts(o);
    fig_del_opts(comm);

    VMap2obj o1(type);

    // coordinates
    if (o.size()<1) continue; // skip empty objects
    dLine pts(o);
    cnv.frw(pts); // point-to-point conversion
    dPoint pt0 = pts[0];

    // if closed polyline -> add one more point
    if (o1.get_class()==VMAP2_LINE && o.is_closed() &&
        o.size()>0 && o[0]!=o[o.size()-1]) pts.push_back(pts[0]);

    // Look at arrows, invert line if needed
    if (o.forward_arrow==0 && o.backward_arrow==1) pts.invert();

    o1.push_back(pts);

    // tags - space-separated list of words
    if (o_opts.exists("Tags")){
      std::istringstream s(o_opts.get("Tags"));
      std::string tag;
      while (s) {
        s >> tag;
        if (tag!="") o1.tags.insert(tag);
      }
    }

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

      // angle -- from text angle
      // angle is inverted because of y inversion
      if (o.angle!=0) o1.angle = -cnv.frw_angd(pt0, 180/M_PI*o.angle, 1000);
      else o1.angle = std::nan("");
    }
    // == Other objects ==
    else {
      // name and comment
      // copy comment from compound to the first object:
      if (cmp_comm.size()>0){
        comm=cmp_comm;
        cmp_comm.clear();
      }
      if (comm.size()>0){
        o1.name = comm[0];
        for (size_t i = 1; i<comm.size(); i++)
          o1.comm += (i>1?"\n":"") + comm[i];
      }

    }

    vmap2.add(o1);
  }
}

/****************************************************************************/

// Convert vmap2 object to fig format and add to Fig
void
vmap2_to_fig(VMap2 & vmap2, const VMap2types & types, Fig & fig){

  bool quiet = false; // by quiet (by default types of skipped objects are printed)

  // get fig reference
  GeoMap ref = fig_get_ref(fig);
  ConvMap cnv(ref); // conversion fig->wgs

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

    FigObj o1 = figobj_template(info.fig_mask);

    // convert angle
    double angle = o.angle;
    if (!std::isnan(o.angle)){
      angle=-cnv.bck_angd(pt0, -o.angle, 0.01);
    }

    // Tags, space-separated words
    if (o.tags.size()>0){
      std::string s;
      for (const auto & t:o.tags)
        s += (s.size()?" ":"") + t;
      fig_add_opt(o1, "Tags", s);
    }

    // Scale
    if (o.scale!=1.0)
      fig_add_opt(o1, "Scale", type_to_str(o.scale));


    if (o.get_class() != VMAP2_TEXT) {
      // We use text direction to keep (and edit) alignment
      if (o.align!=0)
        fig_add_opt(o1, "Align", type_to_str<int>(o.align));

      // We use text angle to keep (and edit) angle
      if (!std::isnan(angle))
        fig_add_opt(o1, "Angle", type_to_str(angle));

      if (o.name!="" || o.comm!="")
        o1.comment.push_back(o.name);
      if (o.comm!="")
        o1.comment.push_back(o.comm);
    }

    dMultiLine pts(o);
    cnv.bck(pts);
    cnv.bck(pt0);

    // Polygon: combine all segments to a single one
    // (TODO: some better solution is needed)
    if (o.get_class() == VMAP2_POLYGON){
      o1.type=2;
      o1.set_points(join_polygons(pts));
      fig.push_back(o1);
      continue;
    }

    // Line: one fig object for each segment
    // closed/open lines if needed
    if (o.get_class() == VMAP2_LINE){
      o1.type=2;
      for (const auto & l:pts){
        o1.clear();
        o1.set_points(l);
        if (o1.size()>2 && o1[0]==o1[o1.size()-1]){
          o1.resize(o1.size()-1);
          o1.close();
        }
        else {
          o1.open();
        }
        fig.push_back(o1);
      }
      continue;
    }

    // Points
    if (o.get_class() == VMAP2_POINT){
      o1.type=2;
      o1.sub_type=1;
      o1.push_back(pt0);

      // Pictures
      if (info.fig_pic.size()){
        std::list<FigObj> pic = info.fig_pic;
        for (auto & o:pic) fig_add_opt(o, "MapType", "pic");
        if (!std::isnan(angle)) fig_rotate(pic, angle);
        fig_shift(pic, pt0);
        pic.push_back(o1);
        fig_make_comp(pic);
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
      fig_add_opt(o1, "RefPt",   type_to_str(ref_pt));
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
      o1.angle = std::isnan(angle)? 0 : M_PI/180*angle;

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

}

/****************************************************************************/
