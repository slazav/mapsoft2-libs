#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>

#include "fig.h"
#include "iconv/iconv.h"

using namespace std;
string fig_default_enc("KOI8-R");

/******************************************************************/
void
ms2opt_add_fig(GetOptSet & opts){
  const char *g = "FIG";
  opts.add("fig_enc", 1,0,g,
    "Encoding for reading/writing fig files (default: KOI8-R). "
    "When reading utf8 files with a special comment (made by xfig>3.2.9) this option is ignored.");
// // Fig_header option is needed only for tests,
// // I do not believe it is useful for any programs.
//  opts.add("fig_header", 1,0,g,
//    "Read/write fig headers (default: true).");
}

/******************************************************************/
// extract a value from a single line
template <typename T>
void read_line(std::istream & s, T & val){
  std::string l;
  std::getline(s,l);
  std::istringstream s1(l);
  s1 >> val;
  if (!s1.eof()) s1 >> std::ws;
  if (s1.fail() || !s1.eof())
    throw Err() << "Fig: can't read fig file";
}
// version for string: read averything until end of line
void read_line(std::istream & s, std::string & val){
  std::getline(s,val);
  val.erase(val.find_last_not_of(' ') + 1); // trim spaces
  val.erase(0, val.find_first_not_of(' '));
  if (s.fail())
    throw Err() << "Fig: can't read fig file";
}


// read object text (char by char, with \XXX conversions).
// Returns true if '\\001' did not found.
// Data will be appended to text variable
bool read_text(std::istream & ss, string & text){
  while (1){
    char c = ss.get();
    if (ss.eof()) return true;
    if (ss.fail())
      throw Err() << "";
    if (c == '\\'){
      char c1,c2,c3;
      c1 = ss.get();
      if (ss.fail())
        throw Err() << "";
      if (c1 < '0' || c1 > '3'){
        c = c1;
      }
      else {
        if ((c2 = ss.get()) < '0' || c2 > '7' || ss.fail() ||
            (c3 = ss.get()) < '0' || c3 > '7' || ss.fail())
          throw Err() << "bad \\nnn octal number";

        c1 -= '0'; c2 -= '0'; c3 -= '0';
        c = (c1 << 6) +  (c2 << 3) + c3;
      }
    }
    if (c == 1) return false;
    text.push_back(c);
  }
  throw Err() << "broken code";
}

// read object comments, return first non-comment line
// (object header)
std::string
read_comments(std::istream & s, vector<string> & comment, const IConv & cnv){
  std::string ret;
  while (getline(s, ret)){
    if (s.fail()) throw Err() << "FigObj: can't read object";
    if (ret.size()<1) continue; // skip empty lines
    if (ret[0] != '#') break; // start reading the object ret
    if (ret.size()<2 || ret[1] != ' ')
      std::cerr << "FigObj: broken comment: [" << ret << "]\n";
    comment.push_back(cnv(ret.substr(2, -1)));
  }
  return ret;
}

// Working with colors.
// - Internally all colors are stored as 24-bit RGB values.
//   No transparency.
// - Default color (-1) is converted to 0xFF000000
// - There is a default colormap (Fig colors 0..31).
//   A custom colormap can also exist in the fig file.
// - When reading object header, colors are converted to RGB.
//   using both colormaps.
//   It is allowed to use #RRGGBB values instead of colors in
//   object headers. (This is my extension of Fig, I use it to
//   construct objects with non-std colors).

// Convert Fig color to RGB, interger version, does not support #RRGGBB
// colors, to be used in normal reading
int
color_fig2rgb(const int c, const map<int,int> & custom){
  if (c==-1) return 0xFF000000; // default color
  // find color in the default colormap
  auto i=Fig::colors.find(c);
  if (i!=Fig::colors.end()) return i->second;
  // find color in the custom colormap
  i=custom.find(c);
  if (i!=custom.end()) return i->second;
  cerr << "Fig: Unknown color: " << c << "\n";
  return -1;
}

