## Vmap2 editor. Read text file with instructions and modify vmap2.

### File structure

if <conditions> then <command> <arguments>

if <conditions>
<command> <arguments>
...
endif

Commands are applied to each object of VMap2 structure according with conditions.

### Conditions

Each condition is a word (spaces should be quoted).
* `true` -- always true
* `false` -- always false

* `name_7bit` -- true if name contains only 7-bit characters
* `name_8bit` -- true if name contains 8-bit characters

* `type==(point|line|area|text)` -- true for any object of specified class
* `type!=(point|line|area|text)` -- false for any object of specified class
* `type==<type>` -- true for any object of the specified type
* `type!=<type>` -- false for any object of the specified type

* `ref_type==(point|line|area|text)` -- true for any object with ref_type of specified class
* `ref_type!=(point|line|area|text)` -- false for any object with ref_type of specified class
* `ref_type==<type>` -- true for any object with the specified ref_type
* `ref_type!=<type>` -- false for any object with the specified ref_type

* `name==<name>`, `name!=<name>`, `name=~<name>`, `name!~<name>` -- conditions for object names
* `npts==<num>`, `npts!=<num>`, `npts&lt;<num>`, `npts&gt;<num>`, `npts&lt;=<num>`, `npts&gt;=<num>` -- conditions
* `nseg==<num>`, `nseg!=<num>`, `nseg&lt;<num>`, `nseg&gt;<num>`, `nseg&lt;=<num>`, `nseg&gt;=<num>` -- conditions

* `opts[<key>]==<val>`, `opts[<key>]!=<val>`, `opts[<key>]=~<val>`, `opts[<key>]!~<val>` -- conditions for an option in the object

Conditions can be joined with AND or OR word

### Commands

* `delete` -- delete object
* `print <msg>` -- print message. ${name}, ${type}, etc. will be replaced by object name, type, etc.
* `set_type <new type>` -- change object type
* `set_ref_type <new type>` -- change object ref_type
* `set_scale <scale>` -- set object scale
* `set_angle <angle>` -- set object angle
* `set_name <new name>` -- change object name
* `re_name <regex> <replace>` -- find and replace regex in object name
* `tr_name <old_name> <new name>` -- equivalent to `if name==<old_name> then set_name <new_name>`
* `crop_rect <rectangle>` -- crop objects to a rectangle
* `crop_nom <name>` -- crop objects to nomenclature name
* `crop_nom_fi <name>` -- crop objects to nomenclature name
* `set_alt_name <DEM filder>` -- set object name using altitude from DEM data
* `move_ends <dist[m]> <type1> ...` -- move object ends towards nearest point or segment
* `rem_short <min_npts> <min_len[m]>` -- remove segments shorter then n points (<0 for no filtering) or d distance (<0 for no filtering)
* `rem_dup_pts <dist[m]>` -- remove duplicated points
