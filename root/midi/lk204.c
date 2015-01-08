/*
 * LK204-7T-1U LCD panel library
 * Copyright (C) 2015  Matthew Skala
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Matthew Skala
 * http://ansuz.sooke.bc.ca/
 * mskala@ansuz.sooke.bc.ca
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <termios.h>
#include <unistd.h>


#include "lk204.h"

/**********************************************************************/

static int test_mode;
static FILE *lk204_in_fp=NULL,*lk204_out_fp=NULL;

void lk204_open(int testmode) {
   struct utsname name_data;
   struct termios termios_data;

   test_mode=testmode;
   if (uname(&name_data)>=0) {
      if (strstr(name_data.machine,"x86") ||
	  strstr(name_data.machine,"486") ||
	  strstr(name_data.machine,"686"))
	test_mode=1;
   }

   if (test_mode) {
      lk204_in_fp=stdin;
      lk204_out_fp=stdout;
   } else {
      lk204_in_fp=fopen("/dev/ttyUSB0","r");
      lk204_out_fp=fopen("/dev/ttyUSB0","w");
   }

   setvbuf(lk204_in_fp,NULL,_IONBF,0);
   tcgetattr(fileno(lk204_in_fp),&termios_data);
   termios_data.c_lflag&=~(ECHO|ICANON);
   termios_data.c_cc[VMIN]=1;
   termios_data.c_cc[VTIME]=0;
   tcsetattr(fileno(lk204_in_fp),TCSAFLUSH,&termios_data);

   if (test_mode)
     fputs("\e[H\e[J"
	   "\e[8;8HSimulated LK204-7T-1U"
	   "\e[9;8H########################"
	   "\e[10;8H#\e[10;31H#"
	   "\e[11;8H#\e[11;31H#"
	   "\e[12;8H#\e[12;31H#"
	   "\e[13;8H#\e[13;31H#"
	   "\e[14;8H########################"
	   "\e[10;10H",
	   lk204_out_fp);
   else
     fputs("\xFE\x4B\xFE\x53",lk204_out_fp);
}

void lk204_close(void) {
   struct termios termios_data;

   if (test_mode) {
      tcgetattr(fileno(lk204_in_fp),&termios_data);
      termios_data.c_lflag|=(ECHO|ICANON);
      tcsetattr(fileno(lk204_in_fp),TCSAFLUSH,&termios_data);
      fputs("\n\n\n\n\n",lk204_out_fp);
   }
   
   fclose(lk204_in_fp);
   fclose(lk204_out_fp);
}

/**********************************************************************/

void lk204_printf(const char *format,...) {
   char buf[4096];
   va_list ap;
   int i,j;
   
   va_start(ap,format);
   vsnprintf(buf,4096,format,ap);
   va_end(ap);
   
   for (i=0,j=0;buf[i];i++) {
      if (buf[i]==0xFE)
	j++;
      else
	buf[i-j]=buf[i];
   }
   buf[i-j]=0;
   
   fputs(buf,lk204_out_fp);
   fflush(lk204_out_fp);
}

/**********************************************************************/

void lk204_clear_screen(void) {
   if (test_mode)
     fputs("\e[9;8H########################"
	   "\e[10;8H#                      #"
	   "\e[11;8H#                      #"
	   "\e[12;8H#                      #"
	   "\e[13;8H#                      #"
	   "\e[14;8H########################"
	   "\e[10;10H",
	   lk204_out_fp);
   else
     fputs("\xFE\x58",lk204_out_fp);
   fflush(lk204_out_fp);
}

void lk204_set_cursor_pos(int r,int c) {
   if (r<1) r=1;
   if (r>4) r=4;
   if (c<1) c=1;
   if (c>20) c=20;

   if (test_mode)
     fprintf(lk204_out_fp,"\e[%d;%dH",9+r,9+c);
   else
     fprintf(lk204_out_fp,"\xFE\x47%c%c",c,r);
   fflush(lk204_out_fp);
}

void lk204_go_home(void) {
   if (test_mode)
     fputs("\e[10;10H",lk204_out_fp);
   else
     fputs("\xFE\x48",lk204_out_fp);
   fflush(lk204_out_fp);
}

/**********************************************************************/

char lk204_get_key(void) {
   char tilde='~',du;
   
   while (1) {
      if (tilde!='~') {
	 du=tilde;
	 tilde='~';
      } else
	fread(&du,1,1,lk204_in_fp);
     
      if ((du>='A') && (du<='E')) return du;
      if ((du=='G') || (du=='H')) return du;
      if (test_mode && ((du=='\n') || (du=='\r'))) return 'E';
      if ((!test_mode) || (du!='\e')) continue;
      
      fread(&du,1,1,lk204_in_fp);
      if (du!='[') continue;
      
      fread(&du,1,1,lk204_in_fp);

      if (du=='A') return 'B';
      if (du=='C') return 'C';
      if (du=='D') return 'D';
      if (du=='E') return 'E';
      if (du=='B') return 'H';
      
      fread(&tilde,1,1,lk204_in_fp);
      if (tilde!='~') continue;

      if (du=='1') return 'A';
      if (du=='4') return 'G';
   }
}

/**********************************************************************/

void lk204_led_red(int ledno) {
   if (ledno<1) ledno=1;
   if (ledno>3) ledno=3;

   if (test_mode)
     fprintf(lk204_out_fp,"\e[%d;3H\e[1;31mRED\e[0m",7+2*ledno);
   else
     fprintf(lk204_out_fp,"\xFE\x57%c\xFE\x56%c",2*ledno-1,2*ledno);
   fflush(lk204_out_fp);
}

