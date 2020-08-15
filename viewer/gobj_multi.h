#ifndef GOBJ_MULTI_H
#define GOBJ_MULTI_H

#include <map>
#include <vector>
#include <memory>
#include "gobj.h"

// Combine multiple GObj into one.

class GObjMulti : public GObj {
private:

  // add additional information to GObj
  struct GObjData {
    std::shared_ptr<GObj> obj;
    bool on; // on/off
    sigc::connection redraw_conn;
  };

  std::multimap<int, GObjData> data;

  // find an object
  std::multimap<int, GObjData>::iterator
    find(const std::shared_ptr<GObj> & o);

  // find an object (const version)
  std::multimap<int, GObjData>::const_iterator
    find(const std::shared_ptr<GObj> & o) const;

  // Copy of cnv and opt. Used to set up added objects
  std::shared_ptr<ConvBase> cnv;
  Opt opt;

public:

  // Add new object at some depth (larger depth - earlier the object is drawn)
  void add(int depth, std::shared_ptr<GObj> o);

  // Return number of objects
  int size() const { return data.size(); }

  // get list of all objects
  std::vector<std::shared_ptr<GObj> > get_data() const;

  // get object depth (-1 if there is no such object)
  int get_depth(std::shared_ptr<GObj> o) const;

  // get object visibility trow Err if there is no such object)
  bool get_visibility(std::shared_ptr<GObj> o) const;

  // set object depth
  void set_depth(std::shared_ptr<GObj> o, int depth);

  // set object visibility
  void set_visibility(std::shared_ptr<GObj> o, bool on);

  // delete an object
  void del(std::shared_ptr<GObj> o);

  // delete all objects
  void clear();

  // Override GObj methods

  // Draw all objects
  int draw(const CairoWrapper & cr, const dRect & draw_range) override;

  // prepare range
  void prepare_range(const dRect & range) override;

  // Set cnv
  void set_cnv(std::shared_ptr<ConvBase> cnv) override;

  // Set options
  void set_opt(const Opt & opt) override;

};

#endif