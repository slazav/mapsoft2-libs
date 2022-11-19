#include "vmap2io.h"
#include "fig_geo/fig_geo.h"
#include "geo_data/conv_geo.h"

/****************************************************************************/

uint32_t
fig_to_type (const FigObj & o, const VMap2types & types) {

  // we are interested only in poilyline, spline, and text objects:
  if (!o.is_polyline() && !o.is_spline() && !o.is_text()) return 0;

  for (const auto & type:types){
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
    if (o.is_compound_end()) cmp_comm.clear(); continue; }
/*
    // find normal labels
    if (o.opts.get("MapType")=="label" &&
        o.opts.exists("RefPt") &&
        o.is_text() && o.size()>0 ){

      auto t = // ...
      VMap2obj l1(VMAP2_TEXT, t);

      l1.pos = (*i)[0]; cnv.frw(l.pos);
      l1.ref_pt = i->opts.get("RefPt", l.pos); cnv.frw(l.ref);
      l1.dir   = i->sub_type;
      l1.fsize = i->font_size; // now it is absolute value
      l1.text  = i->text;
      if (i1->angle!=0){
        // angle is inverted because of y inversion
        l1.ang = -cnv.angd_frw((*i)[0], 180/M_PI*i->angle, 1000);
      }
      else {
        l1.ang = std::nan("");
      }
      vmap2.add(l1);
      continue;
    }

    // normal objects
    VMap2obj o1;
    o1.type = type;
    set_source(o.opts, i->opts.get<string>("Source"));

    if (comm.size()>0){
      o.text = comm[0];
      o.comm.insert(o.comm.begin(),
          comm.begin()+1, comm.end());
    }
    dLine pts = cnv.line_frw(*i);
    // if closed polyline -> add one more point
    if ((o.get_class() == POLYLINE) &&
        (i->is_closed()) &&
        (i->size()>0) &&
        ((*i)[0]!=(*i)[i->size()-1])) pts.push_back(pts[0]);
    o.push_back(pts);
    o.dir=zn::fig_arr2dir(*i);

    if (o.size()>0) ret.push_back(o);

    // read map objects
    if (!zconverter.is_map_depth(*i)) continue;

    int type = zconverter.get_type(*i);
    if (!type) continue;

    // copy comment from compound to the first object:
    if (cmp_comm.size()>0){
      comm=cmp_comm;
      cmp_comm.clear();
    }
    else{
      comm=i->comment;
    }

    // special type -- border
    if (type==border_type){.
      ret.brd = cnv.line_frw(*i);
      continue;
    }

    // special type -- label objects
    if (type==label_type){
      if (i->size()<2) continue;
      if (comm.size()<1) continue;
      lpos_full l;
      l.text = comm[0];
      l.ref = (*i)[0]; cnv.frw(l.ref);
      l.pos = (*i)[1]; cnv.frw(l.pos);
      l.dir = zn::fig_arr2dir(*i, true);
//      if (i->size()>=3){
//        dPoint dp=(*i)[2]-(*i)[1];
//        l.ang=cnv.angd_frw((*i)[0], 180/M_PI*atan2(dp.y, dp.x), 1000);
//        l.hor=false;
//      }
      if (i->opts.exists("TxtAngle")){
        double angle = i->opts.get<double>("TxtAngle", 0);
        l.ang=cnv.angd_frw((*i)[0], angle, 1000);
        l.hor=false;
      }
      else{
        l.ang=0;
        l.hor=true;
      }
      ret.lbuf.push_back(l);
      continue;
    }

    // normal objects
    VMap2obj o;
    o.type = type;
    set_source(o.opts, i->opts.get<string>("Source"));

    if (comm.size()>0){
      o.text = comm[0];
      o.comm.insert(o.comm.begin(),
          comm.begin()+1, comm.end());
    }
    dLine pts = cnv.line_frw(*i);
    // if closed polyline -> add one more point
    if ((o.get_class() == POLYLINE) &&
        (i->is_closed()) &&
        (i->size()>0) &&
        ((*i)[0]!=(*i)[i->size()-1])) pts.push_back(pts[0]);
    o.push_back(pts);
    o.dir=zn::fig_arr2dir(*i);

    if (o.size()>0) vmap2.push_back(o);
  }
  return ret;
*/

}

/****************************************************************************/

// Convert vmap2 object to fig format and add to Fig
void
vmap2_to_fig(VMap2 & vmap2, const VMap2types & types, Fig & fig){

  // get fig reference
  GeoMap ref = fig_get_ref(fig);
  ConvMap cnv(ref); // conversion fig->wgs

  // Loop through VMap2 objects:
  vmap2.iter_start();
  while (!vmap2.iter_end()){
    VMap2obj o = vmap2.iter_get_next().second;
    if (o.size()==0) continue;

    // Get type info
    if (types.count(o.type)<1) throw Err()
      << "unknown type: " << VMap2obj::print_type(o.type);
    auto info = types.find(o.type)->second;

    FigObj o1 = figobj_template(info.fig_mask);

    o1.comment.push_back(o.name);
    o1.comment.push_back(o.comm);

//    Opt opts;
//    opts.put("Source", o.opts.get("Source"));

//    if (o.opts.exists("Angle")){
//      double a = o.opts.get<double>("Angle");
//      a=-cnv.angd_bck((*o)[0].center(), -a, 0.01);
//      opts.put<double>("Angle", a);
//    }
/*
    if (o.get_class() == VMAP2_POLYGON){
      o1.set_points(cnv.line_bck(join_polygons(*o)));
      fig.push_back(fig);
    } else {
      dMultiLine::const_iterator l;
      for (l=o->begin(); l!=o->end(); l++){
        o1.clear();
        o1.open(); // previous part can be closed!
        o1.set_points(cnv.line_bck(*l));
        // closed polyline
        if ((o->get_class() == VMAP2_LINE) &&
            (o1.size()>2) && (o1[0]==o1[o1.size()-1])){
          o1.resize(fig.size()-1);
          o1.close();
        }
*/
//        zn::fig_dir2arr(fig, o->dir); // arrows
//        // pictures
//        std::list<fig::fig_object> tmp=zconverter.make_pic(fig, o->type);
//        F.insert(F.end(), tmp.begin(), tmp.end());
//      }
//    }
/*
    // labels connected to the object
    if (keep_labels || (o->text == "")) continue;
    std::list<lpos>::const_iterator l;
    for (l=o->labels.begin(); l!=o->labels.end(); l++){
      dPoint ref;  dist_pt_l(l->pos, *o, ref);
      cnv.bck(ref);
      dPoint pos = l->pos;
      cnv.bck(pos);

      double angle = l->hor ? 0 : cnv.angd_bck(l->pos, -l->ang, 0.01);

      fig::fig_object txt;
      if (fig_text_labels){
        txt=zconverter.get_label_template(o->type);
        txt.text=conv_label(o->text);
        txt.sub_type=l->dir;
        txt.angle=M_PI/180*angle;
        txt.font_size += l->fsize;
        txt.push_back(pos);
        txt.opts.put<iPoint>("RefPt", ref);
        txt.opts.put<string>("MapType", "label");
      }
      else {
        txt.clear();
        txt=zconverter.get_fig_template(label_type);
        zn::fig_dir2arr(txt, l->dir, true);
        txt.push_back(ref);
        txt.push_back(pos);
        if (!l->hor) txt.opts.put<double>("TxtAngle", angle);
        txt.comment.push_back(o->text);
      }
      F.push_back(txt);
    }
*/
  }
  // TODO: write detached labels (lbuf)!
}

/****************************************************************************/
