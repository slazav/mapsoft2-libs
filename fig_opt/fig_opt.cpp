#include <map>
#include <string>
#include <vector>
#include <iomanip>

#include "err/err.h"
#include "fig_opt.h"

using namespace std;

Opt
fig_get_opts(const vector<string> & comment){
  Opt opts;
  for (const auto & i:comment){
    if (i.size()>0 && i[0] == '\\'){
      int p1 = 1;
      int p2 = i.find('=');
      if (p2<p1){
        opts.put(i.substr(p1,-1), 1);
      }
      else{
        auto key = i.substr(p1,p2-p1);
        auto val = i.substr(p2+1,-1);
        if (opts.exists(key)) opts[key] += val;
        else opts.put(key, val);
      }
    }
  }
  return opts;
}
Opt fig_get_opts(const Fig & f) { return fig_get_opts(f.comment); }
Opt fig_get_opts(const FigObj & o) { return fig_get_opts(o.comment); }


void fig_del_opts(vector<string> & comment){
  Opt ret;
  for (auto i = comment.begin(); i!= comment.end(); ++i){
    if (i->size()>0 && (*i)[0] == '\\') comment.erase(i--);
  }
}
void fig_del_opts(Fig & f){ fig_del_opts(f.comment); }
void fig_del_opts(FigObj & o){ fig_del_opts(o.comment); }


void
fig_set_opts(vector<string> & comm, const Opt & opts){
  fig_del_opts(comm);
  for (const auto & i:opts)  fig_add_opt(comm, i.first, i.second);
}
void fig_set_opts(Fig & f, const Opt & opts) {fig_set_opts(f.comment, opts);}
void fig_set_opts(FigObj & o, const Opt & opts) {fig_set_opts(o.comment, opts);}


void
fig_add_opt(vector<string> & comm, const string & key, const string & val){
  auto s = string("\\") + key + "=" + val;
  // split long options (xfig limitation for line lengths)
  while (s.size()>1021) {
    auto s1 = s.substr(0,1021);
    comm.push_back(s1);
    s = string("\\") + key + "=" + s.substr(1021);
  }
  comm.push_back(s);
}
void fig_add_opt(Fig & f, const string & key, const string & val) {
  fig_add_opt(f.comment, key, val);}
void fig_add_opt(FigObj & o, const string & key, const string & val) {
  fig_add_opt(o.comment, key, val);}
