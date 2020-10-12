#ifndef READ_CONF_H
#define READ_CONF_H

#include <string>
#include "opt/opt.h"

/*
 Read options from a simple config file.
 All known options should be mentioned in `known` list.
 If `should_exist=true` and file is missing then error will be thrown.
*/

Opt read_conf(const std::string & fname,
              std::list<std::string> known,
              bool should_exist=false);

#endif