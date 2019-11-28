# Run command and check its output and status code.
function assert_cmd(){
  cmd="$1"
  exp="$2"
  set +o errexit
  res="$($cmd 2>&1)";
  ret="$?"
  set -o errexit

  rete="${3:-''}"
  if [ "$exp" != "$res" ]; then
    printf "ERROR ($cmd):\n"
    printf "  exp:\n%s\n" "$exp"
    printf "  res:\n%s\n" "$res"
    exit 1
  fi
  if [ "$rete" != "" -a "$ret" != "$rete" ]; then
    printf "ERROR in return code ($cmd):\n"
    printf "  exp:\n%s\n" "$rete"
    printf "  ret:\n%s\n" "$ret"
    exit 1
  fi
}

# Check difference between two files.
# On sucsess the second one will be deleted.
function assert_diff(){
  f_exp="$1"
  f_res="$2"
    if ! diff -q -- "$f_exp" "$f_res"; then
    echo "different files: $f_exp $f_res:"
    diff -- "$f_exp" "$f_res"
    exit 1
  else
    rm -f "$f_res"
  fi
}

# Remove time and cairo version from pdf file.
function fix_pdf(){
  sed -i 's/CreationDate (D:[^)]\+)/CreationDate ()/
          s/Producer (.\+)/Producer ()/' $@
}

