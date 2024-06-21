/^[^#[:blank:]C]/ {
  num=""$1; name=$0; 
  sub("^"num, "", name); 
  sub("[[:blank:]]+", "", name);
  gsub("\"", "\\\"", name);
  print "PCI_VENDOR(0x"num",\""name"\")"
}
