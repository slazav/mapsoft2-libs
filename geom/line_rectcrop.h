#ifndef LINE_RECTCROP_H
#define LINE_RECTCROP_H

#include "multiline.h"
#include "rect.h"

///\addtogroup libmapsoft
///@{

/// Crop a line by a rectangle; line connectivity is kept but extra segments
/// can appear on crop bounds. if closed=true then segment between ends is
/// also processed. Return true if the line was modified.
bool rect_crop(const dRect & cutter, dLine & line, bool closed=false);

// Split result of rect_crop into segments.
dMultiLine rect_split_cropped(const dRect & cutter,
  const dLine & cropped, bool closed=false);


// Crop MultiLine to a cutter rectangle
dMultiLine rect_crop_multi(const dRect & cutter,
  const dMultiLine & obj, bool closed=false);

///@}
#endif
