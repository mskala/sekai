#
# LK204-7T-1U LCD panel library
# Copyright (C) 2014  Matthew Skala
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Matthew Skala
# http://ansuz.sooke.bc.ca/
# mskala@ansuz.sooke.bc.ca
#

########################################################################

sub lk204_open {
  if ((`uname -m`=~/[x46]86/) || ($ARGV[1]=~/-t/)) {
    `/usr/bin/stty raw -echo cbreak`;
    open($lk204_out_fh,'>-');
    open($lk204_in_fh,'<-');
    $lk204_is_ansi=1;
    print $lk204_out_fh "\e[H\e[J";
    print $lk204_out_fh "\e[8;8HSimulated LK204-7T-1U";
    print $lk204_out_fh "\e[9;8H########################";
    print $lk204_out_fh "\e[10;8H#\e[10;31H#";
    print $lk204_out_fh "\e[11;8H#\e[11;31H#";
    print $lk204_out_fh "\e[12;8H#\e[12;31H#";
    print $lk204_out_fh "\e[13;8H#\e[13;31H#";
    print $lk204_out_fh "\e[14;8H########################";
    print $lk204_out_fh "\e[10;10H";
  } else {
    `/usr/bin/stty raw -echo cbreak < /dev/ttyUSB0`;
    open($lk204_out_fh,'>/dev/ttyUSB0');
    open($lk204_in_fh,'</dev/ttyUSB0');
    $lk204_is_ansi=0;
    print $lk204_out_fh "\xFE\x4B\xFE\x53";
  }
  flush $lk204_out_fh;
}

sub lk204_close {
  close($lk204_out_fh);
  close($lk204_in_fh);
  if ($lk204_is_ansi) {
    print "\n\n\n\n\n";
    `/usr/bin/stty sane`;
  }
}

########################################################################

sub lk204_text {
  foreach $t (@_) {
    $s=$t;
    $s=~tr/ -}\xFE/ -}/d;
    print $lk204_out_fh $t;
  }
  flush $lk204_out_fh;
}

########################################################################

sub lk204_clear_screen {
  if ($lk204_is_ansi) {
    print $lk204_out_fh "\e[9;8H########################";
    print $lk204_out_fh "\e[10;8H#                      #";
    print $lk204_out_fh "\e[11;8H#                      #";
    print $lk204_out_fh "\e[12;8H#                      #";
    print $lk204_out_fh "\e[13;8H#                      #";
    print $lk204_out_fh "\e[14;8H########################";
    print $lk204_out_fh "\e[10;10H";
  } else {
    print $lk204_out_fh "\xFE\x58";
  }
  flush $lk204_out_fh;
}

sub lk204_set_cursor_pos {
  my($row,$col)=@_;
  
  $row=1 if $row<1;
  $row=4 if $row>4;
  $col=1 if $col<1;
  $col=20 if $col>20;
  
  if ($lk204_is_ansi) {
    printf $lk204_out_fh "\e[%d;%dH",9+$row,9+$col;
  } else {
    printf $lk204_out_fh "\xFE\x47%c%c",$col,$row;
  }
  flush $lk204_out_fh;
}

sub lk204_go_home {
  if ($lk204_is_ansi) {
    print $lk204_out_fh "\e[10;10H";
  } else {
    print $lk204_out_fh "\xFE\x48";
  }
  flush $lk204_out_fh;
}

########################################################################

sub lk204_get_key {
  $tilde='~';
  while (1) {
    if ($tilde ne '~') {
      $_=$tilde;
      $tilde='~';
    } else {
      read($lk204_in_fh,$_,1);
    }
    return $_ if /^[A-EGH]$/;
    return 'E' if ($_ eq "\n") && $lk204_is_ansi;
    return 'E' if ($_ eq "\r") && $lk204_is_ansi;
    next unless $lk204_is_ansi && ($_ eq "\e");
    read($lk204_in_fh,$_,1);
    next unless $_ eq '[';
    read($lk204_in_fh,$_,1);
    return 'B' if $_ eq 'A';
    return 'C' if $_ eq 'C';
    return 'D' if $_ eq 'D';
    return 'E' if $_ eq 'E';
    return 'H' if $_ eq 'B';
    read($lk204_in_fh,$tilde,1);
    next unless $tilde eq '~';
    return 'A' if $_ eq '1';
    return 'G' if $_ eq '4';
  }
}