// Convert Fig color to RGB, string version, supports #RRGGBB
// colors, to be used in template reading
int
color_fig2rgb(const std::string & s, const map<int,int> & custom){
  if (s.size()<1) return -1;

  if (s[0] == '#'){
    int ret;
    std::istringstream ss(s.substr(1));
    ss >> std::hex >> ret;
    if (ss.fail() || !ss.eof())
      throw Err() << "can't parse color value: \"" << s << "\"";
    return ret & 0xFFFFFF;
  }
  return color_fig2rgb(str_to_type<int>(s), custom);
}

// Convert RGB color to Fig color.
int
color_rgb2fig(const int c, const map<int,int> & custom_cmap){
  if (c==0xFF000000) return -1; // default color
  int c1 = c & 0xFFFFFF;
  for (const auto & m : Fig::colors){
    if (m.first != -1 && m.second == c1) return m.first;
  }
  for (const auto & m : custom_cmap){
    if (m.second == c1) return m.first;
  }
  cerr << "Fig: Unknown color: " << c << "\n";
  return -1;
}

// Add new rgb color to custom colormap if needed. Return true if color was added.
bool
color_rgbadd(const int c, map<int,int> & custom_cmap){
  int maxc=0;
  int c1 = c & 0xFFFFFF;
  for (const auto & m : Fig::colors){
    if (m.first > maxc) maxc=m.first;
    if (m.first != -1 && m.second == c1) return false;
  }
  for (const auto & m : custom_cmap){
    if (m.first > maxc) maxc=m.first;
    if (m.second == c1) return false;
  }
  custom_cmap.emplace(maxc+1, c1);
  return true;
}

/******************************************************************/
// Create object using  header string
int
read_figobj_header(FigObj & o, const std::string & header,
                       const map<int,int> & custom_cmap){
  std::istringstream ss(header);
  ss >> o.type;
  if (!ss.eof()) ss >> ws;
  if (ss.fail())
    throw Err() << "FigObj: can't read object: [" << header << "]";

  int ret = 0;
  int x1,x2,x3,y1,y2,y3;
  switch (o.type) {
    case FIG_ELLIPSE:
      ss >> o.sub_type >> o.line_style >> o.thickness >> o.pen_color >> o.fill_color >> o.depth
         >> o.pen_style >> o.area_fill >> o.style_val >> o.direction >> o.angle
         >> x1 >> y1 >> o.radius_x >> o.radius_y >> o.start_x >> o.start_y >> o.end_x >> o.end_y;
      if (!ss.eof()) ss >> ws;
      if (ss.fail() || !ss.eof())
        throw Err() << "FigObj: can't read ellipse object: [" << header << "]";
      o.push_back(iPoint(x1,y1));
      break;
    case FIG_POLYLINE:
      ss >> o.sub_type >> o.line_style >> o.thickness >> o.pen_color >> o.fill_color >> o.depth
         >> o.pen_style >> o.area_fill >> o.style_val >> o.join_style >> o.cap_style >> o.radius
         >> o.forward_arrow >> o.backward_arrow >> ret;
      if (!ss.eof()) ss >> ws;
      if (ss.fail() || !ss.eof())
        throw Err() << "FigObj: can't read line object: [" << header << "]";
      break;
    case FIG_SPLINE:
      ss >> o.sub_type >> o.line_style >> o.thickness >> o.pen_color >> o.fill_color >> o.depth
         >> o.pen_style >> o.area_fill >> o.style_val >> o.cap_style
         >> o.forward_arrow >> o.backward_arrow >> ret;
      if (!ss.eof()) ss >> ws;
      if (ss.fail() || !ss.eof())
        throw Err() << "FigObj: can't read spline object: [" << header << "]";
      break;
    case FIG_TXT:
      ss >> o.sub_type >> o.pen_color >> o.depth >> o.pen_style
         >> o.font >> o.font_size >> o.angle >> o.font_flags
         >> o.height >> o.length >> x1 >> y1;
      o.fill_color=7;
      ss.get(); // read space;
      if (ss.fail())
        throw Err() << "FigObj: can't read text object: [" << header << "]";

      try{
        o.text.resize(0);
        ret = read_text(ss, o.text);
      }
      catch (Err & e){
        throw Err() << "FigObj: can't read text: " << e.str()
                    << ": [" << header << "]";
      }
      o.push_back(iPoint(x1,y1));
      break;
    case FIG_ARC:
      ss >> o.sub_type >> o.line_style >> o.thickness >> o.pen_color >> o.fill_color >> o.depth
         >> o.pen_style >> o.area_fill >> o.style_val >> o.cap_style
         >> o.direction >> o.forward_arrow >> o.backward_arrow
         >> o.center_x >> o.center_y >> x1 >> y1 >> x2 >> y2 >> x3 >> y3;
      if (!ss.eof()) ss >> ws;
      if (ss.fail() || !ss.eof())
         throw Err() << "FigObj: can't read arc object: [" << header << "]";
      o.push_back(iPoint(x1,y1));
      o.push_back(iPoint(x2,y2));
      o.push_back(iPoint(x3,y3));
      break;
    case FIG_COMPOUND:
      if (!ss.eof()) ss >> x1 >> y1 >> x2 >> y2;
      else x1=x2=y1=y2=0;
      if (!ss.eof()) ss >> ws;
      if (ss.fail() || !ss.eof())
        throw Err() << "FigObj: can't read compound object: [" << header << "]";
      o.push_back(iPoint(x1,y1));
      o.push_back(iPoint(x2,y2));
      break;
    case FIG_END_COMPOUND:
      if (ss.fail() || !ss.eof())
        throw Err() << "FigObj: can't read compound end object: [" << header << "]";
      break;
    case FIG_COLOR_DEF:
      throw Err() << "FigObj: color definitions must come before other objects: [" << header << "]";
    default:
      throw Err() << "FigObj: unknown object type: [" << header << "]";
  }

  // convert colors
  o.pen_color  = color_fig2rgb(o.pen_color, custom_cmap);
  o.fill_color = color_fig2rgb(o.fill_color, custom_cmap);

  return ret;
}

