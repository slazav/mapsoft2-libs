## rainbow -- convert double values into RGB color gradients

* `Rainbow(const std::vector<rainbow_data> & RD);` -- the most universal
interface to convert a double value into RBG color gradient using a
mapping table `RD`. The table is `std::vector<rainbow_data>` where
`rainbow_data` has two fiels, `v` for value and `c` for the color. The
table must be sorted by `v`, either increasing or decreasing.

* `Rainbow(double min, double max, const char *colors);` -- this
constructor makes a color gradient using a string: characters in the
string: `R`, `G`, `B` for red, green, and blue; `C`, `M`, `Y` for cyan,
magenta, and yellow; `W` and `K` for white and black, `r`, `g`, `b`, `c`,
`m`, `y`, `w` for darker colors. Other characters are ignored. Values
`min` and `max` are data values which correspond to the first and last
color. If `max` > `min` the gradient will be reversed.

* `Rainbow(double min, double max, rainbow_type type=RAINBOW_NORMAL);` -- constructor
with some predefined gradients:

- `RAINBOW_NORMAL`   -- B-C-G-Y-R-M gradient
- `RAINBOW_BURNING`  -- W-Y-R-M-B-b gradient
- `RAINBOW_BURNING1` -- K-R-Y-W gradient

* `Rainbow(double min, double max, int cmin, int cmax);` -- constructor
for making a simple two-color gradient.

* `Rainbow()` -- empty constructor, converts everything to zero.

-------------

Method `get(v)` converts value to a color. Example:
```c++
std::vector<rainbow_data> RD = {
  {0.1, 0x000000},
  {0.5, 0xFF0000}, // 0.1 - 0.5 black -> blue
  {0.5, 0xFF00FF}, // - color step
  {0.9, 0x000000}, // 0.5 - 0.9 magenta -> black
};
Rainbow R(RD);

int color = R.get(v); // get color for v!
```

Method `set_limits(c1,c2)` sets colors for data values which are
below minimum and above maximum value in the conversion table;
Use -1 to set default color which correspond to min/max in the table.

In addition there is `color_shade` function for shading colors:
`int c=color_shade(c, 0.2);`


--------------
## Changelog:

2020.08.16 V.Zavjalov 3.0:
- switch to ARGB colors

2020.08.16 V.Zavjalov 2.1:
- Add empty constructor, rewrite Readme

2019.05.02 V.Zavjalov 2.0:
- New interface: a single class Rainbow with get() and
  set_limits() methods.
- Build color gradients from strings where characters represent
  colors ("RGB", "BCGYRM", etc.)
- Fix error with inverse gradient limits
- Faster get() method

2019.05.01 V.Zavjalov 1.0:
- First version, taken from mapsoft, pico_rec.
