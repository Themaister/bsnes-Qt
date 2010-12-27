synchronize() {
  if [ -d ../"$1" ]; then
    test -d "$1" && rm -r "$1"
    cp -r ../"$1" ./"$1"
  fi
}

synchronize "nall"
synchronize "ruby"

test -d ruby/_test && rm -r ruby/_test