########################################################################

sub lk204_led_red {
  my($ledno)=@_;

  $ledno=1 if $ledno<1;
  $ledno=3 if $ledno>3;

  if ($lk204_is_ansi) {
    printf $lk204_out_fh "\e[%d;3H\e[1;31mRED\e[0m",7+2*$ledno;
  } else {
    printf $lk204_out_fh "\xFE\x57%c\xFE\x56%c",2*$ledno-1,2*$ledno;
  }
  flush $lk204_out_fh;
}

sub lk204_led_yellow {
  my($ledno)=@_;

  $ledno=1 if $ledno<1;
  $ledno=3 if $ledno>3;

  if ($lk204_is_ansi) {
    printf $lk204_out_fh "\e[%d;3H\e[1;33mYEL\e[0m",7+2*$ledno;
  } else {
    printf $lk204_out_fh "\xFE\x56%c\xFE\x56%c",2*$ledno-1,2*$ledno;
  }
  flush $lk204_out_fh;
}

sub lk204_led_green {
  my($ledno)=@_;

  $ledno=1 if $ledno<1;
  $ledno=3 if $ledno>3;

  if ($lk204_is_ansi) {
    printf $lk204_out_fh "\e[%d;3H\e[1;32mGRN\e[0m",7+2*$ledno;
  } else {
    printf $lk204_out_fh "\xFE\x56%c\xFE\x57%c",2*$ledno-1,2*$ledno;
  }
  flush $lk204_out_fh;
}

sub lk204_led_off {
  my($ledno)=@_;

  $ledno=1 if $ledno<1;
  $ledno=3 if $ledno>3;

  if ($lk204_is_ansi) {
    printf $lk204_out_fh "\e[%d;3H\e[0mOFF",7+2*$ledno;
  } else {
    printf $lk204_out_fh "\xFE\x57%c\xFE\x57%c",2*$ledno-1,2*$ledno;
  }
  flush $lk204_out_fh;
}

########################################################################

