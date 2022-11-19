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
      string key, val;
      if (p2<p1) opts.put(i.substr(p1,-1), 1);
      else opts.put(i.substr(p1,p2-p1), i.substr(p2+1,-1));
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
  for (const auto & i:opts){
    comm.push_back(string("\\") + i.first + "=" + i.second);
  }
}
void fig_set_opts(Fig & f, const Opt & opts) {fig_set_opts(f.comment, opts);}
void fig_set_opts(FigObj & o, const Opt & opts) {fig_set_opts(o.comment, opts);}


void
fig_add_opt(vector<string> & comm, const string & key, const string & val){
  comm.push_back(string("\\") + key + "=" + val);}
void fig_add_opt(Fig & f, const string & key, const string & val) {
  fig_add_opt(f.comment, key, val);}
void fig_add_opt(FigObj & o, const string & key, const string & val) {
  fig_add_opt(o.comment, key, val);}
