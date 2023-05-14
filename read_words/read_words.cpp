#include "err/err.h"
#include "read_words.h"

std::vector<std::string> read_words(
      std::istream & ss, int line_num[2],
      const bool lc, const bool raw) {

  bool first = true;
  std::string str;
  std::vector<std::string> ret;
  bool quote1=false, quote2=false, comment=false;
  bool qq=false; // empty strings are added if they were quoted.

  try {

    // read stream char by char
    for (char c; ss.get(c); !ss.eof()){
      if (raw) str += c;

      if (c == '\n' && line_num) line_num[1]++;

      // end of line
      if (c == '\n' && !quote1 && !quote2) {
        comment=false;
        if (str!="" || qq) ret.push_back(str);
        str=""; qq=false;
        if (ret.size()) return ret;
        else continue;
      }

      // comments -- until end of line
      if (c == '#' && !quote1 && !quote2) comment=true;
      if (comment) continue;

      // escape character (all chars including newline can be escaped!)
      if (c == '\\') {
        ss.get(c);
        if (raw) str += c;
        switch (c) {
          //protected \n works as word separator:
          case '\n':
            if (line_num) line_num[1]++;
            if ((str!="" || qq) && !raw){
              ret.push_back(str);
              str="";
            }
            qq=false;
            continue;
          // standard ANSI escape sequences + '#', space, tab:
          case 'a': c = '\a'; break;
          case 'b': c = '\b'; break;
          case 'f': c = '\f'; break;
          case 'n': c = '\n'; break;
          case 'r': c = '\r'; break;
          case 't': c = '\t'; break;
          case 'v': c = '\v'; break;
          case '\\':
          case '\'': case '\"':
          case '?':  case '#':
          case ' ':  case '\t':
            break;
          // oct code
          case '0': case '1':
          case '2': case '3':
          case '4': case '5':
          case '6': case '7':
            // read two more symbols if they are numbers
            c-='0';
            for (int i=0; i<2; ++i){
              char c1;
              ss.get(c1);
              if (c1<'0' || c1>'7'){
                ss.unget();
                break;
              }
              if (raw) str += c1;
              c = (c<<3) + (c1-'0');
            }
            break;
          // hex code, read exactly two symbols
          case 'x':{
            char c1,c2;
            ss.get(c1);
            ss.get(c2);
            if (raw) {
              str += c1;
              str += c2;
            }
            if (c1>='0' && c1<='9') c1-='0';
            else if (c1>='A' && c1<='F') c1-='A'-10;
            else if (c1>='a' && c1<='f') c1-='a'-10;
            else throw Err() << "bad hex escape sequence";
            if (c2>='0' && c2<='9') c2-='0';
            else if (c2>='A' && c2<='F') c2-='A'-10;
            else if (c2>='a' && c2<='f') c2-='a'-10;
            else throw Err() << "bad hex escape sequence";
            c = (c1<<4) + c2;
            break;
          }
          default:
            throw Err() << "bad escape sequence";
        }
        // add character
        goto add_char;
      }

      // quote
      if (c == '"'  && !quote2) {quote1=!quote1; qq=true; continue; }
      if (c == '\'' && !quote1) {quote2=!quote2; qq=true; continue; }


      // space -- word separaters unless quoted
      if (!quote1 && !quote2 && (c == ' ' || c == '\t')){
         if ((str!="" || qq) && !raw){
           ret.push_back(str);
           str="";
         } qq=false;
         continue;
      }

      // to lower case if needed
      if (lc && c>='A' && c<='Z')  c = tolower(c);

      // append normal characters
      add_char:
      if (!raw) str += c;
      if (first && line_num) {line_num[0] = line_num[1]+1; first=false;}
    }

    // at end of input we have unclosed quote:
    if (quote1 || quote2) throw Err() << "Unclosed quote";

  }
  catch (Err & e){
    if (line_num) e << " in line " << line_num[1];
    throw;
  }

  if ((str!="") | qq) ret.push_back(str);
  return ret;
}

