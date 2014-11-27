#!/usr/bin/perl

$filename = $ARGV[0];

#print "filename $filename\n";

@names = ();

open($fh,"<",$filename) or die "Can't open $filename";
while ($name = <$fh>) {
  $name =~ s/\n$//;
  push(@names,$name);
}
close($fh);

print("// Automatically generated from $filename\n");
print("ElementTypes = {\n");
$nextId = 1;
for $name (@names) {
  $upper = uc($name);
  $lower = lc($name);
  print("  \"$upper\": $nextId,\n");
  print("  \"$lower\": $nextId,\n");
  $nextId++;
}
print("};\n");
print("\n");
$nextId = 1;
for $name (@names) {
  $temp = $name;
  $temp =~ s/#//;
  $upper = uc($temp);
  print("HTML_$upper = $nextId;\n");
  $nextId++;
}
print("HTML_COUNT = $nextId;\n");
