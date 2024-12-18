///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include "err/assert_err.h"
#include "geo_nom_fi.h"

int
main(){
  try {

  assert_err(nom_to_range_fi(""), "nom_to_range_fi: can't parse name: \"\": letter k..n or p..x expected");
  assert_err(nom_to_range_fi("o21"), "nom_to_range_fi: can't parse name: \"o21\": letter k..n or p..x expected");
  assert_err(nom_to_range_fi("j21"), "nom_to_range_fi: can't parse name: \"j21\": letter k..n or p..x expected");
  assert_err(nom_to_range_fi("y21"), "nom_to_range_fi: can't parse name: \"y21\": letter k..n or p..x expected");
  assert_err(nom_to_range_fi("v11"), "nom_to_range_fi: can't parse name: \"v11\": first digit 2..6 expected");
  assert_err(nom_to_range_fi("v71"), "nom_to_range_fi: can't parse name: \"v71\": first digit 2..6 expected");
  assert_err(nom_to_range_fi("v50"), "nom_to_range_fi: can't parse name: \"v50\": second digit 1..4 expected");
  assert_err(nom_to_range_fi("v55"), "nom_to_range_fi: can't parse name: \"v55\": second digit 1..4 expected");
  assert_err(nom_to_range_fi("v51a"), "nom_to_range_fi: can't parse name: \"v51a\": extra symbols after the name");

  assert_eq((iRect)nom_to_range_fi("v51"), 1000*iRect(500,7530, 96,48));
  assert_eq((iRect)nom_to_range_fi("v52"), 1000*iRect(500,7578, 96,48));

  assert_eq((iRect)nom_to_range_fi("v53"), 1000*iRect(596,7530, 96,48));
  assert_eq((iRect)nom_to_range_fi("v54"), 1000*iRect(596,7578, 96,48));

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v51").cnt()), "v51");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v52").cnt()), "v52");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v53").cnt()), "v53");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v54").cnt()), "v54");

  assert_eq(pt_to_nom_fi(nom_to_range_fi("k21").cnt()), "k21");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k22").cnt()), "k22");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k23").cnt()), "k23");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k24").cnt()), "k24");

  }
  catch (Err & E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
