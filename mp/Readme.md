## Mapsoft2 can read and write MP files.


Format documentation:
http://magex.sourceforge.net/doc/cGPSmapper-UsrMan-v02.4.pdf

#### Reading:

- Section names and keywords are case insensitive.

- Header (`[IMG ID]` section) is required.

- In the header following fields are supported: `ID`, `Name`,
  `LblCoding`, `Codepage`, `Elevation`, `TreSize`, `RgnLimit`,
  `PreProcess`, `Levels`, `Level`. Unknown fields are written to options.

- Comments before the header are converted to UTF-8 according to the
  `Codepage` field and attached to the MP object.

- Only `[POI]`, `[POLYLINE]`, `[POLYGON]`,
  `[RGN10]`, `[RGN20]`, `[RGN30]`, and `[RGN40]`
  sections are suppored, other sections are skipped.

- For objects only `Type`, `Data#` (or `Origin#`), `Label`,
  and `EndLevel` fields are supported.

- There was a non-standard field `Direction` used in mapsoft1.
  On input coordinates of objects with `Direction == 2`
  are saved in reversed order. On output this field is not used.

- Comments before object sections are converted to UTF-8 according to the
  `Codepage` field and attached to corresponding objects. Object label is
  also converted fo UTF-8.

- Unknown object parameters are read to Opt structure

- When reading a few files the header will be overwritten and
  objects will be added.

#### Writing:

- Only sections `[IMG ID]`, `[POI]`, `[POLYLINE]` and `[POLYGON]` are written.



