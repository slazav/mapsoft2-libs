#ifndef VMAP2EDIT_H
#define VMAP2EDIT_H

/*
## Vmap2 editor. Read text file with instructions and modify vmap2.

### File structure

There are 4 statement types in the file:

* IF: starts with "if" word in the beginning
  of a line. Contains a few condition words and ends either with
  end of the line or with "then" word followed by a COMMAND.
  IF statement can only follow a COMMAND or be the first statement
  in the file.

* AND: starts with "and" word in the beginning
  of a line. All other words in the line are conditions.
  Can not follow a COMMAND or be first statement in the file.

* OR: starts with "or" word in the beginning
  of a line. All other words in the line are conditions.
  Can not follow a COMMAND or be first statement in the file.

* COMMAND: Any other line, or words after IF statement.
  Contains a few words (command and its arguments). Can appear
  in every place.

Example:
```
IF <cond1> <cond2> ... THEN <command1> <args> ...
<command2> <args>

IF <cond3> <cond4> ... THEN
AND <cond5> <cond6> ...
OR <cond7> <cond8> ...
<command3> <args>
<command4> <args>
```

Conditions are calculated in the following way:
* Global condition is cleared in the beginning of IF statement.
* Result of multiple conditions inside IF, AND, OR statement
  is caluclated as a conjunction of them (always logical "and").
* Then result is applied to a global condition with logical "and" for
  AND statement and logical "or" for "OR" statement.

In the example above commands 1 and 2 are executed if
<cond1> && <cond2> is true, commands 3 and 4 are executed if
(<cond1> && <cond2> && <cond5> && <cond6>) || (<cond7> && <cond8>) is true.

All statements are applied to each object of VMap2 structure. Commands
can modify the object.

### Conditions

Each condition is a word (without spaces, or quoted with quotes).
* `type==(point|line|area|text)` -- true for any object of specified class
* `type!=(point|line|area|text)` -- false for any object of specified class
* `type==<type>` -- true for any object of the specified type
* `type!=<type>` -- false for any object of the specified type

* `ref_type==(point|line|area|text)` -- true for any object with ref_type of specified class
* `ref_type!=(point|line|area|text)` -- false for any object with ref_type of specified class
* `ref_type==<type>` -- true for any object with the specified ref_type
* `ref_type!=<type>` -- false for any object with the specified ref_type


### Commands

* `delete` -- delete object
* `set_type <new type>` -- change object type
* `set_ref_type <new type>` -- change object ref_type
* `crop_rect <rectangle>` -- crop objects to a rectangle
* `crop_nom <name>` -- crop objects to nomenclature name
* `crop_nom_fi <name>` -- crop objects to nomenclature name

*/

#include <string>
#include "vmap2/vmap2.h"

// Read filter file, apply to vmap
void vmap2edit(VMap2 & vmap, const std::string & fname);

// Calculate conditions for an object, return true if all are valid
bool calc_cond(const std::vector<std::string> & cond, const VMap2obj & o);

// Run a command for an object. Return true if object has been modified.
void run_cmd(const std::vector<std::string> & cmd, VMap2 & vmap, uint32_t id, VMap2obj & o);

#endif
