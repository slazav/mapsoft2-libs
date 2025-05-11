#include <regex>

#include "vmap2/vmap2types.h"
#include "filename/filename.h"
#include "geom/line_rectcrop.h"
#include "geom/poly_tools.h"
#include "geo_data/conv_geo.h"
#include "geo_data/geo_utils.h"
#include "geo_nom/geo_nom_fi.h"
#include "srtm/srtm.h"

#include "actions.h"


/***********************************************************/
// check argument number, throw Err if needed
void
VMap2action::check_args(const std::vector<std::string> & cmd,
           const std::vector<std::string> & expected){
  auto n = expected.size();
  if (n>0 && expected[n-1] == "..." && cmd.size()>=n-1) return;
  if (cmd.size()==n) return;
  Err e;
  e << "wrong number of arguments (" << expected.size() << " expected): ";
  for (const auto & p:expected) e << " <" << p << ">";
  throw e; 
}

class vmap_action_delete: public VMap2action {
  public:
  vmap_action_delete(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {});
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    vmap.del(id);
    return true;
  }
};

class vmap_action_print: public VMap2action {
  std::string msg;
  public:
  vmap_action_print(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"msg"});
    msg = args[0];
  }
  void repl(std::string & str, const std::string & f, const std::string & r){
    auto n = str.find(f);
    if (n != str.npos) str.replace(n, f.size(), r);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    auto m(msg);
    repl(m, "${name}", o.name);
    repl(m, "${type}", o.print_type());
    repl(m, "${ref_type}", o.print_ref_type());
    repl(m, "${angle}", type_to_str(o.angle));
    repl(m, "${scale}", type_to_str(o.scale));
    std::cout << m << "\n";
    return false;
  }
};

class vmap_action_set_type: public VMap2action {
  uint32_t type;
  public:
  vmap_action_set_type(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"type"});
    type = VMap2obj::make_type(args[0]);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    if (o.type == type) return false;
    o.type = type;
    vmap.put(id, o);
    return true;
  }
};

class vmap_action_set_ref_type: public VMap2action {
  uint32_t type;
  public:
  vmap_action_set_ref_type(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"type"});
    type = VMap2obj::make_type(args[0]);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    if (o.ref_type == type) return false;
    o.ref_type = type;
    vmap.put(id, o);
    return true;
  }
};

class vmap_action_set_scale: public VMap2action {
  double scale;
  uint32_t type;
  public:
  vmap_action_set_scale(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"scale"});
    scale = str_to_type<double>(args[0]);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    if (o.scale == scale) return false;
    o.scale = scale;
    vmap.put(id, o);
    return true;
  }
};

class vmap_action_set_angle: public VMap2action {
  double angle;
  public:
  vmap_action_set_angle(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"angle[deg]"});
    angle = str_to_type<double>(args[0]);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    if (o.angle == angle) return false;
    o.angle = angle;
    vmap.put(id, o);
    return true;
  }
};

class vmap_action_set_name: public VMap2action {
   std::string new_name;
  public:
  vmap_action_set_name(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"name"});
    new_name = args[0];
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    if (o.name == new_name) return false;
    o.name = new_name;
    vmap.put(id, o);
    return true;
  }
};

class vmap_action_re_name: public VMap2action {
  std::regex re;
  std::string repl;
  public:
  vmap_action_re_name(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"match", "repl"});
    re = std::regex(args[0]);
    repl = args[1];
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    auto n = std::regex_replace(o.name, re, repl);
    if (n!=o.name){
      o.name = n;
      vmap.put(id, o);
      return true;
    }
    return false;
  }
};

class vmap_action_tr_name: public VMap2action {
   std::string old_name, new_name;
  public:
  vmap_action_tr_name(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"old_name", "new_name"});
    old_name = args[0];
    new_name = args[1];
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    if (o.name != old_name) return false;
    if (old_name == new_name) return false;
    o.name = new_name;
    vmap.put(id, o);
    return true;
  }
};

class vmap_action_crop_rect: public VMap2action {
  dRect box;
  public:
  vmap_action_crop_rect(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"bbox"});
    box = str_to_type<dRect>(args[0]);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    bool closed = (o.get_class() == VMAP2_POLYGON);
    o.set_coords(rect_crop_multi(box, o, closed));
    if (o.empty()) vmap.del(id);
    else vmap.put(id, o);
    return true;
  }
};

class vmap_action_crop_nom: public VMap2action {
  dRect box;
  public:
  vmap_action_crop_nom(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"name"});
    box = nom_to_wgs(args[0]);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    bool closed = (o.get_class() == VMAP2_POLYGON);
    o.set_coords(rect_crop_multi(box, o, closed));
    if (o.empty()) vmap.del(id);
    else vmap.put(id, o);
    return true;
  }
};

