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
  assert_err(nom_to_range_fi("v51a"), "nom_to_range_fi: can't parse name: \"v51a\": third digit 1..4 expected");

  assert_eq((iRect)nom_to_range_fi("v5"), 1000*iRect(500,7530, 192,96));

  assert_eq((iRect)nom_to_range_fi("v51"), 1000*iRect(500,7530, 96,48));
  assert_eq((iRect)nom_to_range_fi("v52"), 1000*iRect(500,7578, 96,48));

  assert_eq((iRect)nom_to_range_fi("v53"), 1000*iRect(596,7530, 96,48));
  assert_eq((iRect)nom_to_range_fi("v54"), 1000*iRect(596,7578, 96,48));

  assert_eq((iRect)nom_to_range_fi("v531"), 1000*iRect(596,7530, 48,24));
  assert_eq((iRect)nom_to_range_fi("v5311"), 1000*iRect(596,7530, 24,12));
  assert_eq((iRect)nom_to_range_fi("v5311a"), 1000*iRect(596,7530, 6,6));
  assert_eq((iRect)nom_to_range_fi("v5311b"), 1000*iRect(596,7536, 6,6));
  assert_eq((iRect)nom_to_range_fi("v5311c"), 1000*iRect(602,7530, 6,6));
  assert_eq((iRect)nom_to_range_fi("v5311e"), 1000*iRect(608,7530, 6,6));
  assert_eq((iRect)nom_to_range_fi("v5311h"), 1000*iRect(614,7536, 6,6));
  assert_eq((iRect)nom_to_range_fi("v5311l"), 1000*iRect(596,7530, 12,12));
  assert_eq((iRect)nom_to_range_fi("v5311r"), 1000*iRect(608,7530, 12,12));

  assert_err(nom_to_range_fi("v5311i"), "nom_to_range_fi: can't parse name: \"v5311i\": letter A..H (or R, or L) expected");

  assert_eq((iRect)nom_to_range_fi("v5311e1"), 1000*iRect(608,7530, 3,3));
  assert_eq((iRect)nom_to_range_fi("v5311e2"), 1000*iRect(608,7533, 3,3));

  assert_err(nom_to_range_fi("v5311a1a"), "nom_to_range_fi: can't parse name: \"v5311a1a\": extra symbols after the name");

  //
  nom_scale_fi_t sc;

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5").cnt(), SC_FI_200k), "V5");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k5").cnt(), SC_FI_200k), "K5");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k2").cnt(), SC_FI_200k), "K2");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("x6", &sc).cnt(), SC_FI_200k), "X6");
  assert_eq(sc, SC_FI_200k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v51").cnt()), "V51");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v52").cnt()), "V52");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v53").cnt()), "V53");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v54",&sc).cnt()), "V54");
  assert_eq(sc, SC_FI_100k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("k21").cnt()), "K21");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k22").cnt()), "K22");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k23").cnt()), "K23");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k24", &sc).cnt()), "K24");
  assert_eq(sc, SC_FI_100k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v511").cnt(), SC_FI_50k), "V511");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v522").cnt(), SC_FI_50k), "V522");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v533").cnt(), SC_FI_50k), "V533");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v544", &sc).cnt(), SC_FI_50k), "V544");
  assert_eq(sc, SC_FI_50k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5111").cnt(), SC_FI_25k), "V5111");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5222").cnt(), SC_FI_25k), "V5222");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333").cnt(), SC_FI_25k), "V5333");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5444", &sc).cnt(), SC_FI_25k), "V5444");
  assert_eq(sc, SC_FI_25k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5111A").cnt(), SC_FI_10k), "V5111A");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5222B").cnt(), SC_FI_10k), "V5222B");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333C").cnt(), SC_FI_10k), "V5333C");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5444D").cnt(), SC_FI_10k), "V5444D");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5111E").cnt(), SC_FI_10k), "V5111E");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5222F").cnt(), SC_FI_10k), "V5222F");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333G").cnt(), SC_FI_10k), "V5333G");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5444H", &sc).cnt(), SC_FI_10k), "V5444H");
  assert_eq(sc, SC_FI_10k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333C1").cnt(), SC_FI_5k), "V5333C1");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333C2").cnt(), SC_FI_5k), "V5333C2");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333C3").cnt(), SC_FI_5k), "V5333C3");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333C4", &sc).cnt(), SC_FI_5k), "V5333C4");
  assert_eq(sc, SC_FI_5k);


  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5l").cnt(), SC_FI_H200k), "V5L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k5r").cnt(), SC_FI_H200k), "K5R");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k2L").cnt(), SC_FI_H200k), "K2L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("x6R", &sc).cnt(), SC_FI_H200k), "X6R");
  assert_eq(sc, SC_FI_H200k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v51l").cnt(), SC_FI_H100k), "V51L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v52r").cnt(), SC_FI_H100k), "V52R");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v53L").cnt(), SC_FI_H100k), "V53L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v54R",&sc).cnt(), SC_FI_H100k), "V54R");
  assert_eq(sc, SC_FI_H100k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("k21l").cnt(), SC_FI_H100k), "K21L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k22r").cnt(), SC_FI_H100k), "K22R");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k23L").cnt(), SC_FI_H100k), "K23L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("k24R", &sc).cnt(), SC_FI_H100k), "K24R");
  assert_eq(sc, SC_FI_H100k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v511l").cnt(), SC_FI_H50k), "V511L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v522r").cnt(), SC_FI_H50k), "V522R");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v533L").cnt(), SC_FI_H50k), "V533L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v544R", &sc).cnt(), SC_FI_H50k), "V544R");
  assert_eq(sc, SC_FI_H50k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5111l").cnt(), SC_FI_H25k), "V5111L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5222r").cnt(), SC_FI_H25k), "V5222R");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333L").cnt(), SC_FI_H25k), "V5333L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5444R", &sc).cnt(), SC_FI_H25k), "V5444R");
  assert_eq(sc, SC_FI_H25k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5111Al").cnt(), SC_FI_H10k), "V5111AL");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5222Br").cnt(), SC_FI_H10k), "V5222BR");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333Cl").cnt(), SC_FI_H10k), "V5333CL");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5444Dr").cnt(), SC_FI_H10k), "V5444DR");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5111El").cnt(), SC_FI_H10k), "V5111EL");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5222Fr").cnt(), SC_FI_H10k), "V5222FR");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333GL").cnt(), SC_FI_H10k), "V5333GL");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5444HR", &sc).cnt(), SC_FI_H10k), "V5444HR");
  assert_eq(sc, SC_FI_H10k);

  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333C1l").cnt(), SC_FI_H5k), "V5333C1L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333C2r").cnt(), SC_FI_H5k), "V5333C2R");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333C3L").cnt(), SC_FI_H5k), "V5333C3L");
  assert_eq(pt_to_nom_fi(nom_to_range_fi("v5333C4R", &sc).cnt(), SC_FI_H5k), "V5333C4R");
  assert_eq(sc, SC_FI_H5k);


  }
  catch (Err & E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
