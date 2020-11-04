///\cond HIDDEN (do not show this in Doxyden)

#include <cassert>
#include <iostream>
#include "filename.h"
#include "err/assert_err.h"
#include <unistd.h>

using namespace std;

int main() {
    assert_eq(file_ext_check("file.jpg",   ".jpg"), 1);
    assert_eq(file_ext_check("file.a.jpg", ".jpg"), 1);
    assert_eq(file_ext_check("file.jpg.a", ".jpg"), 0);
    assert_eq(file_ext_check("FILE.JPG",   ".jpg"), 1);
    assert_eq(file_ext_check("FILE.JPG",   ".gif"), 0);

    assert_eq(file_ext_repl("file.jpg", ".gif"), "file.gif");
    assert_eq(file_ext_repl("file.j", ".gif"), "file.gif");
    assert_eq(file_ext_repl("file", ".gif"), "file.gif");
    assert_eq(file_ext_repl("file.jpg.jpg", ".gif"), "file.jpg.gif");
    assert_eq(file_ext_repl(".jpg", ".gif"), ".gif");

    std::vector<std::string> ff =
      file_get_dirs("dir1/dir2/dir3//dir4/../../dir5/./dir6/file");
//    for (int i=0; i<ff.size(); i++) std::cerr << i << " -- " << ff[i] << "\n";

    assert_eq(ff.size(), 6);
    assert_eq(ff[0], "dir1/dir2/dir3//dir4/../../dir5/./dir6");
    assert_eq(ff[1], "dir1/dir2/dir3//dir4/../../dir5");
    assert_eq(ff[2], "dir1/dir2/dir3//dir4");
    assert_eq(ff[3], "dir1/dir2/dir3");
    assert_eq(ff[4], "dir1/dir2");
    assert_eq(ff[5], "dir1");

    ff = file_get_dirs("/dir1/dir2/");
//    for (int i=0; i<ff.size(); i++) std::cerr << i << " -- " << ff[i] << "\n";

    assert_eq(ff.size(), 2);
    assert_eq(ff[0], "/dir1/dir2");
    assert_eq(ff[1], "/dir1");

    ff = file_get_dirs("a.b");
    assert_eq(ff.size(), 0);

    ff = file_get_dirs("/a.b");
    assert_eq(ff.size(), 0);

    ff = file_get_dirs("./a.b");
    assert_eq(ff.size(), 0);

    // file_get_prefix
    assert_eq(file_get_prefix(""), "");
    assert_eq(file_get_prefix("aaa"), "");
    assert_eq(file_get_prefix("/aaa"), "/");
    assert_eq(file_get_prefix("/abc/def/aaa"), "/abc/def/");
    assert_eq(file_get_prefix("/abc//aaa.bbb"), "/abc//");
    assert_eq(file_get_prefix("/abc/./aaa.ccc"), "/abc/./");
    assert_eq(file_get_prefix("abc/.aaa.ccc"), "abc/");

    // file exists
    assert_eq(file_exists("filename.test.cpp"), 1);
    assert_eq(file_exists("missing.txt"), 0);
    assert_eq(file_exists("."), 1);
    assert_eq(file_exists("missing/../filename.test.cpp"), 0);

    // file_rel_path
    assert_eq(file_rel_path("a.png",       "b.map"),        "a.png");
    assert_eq(file_rel_path("d1/d2/a.png", "d1/d2/b.map"),  "a.png");
    assert_eq(file_rel_path("d1/d2/a.png", "d1/b.map"),     "d2/a.png");
    assert_eq(file_rel_path("d1/a.png",    "d2/b.map"),     "../d1/a.png");
    assert_eq(file_rel_path("d1/a.png",    "d1/d2/b.map"),  "../a.png");
    assert_eq(file_rel_path("/d1/a.png",   "/d1/b.map"),   "/d1/a.png");
    assert_eq(file_rel_path("a.png",       "/d1/b.map"),    std::string(getcwd(0,0)) + "/a.png");
    assert_eq(file_rel_path("a.png",       "a/b/c/d/b.map"),  "../../../../a.png");
}

///\endcond
