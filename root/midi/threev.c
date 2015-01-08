/*
 * Three-voice (actually four) MIDI splitter
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

#include <alsa/asoundlib.h>

#include "lk204.h"

/**********************************************************************/

typedef struct _DEVICE_INFO {
   int card,dev,sub;
} DEVICE_INFO;

DEVICE_INFO devices[100];
char *dev_names[100];
int num_devices;

typedef struct _VOICE_INFO {
   int current_dev,target_dev;
   int current_channel,target_channel;
   int note,busy;
   snd_rawmidi_t *midi_output;
} VOICE_INFO;

VOICE_INFO voices[4]={
   {-1,0,3,3,-1,0,NULL},
   {-1,0,4,4,-1,0,NULL},
   {-1,1,2,2,-1,0,NULL},
   {-1,-1,1,1,-1,0,NULL}
};

int voice_sequence=0;

void note_on(int chan,int note,int vel) {
   int i,voice=-1;
   char out_buf[3];
   
   /* look for free voice */
   for (i=0;(voice<0) && (i<4);i++)
     if ((voices[(i+voice_sequence)%4].current_dev>=0) &&
	 (!voices[(i+voice_sequence)%4].busy))
       voice=(i+voice_sequence)%4;
   
   /* look for any voice */
   for (i=0;(voice<0) && (i<4);i++)
     if (voices[(i+voice_sequence)%4].current_dev>=0)
       voice=(i+voice_sequence)%4;

   /* there better be SOME voice we can use */
   if (voice<0)
     return;
   
   /* record current sequence */
   voice_sequence=(voice+1)%4;
   
   /* stop current note, if any */
   if (voices[voice].busy) {
      out_buf[0]=voices[voice].current_channel+0x8F;
      out_buf[1]=voices[voice].note;
      out_buf[2]=0;
      snd_rawmidi_write(voices[voice].midi_output,out_buf,3);
   } else if (voice<3) {
      lk204_led_red(voice+1);
   }
   
   /* start new note */
   out_buf[0]=voices[voice].current_channel+0x8F;
   out_buf[1]=note;
   out_buf[2]=vel;
   snd_rawmidi_write(voices[voice].midi_output,out_buf,3);
   
   /* record it */
   voices[voice].note=note;
   voices[voice].busy=1;
}

void note_off(int chan,int note,int vel) {
   int i;
   char out_buf[3];
   
   for (i=0;i<4;i++)
     if ((voices[i].current_dev>=0) && voices[i].busy &&
	 (voices[i].note==note)) {
	if (vel==0x40) {
	   out_buf[0]=voices[i].current_channel+0x8F;
	   out_buf[1]=note;
	   out_buf[2]=0;
	} else {
	   out_buf[0]=voices[i].current_channel+0x7F;
	   out_buf[1]=note;
	   out_buf[2]=vel;
	}
	snd_rawmidi_write(voices[i].midi_output,out_buf,3);
	voices[i].busy=0;
	if (i<3)
	  lk204_led_green(i+1);
     }
}

