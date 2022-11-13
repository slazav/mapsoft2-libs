#ifndef DB_SIMPLE_H
#define DB_SIMPLE_H

#include <memory>
#include <string>
#include <stdint.h>
#include "geom/rect.h"

// Simple Berkleydb-based database with integer key and
// std::string value.

class DBSimple{
  private:
    std::shared_ptr<void> db;   // database
    std::shared_ptr<void> cur;  // cursor

  public:

   // open database:
   // fname - file name
   // dbname - database name (can be NULL),
   // create - create flag.
   // dup    - alow duplicates flag (default false).
   // Note: if you use non-null dbname and put a few databases in a single file,
   // you will need to create environment to open both databases.
   DBSimple(std::string fname, const char *dbname, bool create, bool dup = false);
   ~DBSimple();

   // Put data with a given key (overwrite old value if it exists).
   // If key duplication is not allowed and key already exists,
   // then the record will be overwritten.
   void put(const uint32_t key, const std::string & val);

   // Put data after the last record, return the new key
   uint32_t put(const std::string & val) {
     uint32_t key;
     get_last(key);
     key = 0xFFFFFFFF ? 0 : key+1;
     put(key, val);
     return key;
   }

   // Check if the key exists in the database.
   bool exists(const uint32_t key);

   // Low-level get function with BerkleyDB flags
   std::string get(uint32_t & key, int flags);

   // Get data for a given key.
   // If record is not found then key is set to 0xFFFFFFFF and
   // empty string is returned.
   // Equivalent to DB_SET flag in BerkleyDB
   std::string get(uint32_t & key);

   // Get data with key larger or equals then the given key.
   // Value of key is set according to the returned record.
   // If record is not found then key is set to 0xFFFFFFFF and
   // empty string is returned.
   // Equivalent to DB_SET_RANGE flag in BerkleyDB
   std::string get_range(uint32_t & key);

   // Get next entry.
   // If record is not found then key is set to 0xFFFFFFFF and
   // empty string is returned.
   // On input key is ignored, cursor position is used.
   // Equivalent to DB_NEXT flag in BerkleyDB
   std::string get_next(uint32_t & key);

   // Get previous entry.
   // If record is not found then key is set to 0xFFFFFFFF and
   // empty string is returned.
   // On input key is ignored, cursor position is used.
   // Equivalent to DB_PREV flag in BerkleyDB
   std::string get_prev(uint32_t & key);

   // Get last entry.
   // If record is not found key is set to 0xFFFFFFFF and
   // empty string is returned. On input key is ignored.
   // Equivalent to DB_LAST flag in BerkleyDB
   std::string get_last(uint32_t & key);

   // Get first entry.
   // If record is not found key is set to 0xFFFFFFFF and
   // empty string is returned. On input key is ignored.
   // Equivalent to DB_FIRST flag in BerkleyDB
   std::string get_first(uint32_t & key);

   // Delete entry with a given key.
   // If key duplication is allowed then multiple records can be deleted.
   // Returns number of deleted entries (maybe zero)
   uint32_t del(const uint32_t key);

   // Return number of records
   size_t size();

   // Iterator interface
   // (A very basic one, BerkleyDB cursors have many more features to implement here)
   struct iterator {
     iterator(void *dbp = NULL);
     iterator(const iterator & i); // copy iterator -- using cursor->dup
     // Low-level BerkleyDB cursor operations.
     void c_del(int flags);
     void c_get(int flags);
     void c_get(uint32_t key, const std::string & val, int flags);
     void c_put(uint32_t key, const std::string & val, int flags);

     std::pair<uint32_t, std::string> & operator*() {
       if (end) throw Err() << "db_simple: invalid iterator";
       return curr_val;}
     std::pair<uint32_t, std::string> * operator->() {
       if (end) throw Err() << "db_simple: invalid iterator";
       return &curr_val;
     }
     iterator& operator++(); // prefix only!
     // only end flag comparison!
     friend bool operator== (const iterator& a, const iterator& b) {return a.end == b.end;}
     friend bool operator!= (const iterator& a, const iterator& b) {return a.end != b.end;}

   private:
     std::shared_ptr<void> cur;            // cursor
     bool end;                             // end flag
     std::pair<uint32_t, std::string> curr_val; // current value
   };
   iterator begin();
   iterator end();
   iterator find(uint32_t key);
   iterator erase(iterator & i); // returns next iterator
};


#endif