FigObj
read_figobj_header(const std::string & header,
                   const map<int,int> & custom_cmap){
  FigObj ret;
  read_figobj_header(ret, header, custom_cmap);
  return ret;
}

FigObj
figobj_template(const std::string & templ){
  FigObj o;

  std::istringstream ss(templ);
  ss >> o.type;
  if (!ss.eof()) ss >> ws;
  if (ss.fail())
    throw Err() << "FigObj: can't read template: [" << templ << "]";

  int ret = 0;
  int x1,x2,x3,y1,y2,y3;
  std:string pen_color, fill_color; // read as strings, convert later
  switch (o.type) {
    case FIG_ELLIPSE:
      ss >> o.sub_type >> o.line_style >> o.thickness >> pen_color >> fill_color >> o.depth
         >> o.pen_style >> o.area_fill >> o.style_val >> o.direction >> o.angle;
      break;
    case FIG_POLYLINE:
      ss >> o.sub_type >> o.line_style >> o.thickness >> pen_color >> fill_color >> o.depth
         >> o.pen_style >> o.area_fill >> o.style_val >> o.join_style >> o.cap_style >> o.radius
         >> o.forward_arrow >> o.backward_arrow;
      break;
    case FIG_SPLINE:
      ss >> o.sub_type >> o.line_style >> o.thickness >> pen_color >> fill_color >> o.depth
         >> o.pen_style >> o.area_fill >> o.style_val >> o.cap_style
         >> o.forward_arrow >> o.backward_arrow;
      break;
    case FIG_TXT:
      ss >> o.sub_type >> pen_color >> o.depth >> o.pen_style
         >> o.font >> o.font_size >> o.angle >> o.font_flags;
      break;
    case FIG_ARC:
      ss >> o.sub_type >> o.line_style >> o.thickness >> pen_color >> fill_color >> o.depth
         >> o.pen_style >> o.area_fill >> o.style_val >> o.cap_style
         >> o.direction >> o.forward_arrow >> o.backward_arrow;
      break;
    case FIG_COMPOUND:
    case FIG_END_COMPOUND:
      break;
    default:
      throw Err() << "FigObj: unknown template type: [" << templ << "]";
  }
  if (!ss.eof()) ss >> ws;

  // read arrow parameters if needed
  if (o.type == FIG_POLYLINE || o.type == FIG_SPLINE || o.type == FIG_ARC){
    if (o.forward_arrow) {
      ss >> o.farrow_type >> o.farrow_style >> o.farrow_thickness
         >> o.farrow_width >> o.farrow_height;
    }
    if (o.backward_arrow) {
      ss >> o.barrow_type >> o.barrow_style >> o.barrow_thickness
         >> o.barrow_width >> o.barrow_height;
    }
  }
  if (!ss.eof()) ss >> ws;
  if (ss.fail() || !ss.eof())
    throw Err() << "FigObj: can't read template: [" << templ << "]";

  // convert colors (no custom colormap)
  map<int,int> cmap;
  o.pen_color  = color_fig2rgb(pen_color, cmap);
  o.fill_color = color_fig2rgb(fill_color, cmap);

  return o;
}



