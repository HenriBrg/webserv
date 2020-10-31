#!/usr/bin/perl

use strict;
use warnings;

print "Status: 200\n";
print "Content-type: text/html\n";
print "\r\n\r\n";

print "<html><head><title> PERL CGI</title> </head><body>";
 
foreach my $key (keys %ENV) {
    print "<br>$key --> $ENV{$key}</br>";
}

print "</body> </html> ";
