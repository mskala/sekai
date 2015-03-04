/*
 * VCO test program
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

#include <string.h>
#include <unistd.h>
#include <math.h>

#include <alsa/asoundlib.h>

int main(int argc,char **argv) {
   snd_rawmidi_t *midi_out;
   int note,i;
   char out_buf[3];
   FILE *wav_file;
   unsigned char wav_data[192044];
   int state,first_fall,last_fall,num_falls;
   double freq,octaves;

   if (snd_rawmidi_open(NULL,&midi_out,"hw:1,0,0",
			SND_RAWMIDI_NONBLOCK|SND_RAWMIDI_APPEND)<0) {
      return 1;
   }
  
   while (1) {
      note=36+(rand()%49);
      
      printf("%d\t",note);

      usleep(100000);

      out_buf[0]=0x90;
      out_buf[1]=note;
      out_buf[2]=100;
      snd_rawmidi_write(midi_out,out_buf,3);
      
      usleep(100000);

      out_buf[0]=0x90;
      out_buf[1]=note;
      out_buf[2]=0;
      snd_rawmidi_write(midi_out,out_buf,3);
      
      usleep(100000);

      system("arecord -c1 -r192000 /tmp/test.wav -d3 2> /dev/null");
      
      usleep(100000);

      wav_file=fopen("/tmp/test.wav","r");
      fread(wav_data,1,384044,wav_file);
      fclose(wav_file);
      
      state=0;
      first_fall=0;
      last_fall=0;
      num_falls=0;
      
      for (i=44;i<384044;i++) {
	 if ((state!=1) && (wav_data[i]<0x7F)) {
	    if (state==2) {
	       if (num_falls==0)
		 first_fall=i;
	       num_falls++;
	       last_fall=i;
	    }
	    state=1;
	 } else if ((state==1) && (wav_data[i]>0x81))
	      state=2;
      }
      
      if ((num_falls>2) && (num_falls<60000)) {
	 freq=((double)(num_falls-1))*384000.0/((double)(last_fall-first_fall));
	 octaves=log(freq/440.0)/log(2.0);
/*	 printf("%d\t%d\n",last_fall-first_fall,num_falls); */
	 printf("%f\t%f\n",freq,octaves);
      } else
	puts("--");
   }
}