void lk204_led_yellow(int ledno) {
   if (ledno<1) ledno=1;
   if (ledno>3) ledno=3;

   if (test_mode)
     fprintf(lk204_out_fp,"\e[%d;3H\e[1;33mYEL\e[0m",7+2*ledno);
   else
     fprintf(lk204_out_fp,"\xFE\x56%c\xFE\x56%c",2*ledno-1,2*ledno);
   fflush(lk204_out_fp);
}

void lk204_led_green(int ledno) {
   if (ledno<1) ledno=1;
   if (ledno>3) ledno=3;

   if (test_mode)
     fprintf(lk204_out_fp,"\e[%d;3H\e[1;32mGRN\e[0m",7+2*ledno);
   else
     fprintf(lk204_out_fp,"\xFE\x56%c\xFE\x57%c",2*ledno-1,2*ledno);
   fflush(lk204_out_fp);
}

void lk204_led_off(int ledno) {
   if (ledno<1) ledno=1;
   if (ledno>3) ledno=3;

   if (test_mode)
     fprintf(lk204_out_fp,"\e[%d;3H\e[0mOFF",7+2*ledno);
   else
     fprintf(lk204_out_fp,"\xFE\x57%c\xFE\x57%c",2*ledno-1,2*ledno);
   fflush(lk204_out_fp);
}

/**********************************************************************/

int lk204_horiz_menu(int lineno,int num_entries,const char **entries) {
   int col[80];
   int i,xcol;
   char du;
   
   if (num_entries>80) num_entries=80;
   
   for (i=0,xcol=1;i<num_entries-1;i++) {
      col[i]=xcol;
      xcol+=(strlen(entries[i])+1);
   }
   col[i]=21-strlen(entries[num_entries-1]);
   
   lk204_set_cursor_pos(lineno,1);
   
   for (i=0,xcol=1;i<num_entries;i++) {
      lk204_printf("%*s",col[i]-xcol+strlen(entries[i]),entries[i]);
      xcol=col[i]+strlen(entries[i]);
   }
   
   i=0;
   while (1) {
      lk204_set_cursor_pos(lineno,col[i]);
      du=lk204_get_key();
      if (du=='E') return i;
      if ((du=='B') || (du=='D')) {
	 i--;
	 if (i<0) i=num_entries-1;
      }
      if ((du=='C') || (du=='H')) {
	 i++;
	 if (i>=num_entries) i=0;
      }
   }
}

int lk204_horiz_submenu(int lineno,int num_entries,const char **entries) {
   int col[80];
   int i,xcol;
   char du;
   
   if (num_entries>80) num_entries=80;
   
   for (i=0,xcol=1;i<num_entries-1;i++) {
      col[i]=xcol;
      xcol+=(strlen(entries[i])+1);
   }
   col[i]=21-strlen(entries[num_entries-1]);
   
   lk204_set_cursor_pos(lineno,1);
   
   for (i=0,xcol=1;i<num_entries;i++) {
      lk204_printf("%*s",col[i]-xcol+strlen(entries[i]),entries[i]);
      xcol=col[i]+strlen(entries[i]);
   }
   
   i=0;
   while (1) {
      lk204_set_cursor_pos(lineno,col[i]);
      du=lk204_get_key();
      if (du=='E') return i;
      if (du=='A') return -1;
      if ((du=='B') || (du=='D')) {
	 i--;
	 if (i<0) i=num_entries-1;
      }
      if ((du=='C') || (du=='H')) {
	 i++;
	 if (i>=num_entries) i=0;
      }
   }
}

int lk204_vert_menu(const char *prompt,int num_entries,const char **entries) {
   int scroll=-1;
   int i,j;
   char du;
   
   lk204_clear_screen();
   lk204_printf("%s",prompt);

   for (i=0;i<num_entries;i++) {
      lk204_set_cursor_pos(i+2,1);
      lk204_printf("%s",entries[i]);
      if (i>=2) break;
   }

   i=0;
   while (1) {
      lk204_set_cursor_pos(i-scroll+1,1);
      du=lk204_get_key();

      if (du=='E') return i;
      if (du=='A') return -1;
      if ((du=='B') || (du=='D')) {
	 i--;
	 if (i<0) i=num_entries-1;
      }
      if ((du=='C') || (du=='H')) {
	 i++;
	 if (i>=num_entries) i=0;
      }
      
      if (i-scroll<0) {
	 scroll=i;
	 lk204_clear_screen();
	 for (j=scroll;j<num_entries;j++) {
	    lk204_set_cursor_pos(j-scroll+1,1);
	    lk204_printf("%s",entries[j]);
	    if (j-scroll>=3)
	      break;
	 }
      }

      if (i-scroll>3) {
	 scroll=i-3;
	 lk204_clear_screen();
	 for (j=scroll;j<num_entries;j++) {
	    lk204_set_cursor_pos(j-scroll+1,1);
	    lk204_printf("%s",entries[j]);
	    if (j-scroll>=3)
	      break;
	 }
      }
   }
}

int lk204_grid_submenu(int lineno,int num_entries,const char **entries) {
/*
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
*/
}

/**********************************************************************/

int lk204_wait_for_key(int timeout) {
   struct timeval tv;
   fd_set fdset;
   int desc;
   
   tv.tv_sec=0;
   tv.tv_usec=timeout;
   FD_ZERO(&fdset);
   desc=fileno(lk204_in_fp);
   FD_SET(desc,&fdset);
   select(desc+1,&fdset,NULL,NULL,&tv);
   
   return FD_ISSET(desc,&fdset);
}
