#!/usr/bin/perl -w

# derived from code found at http://www.troubleshooters.com/codecorn/littperl/perlhashexamps.htm

use strict;

sub usage()
{
	print "\n\nProgram modules. Syntax:\n";
	print "./modules conffile modulename  # prints out params for modulename\n";
	print "./modules conffile            # prints out params for all modules\n";
	print "Anything else prints out this message.\n\n";
}

sub newModule(\%$)
{
	%{$_[0]} = ("modulename"=>$_[1]);
}

sub parseSmbDotConf($)
{
	open(CONFIGFILE, "<" . $_[0]) || die "Cannot read file $_[0]\n";
	my(@configMemoryImage) = <CONFIGFILE>;
	close(CONFIGFILE);

	my($state) = "init";
	my(%allModules);

	my(%module);
	my($configLine);
	foreach $configLine (@configMemoryImage) {
		if ($configLine =~ /^\s*$/) {next;}; #Blow off blank lines
		chomp($configLine);
		$configLine =~ s/^\s+//;            #Delete leading blanks
		$configLine =~ s/\s+$//;            #Delete trailing blanks
		$configLine =~ s/;.*//;             #Delete comments
		$configLine =~ s/#.*//;             #Delete comments
		if ($state eq "init") {
			newModule(%module, "GLOBAL");       #Leading params w/o module are [GLOBAL]
			$state = "init2";
		}
		if ($configLine =~ /^\s*\[\s*(.+)\s*\]\s*$/) {        #if bracket line
			if ($state ne "init2") {
				### RECORD PRIOR SHARE IN %allModules ###
				$allModules{$module{"modulename"}} = {%module};
			}
			newModule(%module, uc($1));
			$state = "brackets";
		} elsif ($configLine =~ /^\s*(.+?)\s*=\s*(.+)\s*$/) {   #if param line
			my($key) = uc($1);
			my($value) = $2;
			$key =~ s/\s//g; 
			$module{$key} = $value;
			$state = "param";
		}
	}
	### ADD FINAL SHARE TO %allModules ###
	$allModules{$module{"modulename"}} = {%module};
	return(%allModules);
}

sub displayOneModule(\%$)
{
	my(%allModules) = %{(shift)};
	my($moduleName) = shift;
	my(%module) = %{$allModules{$moduleName}}; 
	my($key,$value);
	print "\n\n********** [$moduleName] **********\n";
	foreach $key (sort(keys(%module))) {
		print "$key=" . $module{$key} . "\n";
	}
}

sub displayAll(\%)
{
	my(%allModules) = %{(shift)};
	my($key,$value);
	foreach $key (sort(keys(%allModules))) {
		displayOneModule(%allModules, $key)
	}
}

sub main()
{
	my(%modules);
	if($#ARGV < 0) {
		usage();
	} else {
		%modules = parseSmbDotConf($ARGV[0]);
		if ($#ARGV == 0) {
			displayAll(%modules);
		} elsif ($#ARGV == 1) {
			displayOneModule(%modules, uc($ARGV[1]));
		} else {
			usage();
		}
	}
}

main();