/******************************************************************/
void read_fig(std::istream & s, Fig & w, const Opt & ropts){
  string enc = ropts.get("fig_enc", fig_default_enc);
  IConv cnv(enc, "UTF-8");

  if (ropts.get("fig_header", 1)) {
    std::string l;
    std::getline(s, l);
    if (l.size()<8 || l.substr(0,8) != "#FIG 3.2")
      throw Err() << "Fig: non-supported format";

    // comments after #FIG line
    while (s.get()=='#'){
      std::getline(s, l);
      // xfig 3.2.9 produces utf8 files!
      if (l == "encoding: UTF-8") cnv = IConv();
    }
    s.unget();

    read_line(s, w.orientation);
    if (w.orientation!="Landscape" && w.orientation != "Portrait"){
      std::cerr << "Fig: unknown orientation setting: " << w.orientation << "\n";
      w.orientation = "Landscape";
    }
    // WinFIG uses non-standard "Flush left"
    read_line(s, w.justification);
    if (w.justification!="Center" && w.justification != "Flush Left" &&
        w.justification != "Flush left"){
      std::cerr << "Fig: unknown justification setting: " << w.justification << "\n";
      w.justification = "Center";
    }
    read_line(s, w.units);
    if (w.units!="Metric" && w.units != "Inches"){
      std::cerr << "Fig: unknown units setting: " << w.units << "\n";
      w.units = "Metric";
    }
    read_line(s, w.papersize);
    if (w.papersize!="Letter" && w.papersize != "Legal" && w.papersize != "Ledger" &&
        w.papersize != "Tabloid" && w.papersize != "A" && w.papersize != "B" &&
        w.papersize != "C" && w.papersize != "D" && w.papersize != "E" &&
        w.papersize != "A4" && w.papersize != "A3" && w.papersize != "A2" &&
        w.papersize != "A1" && w.papersize != "A0" && w.papersize != "B5"){
      std::cerr << "Fig: unknown papersize setting: " << w.papersize << "\n";
      w.papersize = "A4";
    }
    read_line(s, w.magnification);

    read_line(s, w.multiple_page);
    if (w.multiple_page!="Single" && w.multiple_page != "Multiple"){
      std::cerr << "Fig: unknown multiple_page setting: " << w.multiple_page << "\n";
      w.multiple_page = "Single";
    }
    read_line(s, w.transparent_color);
    if (w.transparent_color<-3){
      std::cerr << "Fig: wrong transparent_color setting: " << w.transparent_color << "\n";
      w.transparent_color = -2;
    }

    // Read comments and options. Same as for FigObj,
    // using same method.
    l = read_comments(s, w.comment, cnv);

    // read resolution line
    std::istringstream s1(l);
    s1 >> w.resolution >> w.coord_system;
    if (!s1.eof()) s1 >> ws;
    if (s1.fail() || !s1.eof())
      throw Err() << "Fig: can't read fig file header";
  }

  // Read custom colors
  map<int,int> custom_cmap;
  while (s.peek()=='0'){
    std::string l;
    std::getline(s, l);
    int type, ckey, cval;
    char c;
    std::istringstream s1(l);
    s1 >> type >> ckey >> c >> hex >> cval;
    if (!s1.eof()) s1 >> ws;
    if (s1.fail() || !s1.eof() || c != '#')
      throw Err() << "Fig: bad color object: [" << l << "]";
    if (Fig::colors.find(ckey) != Fig::colors.end())
      throw Err() << "Fig: redifinition of a predefined color: [" << l << "]";
    custom_cmap.insert(make_pair(ckey, cval));
  }

  // Read objects
  while (1){
    if (s.eof()) break;
    FigObj o;
    std::string header = read_comments(s, o.comment, cnv);
    // parse object header
    int hret = read_figobj_header(o, header, custom_cmap);
    // read extra text lines if needed
    if (o.type == FIG_TXT && hret){
      if (read_text(s, o.text))
        throw Err() << "FigObj: unfinished text";
    }

    // read arrow parameters if needed
    if (o.type == FIG_POLYLINE || o.type == FIG_SPLINE || o.type == FIG_ARC){
      if (o.forward_arrow) {
        std::string arr;
        getline(s, arr);
        std::istringstream s1(arr);
        s1 >> o.farrow_type >> o.farrow_style >> o.farrow_thickness
           >> o.farrow_width >> o.farrow_height;
        if (!s1.eof()) s1 >> ws;
        if (s1.fail() || !s1.eof())
          throw Err() << "FigObj: can't read arrow parameters: [" << arr << "]";
      }
      if (o.backward_arrow) {
        std::string arr;
        getline(s, arr);
        std::istringstream s1(arr);
        s1 >> o.barrow_type >> o.barrow_style >> o.barrow_thickness
           >> o.barrow_width >> o.barrow_height;
        if (!s1.eof()) s1 >> ws;
        if (s1.fail() || !s1.eof())
          throw Err() << "FigObj: can't read arrow parameters: [" << arr << "]";
      }
    }

    // read image
    if (o.type == FIG_POLYLINE && o.sub_type == 5) {
      s >> o.image_orient >> ws;
      getline(s, o.image_file);
      if (s.fail())
        throw Err() << "FigObj: can't image parameters";
      // xfig writes <empty> for empty filenames
      if (o.image_file == "<empty>") o.image_file = "";
    }

    // read coordinates
    if (o.type == FIG_POLYLINE || o.type == FIG_SPLINE) {
      o.resize(hret);
      for (size_t i=0; i<o.size(); i++){
        s >> o[i].x >> o[i].y;
        if (!s.eof()) s >> ws;
        if (s.fail()) throw Err() << "FigObj: can't read line coordinates";
      }
    }
    // read spline factors
    if (o.type == FIG_SPLINE) {
      o.f.resize(hret);
      for (size_t i=0; i<o.f.size(); i++){
        s >> o.f[i];
        if (!s.eof()) s >> ws;
        if (s.fail()) throw Err() << "FigObj: can't read spline coordinates";
      }
    }

    if (!s.eof()) s >> ws; // empty lines after the object
    if (s.fail()) throw Err() << "Fig: can't read fig file";

    // convert text encoding
    o.text = cnv(o.text);
    w.push_back(o);
  }
}

