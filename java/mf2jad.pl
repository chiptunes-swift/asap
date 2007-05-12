#!/usr/bin/perl
use strict;
my ($name) = @ARGV or die "Usage: mf2jad APPNAME\n";
open MF, "MIDLET_MANIFEST.MF" or die "MIDLET_MANIFEST.MF: $!\n";
open JAD, ">$name.jad" or die "$name.jad: $!\n";
print JAD "MIDlet-Jar-URL: $name.jar\n";
print JAD "MIDlet-Jar-Size: ", -s "$name.jar", "\n";
print JAD $_ while <MF>;
