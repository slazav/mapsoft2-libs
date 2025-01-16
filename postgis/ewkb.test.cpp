///\cond HIDDEN (do not show this in Doxyden)

#include "ewkb.h"
#include "err/assert_err.h"

// Point eith with SRID
std::string v0 =
"0101000020E96400000000000078A22341FFFFFF3F0D8A5D41";

// LineString with SRID
std::string v1 =
"0102000020E964000006000000400AD723D8292641686666664F495D41F0285C0FD329264113AE47A14E"
"495D4100000080D2292641B047E18A4E495D4150B81E05D229264185EB51784E495D41EF51B89E43292641B91E85CB38495D41A0703D"
"0A3E2926412085EBF137495D41";

// Polygon with SRID
std::string v2 =
"0103000020E96400000100000011000000000000009C9F2041FFFFFF3F68205D4101000000749F2041FE"
"FFFF7F68205D4100000000509F2041000000C067205D4150B81E05449F2041000000C066205D4100000000429F20410DD7A3C065205D"
"4101000000729F2041FFFFFFBF61205D41000000008C9F2041000000C05A205D4100000000AA9F2041000000805A205D4150B81E05AE"
"9F2041010000805A205D41B147E1FADA9F20413C0AD7335B205D4100000000DE9F2041000000405B205D4101000000E69F2041F3285C"
"3F5C205D4100000000E69F204148E17A245D205D4100000000E69F20410000004060205D4100000000F09F2041F2285CFF65205D4100"
"000000E89F20410000008067205D41000000009C9F2041FFFFFF3F68205D41";

// Compound with SRID: two lines
std::string v3 =
"0109000020E964000002000000010200000004000000ECD294ABC92326417EB3C16263485D41D0A370BD"
"2F23264151B81EF54B485D4190C2F5A823232641ED51B81E4A485D41B047E1FABE222641B147E1CA3A485D41010200000006000000B0"
"47E1FABE222641B147E1CA3A485D41303333B3BC21264191C2F56813485D41305C8FC2BA212641D8A3701D13485D4141E17A945C2126"
"4113AE47C104485D41A29999995B212641B047E19A04485D41C0F5285C532126419899995903485D41";

int
main(){
  try{

    auto ml = ewkb_decode(v0, false, false);
    // Point: order(2) + type(8) + srid(8) + 16*2
    assert_eq(ml.npts(),  1);
    assert_eq(v0.size(),  2 + 8*2 + 16*2);

    ml = ewkb_decode(v1, false, false);
    // LineString: order(2) + type(8) + srid(8) + npts(8) + npts*16*2
    assert_eq(v1.size(),  2 + 8*3 + ml.npts()*16*2);

    // Polygon: order(2) + type(8) + srid(8) + nseg(8) + nseg*(npts(8) + npts*16*2)
    ml = ewkb_decode(v2, false, false);
    assert_eq(v2.size(),  2 + 8*3 + ml.size()*(8 + ml.npts()*16*2));

    // Compound: order(2) + type(8) + srid(8) + nseg(8) + nparts( order(2) + type(8) + npts(8) + npts*16*2 )
    ml = ewkb_decode(v3, false, false);
    assert_eq(ml.size(), 2);
    assert_eq(v3.size(),  2 + 8*3 + (2 + 8*2)*ml.size() + 16*2*ml.npts());

  }
  catch (Err & e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}

///\endcond
