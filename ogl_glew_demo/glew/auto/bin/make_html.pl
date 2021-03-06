#!/usr/bin/perl
##
## Copyright (C) 2003-2005, Marcelo E. Magallon <mmagallo[]debian org>
## Copyright (C) 2003-2005, Milan Ikits <milan ikits[]ieee org>
##
## This program is distributed under the terms and conditions of the GNU
## General Public License Version 2 as published by the Free Software
## Foundation or, at your option, any later version.

use strict;
use warnings;

do 'bin/make.pl';

#---------------------------------------------------------------------------------------

my @extlist = ();
my %extensions = ();
my $group = "";
my $cur_group = "";

if (@ARGV)
{
    @extlist = @ARGV;

	foreach my $ext (sort @extlist)
	{
		my ($extname, $exturl, $types, $tokens, $functions, $exacts) = parse_ext($ext);
		$cur_group = $extname;
		$cur_group =~ s/^(?:W?)GL(?:X?)_([A-Z0-9]+?)_.*$/$1/;
		$extname =~ s/^(?:W?)GL(?:X?)_(.*)$/$1/;
		if ($cur_group ne $group)
		{
			if ($group ne "")
			{
				print "<br>\n";
			}
			$group = $cur_group;
		}
		if ($exturl)
		{
			print "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"$exturl\">$extname</a><br>\n";
		}
		else
		{
			print "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;$extname<br>\n";
		}
	}
}
