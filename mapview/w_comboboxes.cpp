#include "w_comboboxes.h"

CBProj::CBProj(): SimpleCombo(){
  const pair_t data_list[] = {
    pair_t("WGS",    "WGS"),
    pair_t("EWEB",   "mercator"),
    pair_t("WEB",    "spherical web mercator"),
    pair_t("FI",     "KKJ (Finland)"),
    pair_t("ETRS89", "ETRS89 (Finland)"),
    pair_t("GB",     "EPSG:27700 (Great Britain)"),
    pair_t("CH",     "CH (Switzerland)"),
    pair_t("SU",     "Pulkovo-1942 (USSR), m"),
    pair_t("SU_LL",  "Pulkovo-1942 (USSR), lon-lat"),
  };
  set_values(data_list,
    data_list+sizeof(data_list)/sizeof(pair_t));
//  set_entry_text_column(0);
}


CBScale::CBScale(){
  const pair_t data_list[] = {
      pair_t( 50000, " 1:50'000"),
      pair_t(100000, "1:100'000"),
      pair_t(200000, "1:200'000"),
      pair_t(500000, "1:500'000")
  };
  set_values(data_list,
    data_list+sizeof(data_list)/sizeof(pair_t));
}

CBUnit::CBUnit(){
  const pair_t data_list[] = {
      pair_t( 0, "px"),
      pair_t( 1, "cm"),
      pair_t( 2, "in")
  };
  set_values(data_list,
    data_list+sizeof(data_list)/sizeof(pair_t));
}

CBPage::CBPage(){
  const pair_t data_list[] = {
  pair_t(iPoint(0,0), ""),
//    pair_t(iPoint(841,1189), "A0"),
//    pair_t(iPoint(594,841), "A1"),
//    pair_t(iPoint(420,594), "A2"),
  pair_t(iPoint(297,420), "A3"),
  pair_t(iPoint(210,297), "A4"),
  pair_t(iPoint(148,210), "A5")
//    pair_t(iPoint(105,148), "A6"),
//    pair_t(iPoint(74,105), "A7"),
//    pair_t(iPoint(52,74), "A8"),
//    pair_t(iPoint(37,52), "A9"),
//    pair_t(iPoint(26,37), "A10")
  };
  set_values(data_list,
    data_list+sizeof(data_list)/sizeof(pair_t));
}

CBCorner::CBCorner(){
  const pair_t data_list[] = {
    pair_t(0, "top-left"),
    pair_t(1, "top-right"),
    pair_t(2, "bottom-right"),
    pair_t(3, "bottom-left")
  };
  set_values(data_list,
    data_list+sizeof(data_list)/sizeof(pair_t));
}