class vmap_action_crop_nom_fi: public VMap2action {
  ConvGeo cnv;
  dRect box;
  public:
  vmap_action_crop_nom_fi(VMap2 & vmap, const std::vector<std::string> & args):
       VMap2action(vmap), cnv("ETRS-TM35FIN"){
    check_args(args, {"name"});
    box = nom_to_range_fi(args[0]);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    bool closed = (o.get_class() == VMAP2_POLYGON);
    cnv.bck(o); // convert to ETRS-TM35FIN;
    o.set_coords(rect_crop_multi(box, o, closed));
    cnv.frw(o); // convert to WGS;
    if (o.empty()) vmap.del(id);
    else vmap.put(id, o);
    return true;
  }
};

class vmap_action_set_alt_name: public VMap2action {
  SRTM srtm;
  public:
  vmap_action_set_alt_name(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"dir"});
    Opt opt;
    opt.put("srtm_dir", args[0]);
    opt.put("srtm_use_overlay", "1");
    srtm.set_opt(opt);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    o.name = type_to_str<int>(rint(srtm.get_h(o.get_first_pt())));
    vmap.put(id, o);
    return true;
  }
};

// Move object ends towards nearest point
// or extend/reduce end segments to nearest segment of an object from <types>.
class vmap_action_move_ends: public VMap2action {
  double r;
  std::list<std::string> types;
  public:
  vmap_action_move_ends(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"dist[m]", "type1", "..."});
    r = str_to_type<double>(args[0]);
    types = std::list<std::string>(args.begin()+1, args.end());
  }

  int move_ends_pt(VMap2 & vmap, const int id0, dPoint & p1, dPoint & p2){
    // some range around p1 (larger then r)
    dRect rng(p1,p1);
    double d2m = 6380e3*M_PI/180.0; // m/d
    rng.expand(r / d2m);
    dLine dest; // possible destination points
    for (const auto type: types){
      for (const auto id: vmap.find(type, rng)){
        if (id==id0) continue;
        auto obj = vmap.get(id);
        auto cl = obj.get_class();
        for (auto & l:obj){

          // object node near point p1 -> move p1 there
          for (auto & p:l){
            if (geo_dist_2d(p1,p) > r) continue;
            //std::cerr << " p: " << p1 << " -> " << p << "\n";
            dest.push_back(p);
          }

          // object segment near p1 -> change length of p1-p2 segment
          if (cl == VMAP2_POINT) continue;
          for (size_t j=0; j<l.size(); j++){
            auto q1 = l[j];
            auto q2 = (j==l.size()-1) ? l[0]:l[j+1];
            if (q1==q2) continue;
            if (cl == VMAP2_LINE && j==l.size()-1) continue;

            dPoint cr;
            segment_cross_2d(p1, p2, q1, q2, cr);

            if (std::isnan(cr.x) || std::isinf(cr.x) ||
                std::isnan(cr.y) || std::isinf(cr.y)) continue;
            double dp = geo_dist_2d(p1,p2);
            double dq = geo_dist_2d(q1,q2);
            if (geo_dist_2d(cr,q1) > dq || geo_dist_2d(cr,q2) > dq) continue;
            if (geo_dist_2d(cr,p1) > r) continue;
            //std::cerr << " s: " << p1 << " -> " << cr << "\n";
            dest.push_back(cr);
          }
        }
      }
    }
    if (dest.size()==0) return 0;
    // choose nearest destination point
    dPoint pm = dest[0];
    for (const auto p:dest)
      if (geo_dist_2d(p1,p) < geo_dist_2d(p1, pm)) pm=p;
    p1 = pm;
    return 1;
  }

  bool process_object(uint32_t id, VMap2obj & o) override {
    size_t count = 0;
    for (auto & l:o){
      if (l.size()<2) continue;
      // too short line can collapse into one point
      if (geo_dist_2d(l[0], l[l.size()-1]) <= 2*r) continue;
      count +=
        move_ends_pt(vmap, id, l[0], l[1]) +
        move_ends_pt(vmap, id, l[l.size()-1], l[l.size()-2]);
    }
    if (count){
      vmap.put(id, o);
      return true;
    }
    return false;
  }
};

// Remove segments shorter then n points (<0 for no filtering) or d distance (<0 for no filtering)
class vmap_action_rem_short: public VMap2action {
  int npts;
  double len;
  public:
  vmap_action_rem_short(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"min_npts", "min_len[m]"});
    npts = str_to_type<int>(args[0]);
    len = str_to_type<double>(args[1]);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    size_t cnt=0;
    auto l = o.begin();
    while (l!=o.end()) {
      // remove empty and 1-pt segments
      if (l->size()<npts || l->length()<len) {
        l=o.erase(l);
        cnt++;
        continue;
      }
    }
    if (cnt){
      if (o.size()) vmap.put(id, o);
      else vmap.del(id);
      return true;
    }
    return false;
  }
};

