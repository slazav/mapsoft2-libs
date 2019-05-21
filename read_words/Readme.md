## read_words

Read one line from the stream and extract words, separated by spaces.
- words are splitted by ' ' or '\t' symbols,
- comments (everything from # symbol to end of the line) are skipped,
- empty lines are skipped,
- words can be quoted by " or ',
- any symbol (including newline) can be escaped by '\',

`std::vector<std::string> read_words(std::istream & ss);`

------------
## Changelog:

2019.05.02 V.Zavjalov 1.0:
- First version