// read from a file
void
read_fig(const std::string & fname, Fig & w, const Opt & ropts){
  std::ifstream s(fname);
  if (!s) throw Err() << "can't open file: " << fname;
  read_fig(s,w,ropts);
}


/******************************************************************/
// Writing
/******************************************************************/


/// write comments
void
write_comments(std::ostream & s, const vector<string> & comment, const IConv & cnv){
  size_t n=0;
  for (const auto & c:comment){
    if (n>99) {cerr << "fig comment contains > 100 lines! Cutting...\n"; break;}
    size_t n1=0;
    while (n1!=std::string::npos) {
      size_t n2 = c.find('\n', n1);
      size_t len = (n2==std::string::npos)? n2:n2-n1;
      auto str = c.substr(n1,len);
      n1  = (n2==std::string::npos)? n2:n2+1;
      if (str.size()>1022){
        cerr << "fig comment line is > 1022 chars! Cutting...\n";
        str.resize(1022);
      }
      s << "# " << cnv(str) << "\n";
      n++;
    }
  }
}


// write arrow information for an object (spline, polyline, arc)
void
write_arrows(std::ostream & s, const FigObj & o){
  if (o.forward_arrow)
    s << "\t" << setprecision(2)
      << o.farrow_type << " "
      << o.farrow_style << " "
      << o.farrow_thickness << " "
      << o.farrow_width << " "
      << o.farrow_height << "\n";
  if (o.backward_arrow)
    s << "\t" << setprecision(2)
      << o.barrow_type << " "
      << o.barrow_style << " "
      << o.barrow_thickness << " "
      << o.barrow_width << " "
      << o.barrow_height << "\n";
}

