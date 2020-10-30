#!/usr/bin/perl

use strict;
use warnings;

print "Status: 200\n";
print "Content-type: text/html\n";
foreach my $key (keys %ENV) {
    print "$key --> $ENV{$key}<br>";
}