// Remove duplicated points
class vmap_action_rem_dup_pts: public VMap2action {
  double dist;
  public:
  vmap_action_rem_dup_pts(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"dist[m]"});
    dist = str_to_type<double>(args[0]);
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    size_t cnt=0;
    auto l = o.begin();
    while (l!=o.end()) {
      auto i = l->begin();
      while (i+1 != l->end()){
        if (geo_dist_2d(*i, *(i+1)) < dist){
          i=l->erase(i);
          cnt++;
        }
        else i++;
      }
      l++;
    }
    if (cnt){
      vmap.put(id, o);
      return true;
    }
    return false;
  }
};

// Remove duplicated points
class vmap_action_translate: public VMap2action {
  std::map<std::string, std::string> dict;
  public:
  vmap_action_translate(VMap2 & vmap, const std::vector<std::string> & args): VMap2action(vmap){
    check_args(args, {"dict.file"});
    auto fname = args[0];
    std::ifstream ff(fname);
    if (!ff) throw Err() << "can't open file: " << fname;
    int line_num[2] = {0,0}; // line counter for read_words
    read_words_defs defs;
    while (1){
      auto vs = read_words(ff, line_num, false);
      if (vs.size()==0) break;
      try{
        if (vs.size()!=2) throw Err() << "2-column dictionary expected";
        dict.emplace(vs[0],vs[1]);
      }
      catch (Err & e) {
        throw Err() << fname << ":" << line_num[0] << ": " << e.str();
      }
    }
  }
  bool process_object(uint32_t id, VMap2obj & o) override {
    if (dict.count(o.name)){
      o.name = dict[o.name];
      vmap.put(id, o);
      return true;
    }
    std::cout << "can't translate: " << o.name << "\n";
    return false;
  }
};


std::shared_ptr<VMap2action>
VMap2action::get_action(VMap2 & vmap, const std::vector<std::string> & cmd){
  if (cmd.size()==0) throw Err() << "empty command";
  std::vector<std::string> args(cmd.begin()+1, cmd.end());

  if (cmd[0] == "delete")        return std::shared_ptr<VMap2action>(new vmap_action_delete(vmap, args));
  if (cmd[0] == "print")         return std::shared_ptr<VMap2action>(new vmap_action_print(vmap, args));
  if (cmd[0] == "set_type")      return std::shared_ptr<VMap2action>(new vmap_action_set_type(vmap, args));
  if (cmd[0] == "set_ref_type")  return std::shared_ptr<VMap2action>(new vmap_action_set_ref_type(vmap, args));
  if (cmd[0] == "set_scale")     return std::shared_ptr<VMap2action>(new vmap_action_set_scale(vmap, args));
  if (cmd[0] == "set_angle")     return std::shared_ptr<VMap2action>(new vmap_action_set_angle(vmap, args));
  if (cmd[0] == "set_name")      return std::shared_ptr<VMap2action>(new vmap_action_set_name(vmap, args));
  if (cmd[0] == "re_name")       return std::shared_ptr<VMap2action>(new vmap_action_re_name(vmap, args));
  if (cmd[0] == "tr_name")       return std::shared_ptr<VMap2action>(new vmap_action_tr_name(vmap, args));
  if (cmd[0] == "crop_rect")     return std::shared_ptr<VMap2action>(new vmap_action_crop_rect(vmap, args));
  if (cmd[0] == "crop_nom")      return std::shared_ptr<VMap2action>(new vmap_action_crop_nom(vmap, args));
  if (cmd[0] == "crop_nom_fi")   return std::shared_ptr<VMap2action>(new vmap_action_crop_nom_fi(vmap, args));
  if (cmd[0] == "set_alt_name")  return std::shared_ptr<VMap2action>(new vmap_action_set_alt_name(vmap, args));
  if (cmd[0] == "move_ends")     return std::shared_ptr<VMap2action>(new vmap_action_move_ends(vmap, args));
  if (cmd[0] == "rem_short")     return std::shared_ptr<VMap2action>(new vmap_action_rem_short(vmap, args));
  if (cmd[0] == "rem_dup_pts")   return std::shared_ptr<VMap2action>(new vmap_action_rem_dup_pts(vmap, args));
  if (cmd[0] == "translate")     return std::shared_ptr<VMap2action>(new vmap_action_translate(vmap, args));
  throw Err() << "unknown command: " << cmd[0];
}

