COREFILE=$(find . -maxdepth 1 -name "core*" | head -n 1)
if [[ -f "$COREFILE" ]]; then 
  gdb -c "$COREFILE" bin/arangod -ex "thread apply all bt" -ex "set pagination 0" -batch || echo "no gdb found" 
fi 
