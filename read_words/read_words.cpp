#include "err/err.h"
#include "read_words.h"

std::vector<std::string> read_words(
      std::istream & ss, int line_num[2], const bool lc) {

  bool first = true;
  std::string str;
  std::vector<std::string> ret;
  bool quote1=false, quote2=false, comment=false;
  bool qq=false; // empty strings are added if they were quoted.

  try {

    // read stream char by char
    for (char c; ss.get(c); !ss.eof()){

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
        switch (c) {
          //protected \n works as word separator:
          case '\n':
            if (line_num) line_num[1]++;
            if (str!="" || qq) ret.push_back(str);
            str=""; qq=false;
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
              c = (c<<3) + (c1-'0');
            }
            break;
          // hex code, read exactly two symbols
          case 'x':{
            char c1,c2;
            ss.get(c1);
            ss.get(c2);
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
         if (str!="" || qq) ret.push_back(str);
         str=""; qq=false;
         continue;
      }

      // to lower case if needed
      if (lc && c>='A' && c<='Z')  c = tolower(c);

      // append normal characters
      add_char:
      str += c;
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