int main(int argc,char **argv) {
   int i,j,k;
   int num_devices;
   snd_ctl_t *snd_ctl;
   char name[32];
   snd_rawmidi_info_t *info;
   const char *sup_name;
   const char *sub_name;
   char *new_name;
   int subs_in,subs_out;
   int input_device;
   int finished=0;
   int curs_row,curs_col;
   int adjust_direction=1;
   snd_rawmidi_t *midi_input;
   char midi_in_buf[1024];
   int midi_in_state=0;
   int midi_in_channel=-1,midi_in_note=-1,midi_in_vel=-1;
   
   lk204_open((argc>1) && (strcmp(argv[1],"-t")==0));

   /* find all rawMIDI devices */
   for (i=-1,num_devices=0;(snd_card_next(&i)>=0) && (i>=0);) {
      sprintf(name,"hw:%d",i);
      if (snd_ctl_open(&snd_ctl,name,0)<0) continue;
      for (j=-1;(snd_ctl_rawmidi_next_device(snd_ctl,&j)>=0) && (j>=0);) {
	 snd_rawmidi_info_malloc(&info);
	 snd_rawmidi_info_set_device(info,j);

	 snd_rawmidi_info_set_stream(info,SND_RAWMIDI_STREAM_INPUT);
	 if (snd_ctl_rawmidi_info(snd_ctl,info)>=0)
	   subs_in=snd_rawmidi_info_get_subdevices_count(info);
	 else
	   subs_in=0;

	 snd_rawmidi_info_set_stream(info,SND_RAWMIDI_STREAM_OUTPUT);
	 if (snd_ctl_rawmidi_info(snd_ctl,info)>=0)
	   subs_out=snd_rawmidi_info_get_subdevices_count(info);
	 else
	   subs_out=0;
	 
	 for (k=0;(k<subs_in) || (k<subs_out);k++) {
	    snd_rawmidi_info_set_stream(info,k<subs_in?
					SND_RAWMIDI_STREAM_INPUT:
					SND_RAWMIDI_STREAM_OUTPUT);
	    snd_rawmidi_info_set_subdevice(info,k);

	    if (snd_ctl_rawmidi_info(snd_ctl,info)<0)
	      continue;
	    sup_name=snd_rawmidi_info_get_name(info);
	    sub_name=snd_rawmidi_info_get_subdevice_name(info);
	    
	    new_name=malloc(strlen(sup_name)+16);
	    if ((k==0) && (sub_name[0]=='\0')) {
	       sprintf(new_name,"%d,%d %s",i,j,sup_name);
	       k=-1;
	    } else
	      sprintf(new_name,"%d,%d,%d %s",i,j,k,sub_name);

	    new_name[20]='\0';
	    
	    dev_names[num_devices]=new_name;
	    devices[num_devices].card=i;
	    devices[num_devices].dev=j;
	    devices[num_devices].sub=k;
	    
	    num_devices++;
	    
	    if ((k<=0) && (sub_name[0]=='\0')) break;
	 }
	 
	 free(info);
      }
      snd_ctl_close(snd_ctl);
   }

   /* choose input device */
   input_device=lk204_vert_menu("Choose input device",num_devices,
				(const char **)dev_names);
   if (input_device<0) {
      lk204_close();
      return 1;
   }
   
   /* make device names exactly 12 chars */
   for (i=0;i<num_devices;i++) {
      for (j=strlen(dev_names[i]);j<12;j++)
	dev_names[i][j]=' ';
      dev_names[i][12]='\0';
   }
   
   /* draw initial screen */
   lk204_led_green(1);
   lk204_led_green(2);
   lk204_led_green(3);
   lk204_go_home();
   lk204_set_cursor_pos(1,1);
   lk204_printf("--   %s  1",dev_names[0]);
   lk204_set_cursor_pos(2,1);
   lk204_printf("--   %s  2",dev_names[0]);
   lk204_set_cursor_pos(3,1);
   lk204_printf("--   %s  3",dev_names[0]);
   lk204_set_cursor_pos(4,1);
   lk204_printf("--   ------------  -");
   lk204_set_cursor_pos(1,6);
   
   curs_row=1;
   curs_col=6;
  
   /* open MIDI input */
   if (devices[input_device].sub>=0)
     sprintf(name,"hw:%d,%d,%d",devices[input_device].card,
	     devices[input_device].dev,devices[input_device].sub);
   else
     sprintf(name,"hw:%d,%d",devices[input_device].card,
	     devices[input_device].dev);
   if (snd_rawmidi_open(&midi_input,NULL,name,SND_RAWMIDI_NONBLOCK)<0) {
      lk204_close();
      return 1;
   }
   
   /* main loop */
   while (!finished) {

      /* put voices on target */
      for (i=0;i<4;i++)
	 if (!(voices[i].busy)) {
	    if (voices[i].current_dev!=voices[i].target_dev) {
	       if (voices[i].current_dev>=0) {
		  for (j=0;j<4;j++)
		    if ((i!=j) &&
			(voices[j].current_dev==voices[i].current_dev))
		      break;
		  if (j>=4)
		    snd_rawmidi_close(voices[i].midi_output);
	       }
	       
	       if (voices[i].target_dev>=0) {
		  for (j=0;j<4;j++)
		    if ((i!=j) &&
			(voices[j].current_dev==voices[i].target_dev))
		      break;
		  if (j<4)
		    voices[i].midi_output=voices[j].midi_output;
		  else {
		     if (devices[voices[i].target_dev].sub>=0)
		       sprintf(name,"hw:%d,%d,%d",
			       devices[voices[i].target_dev].card,
			       devices[voices[i].target_dev].dev,
			       devices[voices[i].target_dev].sub);
		     else
		       sprintf(name,"hw:%d,%d",
			       devices[voices[i].target_dev].card,
			       devices[voices[i].target_dev].dev);
		     j=snd_rawmidi_open(NULL,
					&(voices[i].midi_output),
					name,SND_RAWMIDI_NONBLOCK|
					SND_RAWMIDI_APPEND);
		     if (j<0) {
			lk204_close();
			puts(snd_strerror(j));
			return 1;
		     }
		  }
	       }

	       voices[i].current_dev=voices[i].target_dev;
	    }
	    voices[i].current_channel=voices[i].target_channel;
	 }

      /* poll for key */
      if (lk204_wait_for_key(10000)) {
	 switch (lk204_get_key()) {
	  case 'B': /* up */
	    curs_row--;
	    if (curs_row<1) curs_row=4;
	    lk204_set_cursor_pos(curs_row,curs_col);
	    break;

	  case 'H': /* down */
	    curs_row++;
	    if (curs_row>4) curs_row=1;
	    lk204_set_cursor_pos(curs_row,curs_col);
	    break;

	  case 'C': /* right */
	  case 'D': /* left */
	    curs_col=curs_col>=19?6:20;
	    lk204_set_cursor_pos(curs_row,curs_col);
	    break;

	  case 'E': /* enter */
	    if (curs_col<19) {
	       voices[curs_row-1].target_dev+=adjust_direction;
	       if (voices[curs_row-1].target_dev<-1)
		 voices[curs_row-1].target_dev=num_devices-1;
	       if (voices[curs_row-1].target_dev>=num_devices)
		 voices[curs_row-1].target_dev=-1;
	       lk204_set_cursor_pos(curs_row,6);
	       if (voices[curs_row-1].target_dev==-1)
		 lk204_printf("------------  -");
	       else
		 lk204_printf("%s %2d",
			      dev_names[voices[curs_row-1].target_dev],
			      voices[curs_row-1].target_channel);
	       lk204_set_cursor_pos(curs_row,6);

	    } else if (voices[curs_row-1].target_dev!=-1) {
	       voices[curs_row-1].target_channel+=adjust_direction;
	       if (voices[curs_row-1].target_channel<1)
		 voices[curs_row-1].target_channel=16;
	       if (voices[curs_row-1].target_channel>16)
		 voices[curs_row-1].target_channel=1;
	       lk204_set_cursor_pos(curs_row,19);
	       lk204_printf("%2d",voices[curs_row-1].target_channel);
	       lk204_set_cursor_pos(curs_row,20);

	    }
	    break;

	  case 'G': /* down-left */
	    adjust_direction=-adjust_direction;
	    break;

	  case 'A': /* up-left */
	  default:
	      finished=1;
	    break;
	 }
      }
      
      /* poll for MIDI data */
      while ((i=snd_rawmidi_read(midi_input,midi_in_buf,
				 sizeof(midi_in_buf)))>0)
	for (j=0;j<i;j++) {
	  switch (midi_in_state) {
	   case 1: /* NOTE OFF waiting for key */
	     if ((midi_in_buf[j]&0x80)==0) {
		midi_in_note=midi_in_buf[j];
		midi_in_state=2;
	     } else {
		midi_in_state=0;
		j--;
		break;
	     }
	     break;
	     
	   case 2: /* NOTE OFF waiting for velocity */
	     if ((midi_in_buf[j]&0x80)==0) {
		midi_in_vel=midi_in_buf[j];
		note_off(midi_in_channel,midi_in_note,midi_in_vel);
		midi_in_state=1;
	     } else {
		midi_in_state=0;
		j--;
		break;
	     }
	     break;
	     
	   case 3: /* NOTE ON waiting for key */
	     if ((midi_in_buf[j]&0x80)==0) {
		midi_in_note=midi_in_buf[j];
		midi_in_state=4;
	     } else {
		midi_in_state=0;
		j--;
		break;
	     }
	     break;

	   case 4: /* NOTE ON waiting for velocity */
	     if ((midi_in_buf[j]&0x80)==0) {
		midi_in_vel=midi_in_buf[j];
		if (midi_in_vel==0)
		  note_off(midi_in_channel,midi_in_note,0x40);
		else
		  note_on(midi_in_channel,midi_in_note,midi_in_vel);
		midi_in_state=3;
	     } else {
		midi_in_state=0;
		j--;
		break;
	     }
	     break;
	     
	   case 0: /* no message in progress */
	   default:
	       if ((midi_in_buf[j]&0xF0)==0x80) {
		  midi_in_state=1;
		  midi_in_channel=midi_in_buf[j]&0xF;
	       } else if ((midi_in_buf[j]&0xF0)==0x90) {
		  midi_in_state=3;
		  midi_in_channel=midi_in_buf[j]&0xF;
	       }
	       break;
	  }
	}
   }
   
   /* close MIDI */
   snd_rawmidi_close(midi_input);
   for (i=0;i<4;i++)
     if (voices[i].current_dev>=0) {
	for (j=0;j<i;j++)
	  if (voices[j].current_dev==voices[i].current_dev)
	    break;
	if (j>=i)
	  snd_rawmidi_close(voices[i].midi_output);
     }

   lk204_close();
   return 0;
}
