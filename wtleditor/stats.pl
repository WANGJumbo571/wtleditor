#!/usr/bin/perl
   use strict;
   use warnings;
   use Encode;
   
   my $path1 = decode("utf8", "K:/娱乐/vs2022/wtleditor/wtleditor");
   my $path= encode("gb2312", $path1);
   my $filecount = 0; 

   my $max = -1;
   
   sub parse_env {    
      my $path = $_[0]; #或者使用 my($path) = @_; @_类似javascript中的arguments
       my $subpath;
       my $handle; 

       if (-d $path)
       {#当前路径是否为一个目录
           if (opendir($handle, $path)) {
               while ($subpath = readdir($handle)) {
                   if ($subpath =~ m/.cpp$/) {
                       my $p = $path."/$subpath"; 
                           ++$filecount;
                           print $p."\n";
                       open(my $file, '<', $p) || die "cant op file\n";
                       while (my $line = <$file>) {
                          if ($line =~ m/reportError\((\d+)/) {
                              print "Result: ".$1."\n";
                              if ($max < $1) {
                                 $max = $1;
                              }
                          }
                       }
                   }
               }
               closedir($handle);            
           }
       } 
       return $filecount;
   } 
    
   my $count = parse_env $path;
   my $str = "max number: ".$max;
   print $str; 