## rainbow

Convert double values into RGB color gradients.

--------------
## Changelog:

2019.05.02 V.Zavjalov 2.0:
- New interface: a single class Rainbow with get() and
  set_limits() methods.
- Build color gradients from strings where characters represent
  colors ("RGB", "BCGYRM", etc.)
- Fix error with inverse gradient limits
- Faster get() method

2019.05.01 V.Zavjalov 1.0:
- First version, taken from mapsoft, pico_rec.
