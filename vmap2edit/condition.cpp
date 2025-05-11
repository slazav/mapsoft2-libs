#include <regex>
#include "err/err.h"
#include "condition.h"

/***********************************************************/

bool
VMap2cond::eval(const VMap2obj & o) const{
  bool res = true;
  for (const auto s: *this)
    if (res!=false) res = res && eval_sequence(s,o);
  return res;
}

bool
VMap2cond::eval_sequence(const std::vector<std::string> & s, const VMap2obj & o){
  if (s.size()==0) throw Err() << "empty condition sequence";

  bool res = true;
  bool cond = true; // are we reanding next condition?
  bool is_and = true; // was it "and" or "or" operator

  for (size_t i = 0; i < s.size(); ++i){
    // read condition
    if (cond){
      if (is_and  && res!=false) res = res && eval_single(s[i],o);
      if (!is_and && res!=true) res = res || eval_single(s[i],o);
      cond = false;
      continue;
    }
    // read and/or before next condition
    if (s[i] != "and" && s[i] != "or")
      throw Err() << "\"and\" or \"or\" expected";
    is_and = (s[i] == "and");
    cond = true;
  }
  if (cond) throw Err() << "unterminated condition sequence";
  return res;
}

bool
test_7bit(const std::string & s){
  for (const auto & c: s) if (c & 0x80) return false;
  return true;
}

int
test_string(const std::string & op, const std::string & lval, const std::string & rval){
  if (op == "==") return lval==rval;
  if (op == "!=") return lval!=rval;
  if (op == "=~") return std::regex_search(lval, std::regex(rval));
  if (op == "!~") return !std::regex_search(lval, std::regex(rval));
  return -1;
}

int
test_int(const std::string & op, const int l, const int r){
  if (op == "==") return l==r;
  if (op == "!=") return l!=r;
  if (op == ">")  return l>r;
  if (op == "<")  return l<r;
  if (op == ">=") return l>=r;
  if (op == "<=") return l<=r;
  return -1;
}

bool
VMap2cond::eval_single(const std::string & c, const VMap2obj & o){

  // true/false
  if (c == "true") return true;
  if (c == "false") return false;

  // if name is in 7-bit encoding
  if (c == "name_7bit") return test_7bit(o.name);
  if (c == "name_8bit") return !test_7bit(o.name);

  // find operator
  std::vector<std::string> ops = {"==", "!=", ">=", "<=", ">", "<", "=~", "!~"};
  size_t p1(std::string::npos), p2(std::string::npos);
  for (const auto op: ops){
    p1 = c.find(op);
    if (p1==std::string::npos) continue;
    p2 = p1+op.size();
    break;
  }

  // process conditions <lval> <op> <rval>
  if (p1!=std::string::npos){
    auto lval = c.substr(0,p1);
    auto op   = c.substr(p1,p2-p1);
    auto rval = c.substr(p2);

    // type/ref_type conditions
    if (lval == "type" || lval == "ref_type"){

      uint32_t t,c;
      uint32_t l,r;

      if (lval == "type"){ t = o.type; c = o.get_class();}
      else {t = o.ref_type; c = o.get_ref_class();}

      if      (rval == "point") {l = c; r = VMAP2_POINT;}
      else if (rval == "line")  {l = c; r = VMAP2_LINE;}
      else if (rval == "area")  {l = c; r = VMAP2_POLYGON;}
      else if (rval == "text")  {l = c; r = VMAP2_TEXT;}
      else {l = t; r = o.make_type(rval);}
      if (op == "==") return l==r;
      if (op == "!=") return l!=r;
    }

    // name
    else if (lval == "name"){
      auto res = test_string(op, o.name, rval);
      if (res>=0) return res;
    }

    // npts
    else if (lval == "npts"){
      size_t l = o.npts(), r = str_to_type<size_t>(rval);
      auto res = test_int(op, l, r);
      if (res>=0) return res;
    }

    // nseg
    else if (lval == "nseg"){
      size_t l = o.size(), r = str_to_type<size_t>(rval);
      auto res = test_int(op, l, r);
      if (res>=0) return res;
    }

    // opts
    else if (lval.find("opts[") == 0 && lval.rfind("]") == lval.size()-1){
      auto opt = lval.substr(5,lval.size()-6);
      auto res = test_string(op, o.opts.get(opt), rval);
      if (res>=0) return res;
    }

  }


  throw Err() << "unknown condition: " << c;
}