std::string
join_words(const std::vector<std::string> & words){
  std::string ret;
  for (auto const & w:words) {

    auto str(w);
    std::string::size_type pos;

    pos = 0u;
    while((pos = str.find("\\", pos)) != std::string::npos){
      str.insert(pos, 1, '\\');
      pos += 2;
    }

    pos = 0u;
    while((pos = str.find("\"", pos)) != std::string::npos){
      str.insert(pos, 1, '\\');
      pos += 2;
    }

    pos = 0u;
    while((pos = str.find("\'", pos)) != std::string::npos){
      str.insert(pos, 1, '\\');
      pos += 2;
    }

    pos = 0u;
    while((pos = str.find("#", pos)) != std::string::npos){
      str.insert(pos, 1, '\\');
      pos += 2;
    }
    bool qq = (str.find(' ') != std::string::npos ||
               str.find('\t') != std::string::npos ||
               str.find('\n') != std::string::npos ||
               str.size()==0);

    if (ret.size()) ret += ' ';
    if (qq) ret+='\"';
    ret+=str;
    if (qq) ret+='\"';
  }
  return ret;
}

std::string
unquote_words(const std::string & in){
  std::string ret;
  std::istringstream ss(in);
  while (1){
    auto words = read_words(ss);
    if (words.size()==0) break;
    for (auto const & w:words){
      if (ret.size()) ret+=" ";
      ret += w;
    }
  }
  return ret;
}

/***********************************/

void
read_words_defs::apply(std::string & str) const{
  while (1) {
    auto n1 = str.find("${");
    if (n1 == std::string::npos) break;
    auto n2 = str.find("}", n1);
    auto v = find(str.substr(n1+2, n2-n1-2));
    if (v == end()) throw Err() << "Undefined variable: " << str.substr(n1, n2-n1+1);
    str.replace(n1,n2-n1+1, v->second);
  }
}

bool
read_words_stdcmds(std::vector<std::string> & words, read_words_defs & defs, std::deque<bool> & ifs){

  // endif command
  if (words[0] == "endif"){
    if (ifs.size()<1) throw Err() << "unexpected endif command";
    ifs.pop_back();
    return 1;
  }
  // else command
  if (words[0] == "else"){
    if (ifs.size()<1) throw Err() << "unexpected else command";
    ifs.back() = !ifs.back();
    return 1;
  }

  // check if conditions
  bool skip = false;
  for (auto const & c:ifs)
    if (c == false) {skip = true; break;}
  if (skip) return 1;

  // apply definitions
  defs.apply(words);

  // if command
  if (words[0] == "if"){
    if (words.size() == 4 && words[2] == "=="){
      ifs.push_back(words[1] == words[3]);
    }
    else if (words.size() == 4 && words[2] == "!="){
      ifs.push_back(words[1] != words[3]);
    }
    else
      throw Err() << "wrong if syntax";
    return 1;
  }
  // ifdef command
  if (words[0] == "ifdef"){
    if (words.size() != 2)
      throw Err() << "wrong ifdef syntax";
    ifs.push_back(defs.count(words[1])>0);
    return 1;
  }
  // ifndef command
  if (words[0] == "ifndef"){
    if (words.size() != 2)
      throw Err() << "wrong ifndef syntax";
    ifs.push_back(defs.count(words[1])==0);
    return 1;
  }

  // define <key> <value> -- define a variable
  if (words[0] == "define") {
    if (words.size()!=3) throw Err() << "define: arguments expected: <key> <value>";
    defs.define(words[1], words[2]);
    return 1;
  }
  // define_if_undef <key> <value> -- define a variable if it is not defined
  if (words[0] == "define_if_undef") {
    if (words.size()!=3) throw Err() << "define_if_undef: arguments expected: <key> <value>";
    if (defs.count(words[1])==0) defs.define(words[1], words[2]);
    return 1;
  }

  return 0;
}
