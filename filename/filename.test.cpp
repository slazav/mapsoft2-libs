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

    assert_eq(file_ext_check("FILE.JPG",   {".jpg", ".jpeg"}), 1);
    assert_eq(file_ext_check("FILE.JPEG",  {".jpg", ".jpeg"}), 1);
    assert_eq(file_ext_check("FILE.JPG1",  {".jpg", ".jpeg"}), 0);

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

    ff = file_get_dirs("/dir1/dir2/", 0);
    // for (int i=0; i<ff.size(); i++) std::cerr << i << " -- " << ff[i] << "\n";
    assert_eq(ff.size(), 2);
    assert_eq(ff[0], "/dir1/dir2");
    assert_eq(ff[1], "/dir1");

    ff = file_get_dirs("/dir1/dir2/", 1);
    // for (int i=0; i<ff.size(); i++) std::cerr << i << " -- " << ff[i] << "\n";
    assert_eq(ff.size(), 2);
    assert_eq(ff[0], "/dir1");
    assert_eq(ff[1], "/dir1/dir2");

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

    // file_get_name
    assert_eq(file_get_name(""), "");
    assert_eq(file_get_name("aaa"), "aaa");
    assert_eq(file_get_name("/aaa"), "aaa");
    assert_eq(file_get_name("/abc/def/aaa"), "aaa");
    assert_eq(file_get_name("/abc//aaa.bbb"), "aaa.bbb");
    assert_eq(file_get_name("/abc/./aaa.ccc"), "aaa.ccc");
    assert_eq(file_get_name("abc/.aaa.ccc"), ".aaa.ccc");

    // file_get_basename
    assert_eq(file_get_basename("", ""), "");
    assert_eq(file_get_basename("aaa", ".aaa"), "aaa");
    assert_eq(file_get_basename("/aaa", ".aaa"), "aaa");
    assert_eq(file_get_basename("/abc/def/aaa", "/aaa"), "aaa");
    assert_eq(file_get_basename("/abc//aaa.bbb", ".bbb"), "aaa");
    assert_eq(file_get_basename("/abc/./aaa.ccc", ".bbb"), "aaa.ccc");
    assert_eq(file_get_basename("abc/.aaa.ccc", ".aaa"), ".aaa.ccc");
    assert_eq(file_get_basename("abc/.aaa.ccc", ".ccc"), ".aaa");
    assert_eq(file_get_basename("abc/.aaa.ccc", ".aaa.ccc"), "");

    // file_get_basename (with all extensions)
    assert_eq(file_get_basename(""), "");
    assert_eq(file_get_basename("aaa"), "aaa");
    assert_eq(file_get_basename("/aaa"), "aaa");
    assert_eq(file_get_basename("/abc//aa.b.c"), "aa");
    assert_eq(file_get_basename("abc/.aaa.ccc"), ".aaa");
    assert_eq(file_get_basename("abc/.a.b.c"), ".a");
    assert_eq(file_get_basename("a.b.c/.a.b.c"), ".a");

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
    char * cwd = getcwd(0,0);
    assert_eq(file_rel_path("a.png",       "/d1/b.map"),    std::string(cwd) + "/a.png");
    free(cwd);
    assert_eq(file_rel_path("a.png",       "a/b/c/d/b.map"),  "../../../../a.png");

    file_mkdir("."); // no need to do anything
    assert_err(file_mkdir("filename.test.cpp"), "not a directory: filename.test.cpp");
    assert_eq(file_exists("test_dir"), 0);
    file_mkdir("test_dir"); // no need to do anything
    assert_eq(file_exists("test_dir"), 1);
    remove("test_dir");
}

///\endcond
