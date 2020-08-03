#!/usr/bin/perl
  
use warnings;

my $string = "";

  # Parsing file 
 open (FILE, 'data.txt');
 while (<FILE>) {
   ($name) = split("\n");
   
   #Removing the 50 first occurence
   my $fragment =  substr $name, 50;
   $string = $string . "$fragment";
 }
 close (FILE);

   # Removing consecutive point
   $string =~ tr///cs; 
   
   # Replace '.' by '\n'
   $string =~ tr/./\n/;
   
   @personal = split(/\n/, $string);
   
   # Removing line with a length == 1
   for my $el (@personal) {
      if(length($el) != 1){
        print "$el\n";
      }
    }
 exit;