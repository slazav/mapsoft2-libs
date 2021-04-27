# Run command and check its output (full match) and status code.
assert_cmd(){
  cmd="$1"
  exp="$2"
  set +o errexit
  res="$(eval $cmd 2>&1)";
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

# Run command and check its output (substring) and status code.
assert_cmd_substr(){
  cmd="$1"
  exp="$2"
  set +o errexit
  res="$(eval $cmd 2>&1)";
  ret="$?"
  rete="${3:-''}"
  printf "%s" "$res" | grep -q "$exp"
  retg="$?"
  set -o errexit

  if [ "$retg" != 0 ]; then
    printf "ERROR ($cmd):\n"
    printf "  exp substring:\n%s\n" "$exp"
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
assert_diff(){
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
fix_pdf(){
  sed -i 's/CreationDate (D:[^)]\+)/CreationDate ()/
          s/Producer (.\+)/Producer ()/' $@
}