sub lk204_horiz_menu {
  my($lineno)=shift;
  my(@col,$xcol);

  for ($i=0,$xcol=1;$i<$#_;$i++) {
    $col[$i]=$xcol;
    $xcol+=(length($_[$i])+1);
  }
  push @col,21-length($_[$#_]);
  
  &lk204_set_cursor_pos($lineno,1);
  for ($i=0,$xcol=1;$i<=$#_;$i++) {
    &lk204_text((' 'x($col[$i]-$xcol)).$_[$i]);
    $xcol=$col[$i]+length($_[$i]);
  }

  $i=0;
  while (1) {
    &lk204_set_cursor_pos($lineno,$col[$i]);
    $_=&lk204_get_key;
    if ($_ eq 'E') {
      return $_[$i];
    }
    if (($_ eq 'B') || ($_ eq 'D')) {
      $i--;
      $i=$#_ if $i<0;
    }
    if (($_ eq 'C') || ($_ eq 'H')) {
      $i++;
      $i=0 if $i>$#_;
    }
  }
}

sub lk204_horiz_submenu {
  my($lineno)=shift;
  my(@col,$xcol);

  for ($i=0,$xcol=1;$i<$#_;$i++) {
    $col[$i]=$xcol;
    $xcol+=(length($_[$i])+1);
  }
  push @col,21-length($_[$#_]);
  
  &lk204_set_cursor_pos($lineno,1);
  for ($i=0,$xcol=1;$i<=$#_;$i++) {
    &lk204_text((' 'x($col[$i]-$xcol)).$_[$i]);
    $xcol=$col[$i]+length($_[$i]);
  }
  
  $i=0;
  while (1) {
    &lk204_set_cursor_pos($lineno,$col[$i]);
    $_=&lk204_get_key;
    if ($_ eq 'E') {
      return $_[$i];
    }
    if ($_ eq 'A') {
      return '[ESC]';
    }
    if (($_ eq 'B') || ($_ eq 'D')) {
      $i--;
      $i=$#_ if $i<0;
    }
    if (($_ eq 'C') || ($_ eq 'H')) {
      $i++;
      $i=0 if $i>$#_;
    }
  }
}

sub lk204_vert_menu {
  my($prompt)=shift;
  my($scroll)=-1;
  
  &lk204_clear_screen;
  &lk204_text($prompt);
  for ($i=0;$i<=$#_;$i++) {
    &lk204_set_cursor_pos($i+2,1);
    &lk204_text($_[$i]);
    last if $i>=2;
  }
  
  $i=0;
  while (1) {
    &lk204_set_cursor_pos($i-$scroll+1,1);
    $_=&lk204_get_key;

    if ($_ eq 'E') {
      return $_[$i];
    }
    if ($_ eq 'A') {
      return '[ESC]';
    }

    if (($_ eq 'B') || ($_ eq 'D')) {
      $i--;
      $i=$#_ if $i<0;
    }
    if (($_ eq 'C') || ($_ eq 'H')) {
      $i++;
      $i=0 if $i>$#_;
    }

    if ($i-$scroll<0) {
      $scroll=$i;
      &lk204_clear_screen;
      for ($j=$scroll;$j<=$#_;$j++) {
        &lk204_set_cursor_pos($j-$scroll+1,1);
        &lk204_text($_[$j]);
        last if $j-$scroll>=3;
      }
    }

    if ($i-$scroll>3) {
      $scroll=$i-3;
      &lk204_clear_screen;
      for ($j=$scroll;$j<=$#_;$j++) {
        &lk204_set_cursor_pos($j-$scroll+1,1);
        &lk204_text($_[$j]);
        last if $j-$scroll>=3;
      }
    }
  }
}

sub lk204_grid_submenu {
  my($startline)=shift;
  my(@col,$numcols,@row);
  my($sel,$special_sel);
  my($i,$j,$k);
  my(@cwidth);

  # find the maximum number of columns we can fit
  for ($numcols=$#_+1;$numcols>0;$numcols--) {
    for ($i=0;$i<$numcols;$i++) { $cwidth[$i]=0; }
    for ($i=0;$i<=$#_;$i++) {
      $cwidth[$i%$numcols]=length($_[$i])
        if length($_[$i])>$cwidth[$i%$numcols];
    }
    $j=0;
    for ($i=0;$i<$numcols;$i++,$j++) { $j+=$cwidth[$i]; }
    $j--;
    last if $j<=20;
  }
  # NOTE big trouble if the loop terminates at $numcols=0

  # compute the layout we have chosen, which just had better fit
  # also display it
  for ($i=0;$i<=$#_;$i++) {
    $row[$i]=$startline+int($i/$numcols);
    if ($i%$numcols==0) {
      $col[$i]=1;
      &lk204_set_cursor_pos($row[$i],1);
      &lk204_text(substr($_[$i].(' 'x20),0,20));
    } else {
      $col[$i]=$col[$i-1]+$cwidth[($i-1)%$numcols]+1;
      &lk204_set_cursor_pos($row[$i],$col[$i]);
      &lk204_text($_[$i]);
    }
  }

  $sel=0;
  $special_sel=int($#_/$numcols)*$numcols;
  while (1) {
    &lk204_set_cursor_pos($row[$sel],$col[$sel]);
    $_=&lk204_get_key;

    if ($_ eq 'E') { # enter
      return $_[$sel];
    }

    if ($_ eq 'A') { # up-left/escape
      return '[ESC]';
    }

    if ($_ eq 'D') { # left
      $sel--;
      if ($sel<0) {
        $sel=$numcols-1;
      } elsif (($sel+1)%$numcols==0) {
        $sel+=$numcols;
      }
      $sel=$#_ if $sel>$#_;
    }

    if ($_ eq 'C') { # right
      $sel++;
      if ($sel>$#_) {
        $sel=$special_sel;
      } elsif ($sel%$numcols==0) {
        $sel-=$numcols;
      }
    }

    if ($_ eq 'B') { # up
      if ($sel>=$numcols) {
        $sel-=$numcols;
      } else {
        $sel+=$special_sel;
        $sel-=$numcols if $sel>$#_;
      }
    }

    if ($_ eq 'H') { # down
      if ($sel>=$special_sel) {
        $sel-=$special_sel;
      } else {
        $sel+=$numcols;
        $sel-=$special_sel if $sel>$#_;
      }
    }
  }
}

########################################################################

sub lk204_edit_ip_address {
  my($lineno,$ip)=@_;
  my(@digits);

  my($a,$b,$c,$d)=split(/\./,$ip);

  $a=0 if $a<0;
  $a=255 if $a>255;
  $b=0 if $b<0;
  $b=255 if $b>255;
  $c=0 if $c<0;
  $c=255 if $c>255;
  $d=0 if $d<0;
  $d=255 if $d>255;

  $digits[1]=int($a/100);
  $digits[2]=int($a/10)%10;
  $digits[3]=$a%10;
  $digits[5]=int($b/100);
  $digits[6]=int($b/10)%10;
  $digits[7]=$b%10;
  $digits[9]=int($c/100);
  $digits[10]=int($c/10)%10;
  $digits[11]=$c%10;
  $digits[13]=int($d/100);
  $digits[14]=int($d/10)%10;
  $digits[15]=$d%10;

  &lk204_set_cursor_pos($lineno,1);
  &lk204_text(sprintf('%3d.%3d.%3d.%3d   OK',$a+0,$b+0,$c+0,$d+0));
  
  my($xcol)=1;
  while (1) {
    &lk204_set_cursor_pos($lineno,$xcol);
    $_=&lk204_get_key;

    if (($_ eq 'E') && ($xcol==19)) { # enter
      return sprintf('%d.%d.%d.%d',
        $digits[1]*100+$digits[2]*10+$digits[3],
        $digits[5]*100+$digits[6]*10+$digits[7],
        $digits[9]*100+$digits[10]*10+$digits[11],
        $digits[13]*100+$digits[14]*10+$digits[15]);
    }
    
    if (($_ eq 'B') && ($xcol!=19)) { # up
      $digits[$xcol]++;
      $digits[$xcol]=0 if $digits[$xcol]>9;
      $digits[$xcol]=0
        if (100*$digits[($xcol&~3)+1]+10*$digits[($xcol&~3)+2]
          +$digits[($xcol&~3)+3])>255;
      if (($xcol&3)==3) {
        &lk204_text($digits[$xcol]);
      } elsif ((($xcol&3)==2) && ($digits[$xcol]+$digits[$xcol-1]>0)) {
        &lk204_text($digits[$xcol]);
      } elsif ((($xcol&3)==1) && ($digits[$xcol]>0)) {
        &lk204_text($digits[$xcol]);
        if ($digits[$xcol]+$digits[$xcol+1]>0) {
          &lk204_text($digits[$xcol+1]);
        } else {
          &lk204_text(' ');
        }
      } elsif (($xcol&3)==1) {
        &lk204_text(' ');
        if ($digits[$xcol]+$digits[$xcol+1]>0) {
          &lk204_text($digits[$xcol+1]);
        } else {
          &lk204_text(' ');
        }
      } else {
        &lk204_text(' ');
      }
    }

    if (($_ eq 'H') && ($xcol!=19)) { # down
      $digits[$xcol]--;
      $digits[$xcol]=9 if $digits[$xcol]<0;
      $digits[$xcol]--
        while (100*$digits[($xcol&~3)+1]+10*$digits[($xcol&~3)+2]
          +$digits[($xcol&~3)+3])>255;
      if (($xcol&3)==3) {
        &lk204_text($digits[$xcol]);
      } elsif ((($xcol&3)==2) && ($digits[$xcol]+$digits[$xcol-1]>0)) {
        &lk204_text($digits[$xcol]);
      } elsif ((($xcol&3)==1) && ($digits[$xcol]>0)) {
        &lk204_text($digits[$xcol]);
        if ($digits[$xcol]+$digits[$xcol+1]>0) {
          &lk204_text($digits[$xcol+1]);
        } else {
          &lk204_text(' ');
        }
      } elsif (($xcol&3)==1) {
        &lk204_text(' ');
        if ($digits[$xcol]+$digits[$xcol+1]>0) {
          &lk204_text($digits[$xcol+1]);
        } else {
          &lk204_text(' ');
        }
      } else {
        &lk204_text(' ');
      }
    }

    if ($_ eq 'D') { # left
      $xcol--;
      $xcol=19 if $xcol<1;
      $xcol=14 if $xcol==18;
      $xcol-- if ($xcol&3)==0;
    }

    if ($_ eq 'C') { # right
      $xcol++;
      $xcol=1 if $xcol>19;
      $xcol=19 if $xcol>15;
      $xcol++ if ($xcol&3)==0;
    }
  }
}

########################################################################

1;
