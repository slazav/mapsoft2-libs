#include "image.h"

// image << operator
std::ostream & operator<< (std::ostream & s, const Image & i) { return i.print(s); }