// Write object coordinates (spline, polyline).
// Use addpt=1 to add first point at the end (only for closed polylines),
// 0 in other cases.
void
write_coords(std::ostream & s, const FigObj & o, const int add_pt=0) {
  int n = o.size();
  for (int i=0; i<n+add_pt; i++)
    s << ((i%6==0) ? "\t":" ")
      << o[i%n].x << " " << o[i%n].y
      << ((i%6==5 || i==n+add_pt-1) ? "\n":"");

  // write spline factors
  if (o.type == FIG_SPLINE){
    int n = o.f.size();
    s << setprecision(3);
    for (int i=0; i<n; i++)
      s << ((i%6==0) ? "\t":" ")
        << o.f[i]
        << ((i%6==5 || i==n-1) ? "\n":"");
  }
}

// write text, char by char, protecting \ and converting
// characters > 127 to \nnn.
void
write_text(std::ostream & s, const string & text, bool txt7bit){
  for (string::const_iterator i = text.begin(); i!=text.end(); i++){
    if (*i == '\\') s << *i;
    if (txt7bit && (unsigned char)*i>127)
      s << '\\' << oct << setfill('0') << setw(3)
        << (int)(unsigned char)*i << dec;
    else s << *i;
  }
  s << "\\001\n";
}

/******************************************************************/
void
write_fig(ostream & s, const Fig & w, const Opt & wopts){

  bool txt7bit = wopts.get("fig_7bit", false);
  string enc = wopts.get("fig_enc", fig_default_enc);
  IConv cnv("UTF-8", enc);
  // Writing header:
  if (wopts.get("fig_header", 1)) {
    s << "#FIG 3.2\n";
    if (enc=="UTF-8") s << "#encoding: UTF-8\n";
    s   << w.orientation << "\n" << w.justification << "\n"
        << w.units << "\n" << w.papersize << "\n"
        << setprecision(2) << fixed << w.magnification << "\n"
        << setprecision(3) << w.multiple_page << "\n"
        << w.transparent_color << "\n";

    // Write comments:
    write_comments(s, w.comment, cnv);
    s << w.resolution << " " << w.coord_system << "\n";
  }


  // Looking for custom colors in objects:
  map<int,int> custom_cmap;
  for (const auto & o: w){
    color_rgbadd(o.pen_color,  custom_cmap);
    color_rgbadd(o.fill_color, custom_cmap);
  }
  // writing custom colors
  for (const auto & c:custom_cmap){
    s << "0 " << c.first << " #"
      << setbase(16) << setw(6) << setfill('0')
      << c.second << setbase(10) << "\n";
  }

  // Writing objects
  for (const auto & o:w){
    write_comments(s, o.comment, cnv);

    int pen_color  = color_rgb2fig(o.pen_color,  custom_cmap);
    int fill_color = color_rgb2fig(o.fill_color, custom_cmap);

    s << o.type << std::setprecision(3) << std::fixed;
    int add_pt;
    switch (o.type) {
      case FIG_ELLIPSE:
        if (o.size()!=1) throw Err() << "FigObj: ellipse should have 1 coordinate point";
        s << " " << o.sub_type <<" " << o.line_style << " " << o.thickness
          << " " << pen_color << " " << fill_color << " " << o.depth
          << " " << o.pen_style << " " << o.area_fill << " " << o.style_val
          << " " << o.direction << " " << std::setprecision(4) << o.angle
          << " " << o[0].x << " " << o[0].y
          << " " << o.radius_x << " " << o.radius_y << " " << o.start_x
          << " " << o.start_y << " " << o.end_x << " " << o.end_y << "\n";
        if (s.fail()) throw Err() << "FigObj: can't write ellipse object";
        break;
      case FIG_POLYLINE:
        if (o.size()<1) throw Err() << "FigObj: line should have at least 1 coordinate point";

        // If line if closed the last point should be same as first one.
        // Let's add the missing point if needed:
        add_pt = (o.is_closed() && o[o.size()-1] != o[0]) ? 1:0;

        s << " " << o.sub_type << " " << o.line_style << " " << o.thickness
          << " " << pen_color << " " << fill_color << " " << o.depth
          << " " << o.pen_style << " " << o.area_fill << " " << o.style_val
          << " " << o.join_style << " " << o.cap_style << " " << o.radius
          << " " << o.forward_arrow << " " << o.backward_arrow << " " << o.size() + add_pt << "\n";
        write_arrows(s, o);
        if (o.sub_type==5){ // image
          s << "\t" << o.image_orient << " "
            << (o.image_file == ""? "<empty>":o.image_file) << "\n";
        }
        write_coords(s, o, add_pt);
        if (s.fail()) throw Err() << "FigObj: can't write line object";
        break;
      case FIG_SPLINE:
        if (o.is_closed()  && o.size()<3) throw Err() << "FigObj: closed spline with < 3 points";
        if (!o.is_closed() && o.size()<2) throw Err() << "FigObj: spline with < 2 points";
        if (o.size()!=o.f.size()) throw Err() << "FigObj: different amount of x,y and f values in a spline";
        s << " " << o.sub_type << " " << o.line_style << " " << o.thickness
          << " " << pen_color << " " << fill_color << " " << o.depth
          << " " << o.pen_style << " " << o.area_fill << " " << o.style_val
          << " " << o.cap_style << " " << o.forward_arrow << " " << o.backward_arrow
          << " " << o.size() << "\n";
        write_arrows(s, o);
        write_coords(s, o);
        if (s.fail()) throw Err() << "FigObj: can't write spline object";
        break;
      case FIG_TXT:
        if (o.size()!=1) throw Err() << "FigObj: text should have 1 coordinate point";
        s << " " << o.sub_type << " " << pen_color << " " << o.depth
          << " " << o.pen_style << " " << o.font
          << " " << std::setprecision(3) << o.font_size
          << " " << std::setprecision(4) << o.angle << " " << o.font_flags
          << " " << std::setprecision(0) << o.height << " " << o.length
          << " " << o[0].x << " " << o[0].y << " ";
        write_text(s, cnv(o.text), txt7bit);
        if (s.fail()) throw Err() << "FigObj: can't write text object";
        break;
      case FIG_ARC:
        if (o.size()!=3) throw Err() << "FigObj: arc should have 3 coordinate point";
        // TODO: calculate center!
        s << " " << o.sub_type << " " << o.line_style << " " << o.thickness
          << " " << pen_color << " " << fill_color << " " << o.depth
          << " " << o.pen_style << " " << o.area_fill << " " << o.style_val
          << " " << o.cap_style << " " << o.direction
          << " " << o.forward_arrow << " " << o.backward_arrow
          << " " << o.center_x << " " << o.center_y
          << " " << o[0].x << " " << o[0].y
          << " " << o[1].x << " " << o[1].y
          << " " << o[2].x << " " << o[2].y << "\n";
        write_arrows(s, o);
        if (s.fail()) throw Err() << "FigObj: can't write arc object";
        break;
      case FIG_COMPOUND:
        if (o.size()!=2) throw Err() << "FigObj: compound should have 2 coordinate point";
        s << " " << o[0].x << " " << o[0].y
          << " " << o[1].x << " " << o[1].y << "\n";
        if (s.fail()) throw Err() << "FigObj: can't write compound object";
        break;
      case FIG_END_COMPOUND:
        s << "\n";
        if (s.fail()) throw Err() << "FigObj: can't write compound end object";
        break;
      default:
        throw Err() << "FigObj: unknown object type: " << o.type;
    }
  }
}

void
write_fig(const std::string & fname, const Fig & w, const Opt & wopts){
  std::ofstream s(fname);
  if (!s) throw Err() << "can't open file: " << fname;
  write_fig(s, w, wopts);
}
