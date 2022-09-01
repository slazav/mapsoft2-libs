#ifndef READ_WORDS_H
#define READ_WORDS_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

///\addtogroup libmapsoft
///@{

/*
 Read one line from the stream and extract words, separated by spaces.
 - Comments (everything from # symbol to end of the line) are skipped.
 - Empty lines are skipped.
 - Words are splitted by ' ' or '\t' symbols.
 - Words can be quoted by " or '.
 - Any symbol (including newline) can be escaped by '\'.
 - Empty vector is returned only at the end of input.
 - If line_num parameter is not NULL, it should be initialized
   by 0,0 before the first call. Then line number of the first word and
   number of the last read wline is returned there.
 - If lc parameter is true then all characters are converted to lower case.
 - If raw parameter is true, read everything as a single string, without
   processing special characters. This is useful if one wants to read
   a single "line" which can be sent to another program which uses read_words
   to read it.
*/
std::vector<std::string> read_words(
  std::istream & ss,
  int line_num[2] = NULL,
  const bool lc = false,
  const bool raw = false
);


/* Inverse operation: join all "words" to have string readable for read_words().
- Characters `\`, `#`, `'`, and `"` are protected with `\`.
- If a word contais `  `, `\t` or `\n` or it is empty, it is surrounded with `"`.
- All words are joined with " " character between them.
*/
std::string join_words(const std::vector<std::string> & words);

/*
  Read text by read_words and join everything using space.
  "a b" \'c --> a b 'c
*/
std::string unquote_words(const std::string & in);

/***********************************/
// Define/substitute variables
// (copy from cryoblocks project)

class read_words_defs : public std::map<std::string, std::string> {
public:
  // initialize from map<str,str>, or Opt
  read_words_defs(const std::map<std::string, std::string> & map):
    std::map<std::string, std::string>(map){}
  read_words_defs() {}

  void define(const std::string & name, const std::string & value){
    emplace(name, value); }

  void apply(std::string & str) const;

  void apply(std::vector<std::string> & vs) const {
     for (auto &s:vs) apply(s); }
};

///@}
#endif
