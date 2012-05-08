 /*
 * Copyright (C) 2007 Johan MATHE - johan.mathe@tremplin-utc.net - Centre
 * Pompfidou - IRI This library is free software; you can redistribute it 
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version
 * 2.1 of the License, or (at your option) any later version. This
 * library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details. You should have received a copy of the GNU
 * Lesser General Public License along with this library; if not, write to 
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA $Id: film.cpp 141 2007-04-02 16:10:53Z
 * johmathe $ $Date: 2009-09-18 18:27:54 +0100 (Fri, 18 Sep 2009) $ 
 */
#include <sys/time.h>
#include <time.h>
#include "film.h"
#include "graph.h"
extern "C" {
#include <libswscale/swscale.h>
}
#define DEBUG

void
film::do_stats (int frame_num)
{
}

void
film::create_main_dir ()
{
  ostringstream str;
  struct stat *buf;
  str.str ("");
  str << this->global_path;
  buf = (struct stat *) malloc (sizeof (struct stat));
  if (stat (str.str ().c_str (), buf) == -1)
    {
#ifdef __WINDOWS__
      mkdir (str.str ().c_str ());
#else
      mkdir (str.str ().c_str (), 0777);
#endif

    }

}

void
film::CompareFrame (AVFrame * pFrame, AVFrame * pFramePrev)
{
  int y;
  int x;
  int diff;
  char c1, c2, c3;
  int c1tot, c2tot, c3tot;
  c1tot = 0;
  c2tot = 0;
  c3tot = 0;
  char c1prev, c2prev, c3prev;
  int score;
  score = 0;
  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
	{

	  c1 = (char) *(pFrame->data[0] + y * pFrame->linesize[0] + x * 3);
	  c2 = (char) *(pFrame->data[0] + y * pFrame->linesize[0] + x * 3 + 1);
	  c3 = (char) *(pFrame->data[0] + y * pFrame->linesize[0] + x * 3 + 2);

	  c1prev = (char) *(pFramePrev->data[0] + y * pFramePrev->linesize[0] + x * 3);
	  c2prev = (char) *(pFramePrev->data[0] + y * pFramePrev->linesize[0] + x * 3 + 1);
	  c3prev = (char) *(pFramePrev->data[0] + y * pFramePrev->linesize[0] + x * 3 + 2);
	  c1tot += int (c1 + 127);
	  c2tot += int (c2 + 127);
	  c3tot += int (c3 + 127);
	  score += abs ((c1 - c1prev));
	  score += abs ((c2 - c2prev));
	  score += abs ((c3 - c3prev));
	}
    }
  int nbpx = (height * width);

  /*
   * On se ramene à la moyenne 
   */
  score /= nbpx;
  c1tot /= nbpx;
  c2tot /= nbpx;
  c3tot /= nbpx;


  /*
   * Derivee numerique 
   */
  diff = abs (score - prev_score);
  prev_score = score;

  g->push_data (score, c1tot, c2tot, c3tot);

  if (diff > this->threshold && score > this->threshold)
    {
      shot s;
      s.fbegin = frame_number;
      s.msbegin = int ((frame_number * 1000) / fps);
      s.myid = shots.back ().myid + 1;

#ifdef DEBUG
      cerr << "Shot log :: " << s.msbegin << endl;
#endif

      /*
       * Convert to ms 
       */
      shots.back ().fduration = frame_number - shots.back ().fbegin;
      shots.back ().msduration = int (((shots.back ().fduration) * 1000) / fps);

      /*
       * Create images if necessary 
       */
      if (this->first_img_set)
	{
	  image *im_begin = new image (this, width, height, s.myid, BEGIN);
	  im_begin->SaveFrame (pFrame);
	  s.img_begin = im_begin;
	}
      if (this->last_img_set)
	{
	  image *im_end = new image (this, width, height, s.myid - 1, END);
	  im_end->SaveFrame (pFramePrev);
	  shots.back ().img_end = im_end;
	}
      shots.push_back (s);
    }
}

void
film::update_metadata ()
{
  char buf[256];
  /* Video metadata */
  if (videoStream != -1)
    {
      this->height = int (pFormatCtx->streams[videoStream]->codec->height);
      this->width = int (pFormatCtx->streams[videoStream]->codec->width);
      this->fps = av_q2d (pFormatCtx->streams[videoStream]->r_frame_rate);
      avcodec_string (buf, sizeof (buf), pFormatCtx->streams[videoStream]->codec, 0);
      this->codec.video = buf;
    }
  else
    {
      this->codec.video = "null";
      this->height = 0;
      this->width = 0;
      this->fps = 0;
    }

  /* Audio metadata */
  if (audioStream != -1)
    {
      avcodec_string (buf, sizeof (buf), pCodecCtxAudio, 0);
      this->codec.audio = buf;
      this->nchannel = pCodecCtxAudio->channels;
      this->samplerate = pCodecCtxAudio->sample_rate;
    }
  else
    {
      this->codec.audio = "null";
      this->nchannel = 0;
      this->samplerate = 0;
    }

  duration.secs = pFormatCtx->duration / AV_TIME_BASE;
  duration.us = pFormatCtx->duration % AV_TIME_BASE;
  duration.mstotal = int (duration.secs * 1000 + duration.us / 1000);
  duration.mins = duration.secs / 60;
  duration.secs %= 60;
  duration.hours = duration.mins / 60;
  duration.mins %= 60;

}

void
film::shotlog (string message)
{
#ifdef DEBUG
      cerr << "Shot log :: " << message << endl;
#endif
}

int
film::process ()
{
  int audioSize;
  uint8_t *buffer;
  uint8_t *buffer2;
  int frameFinished;
  int numBytes;
  shot s;
  static struct SwsContext *img_convert_ctx = NULL;

  create_main_dir ();

  string graphpath = this->global_path + "/";
  g = new graph (600, 400, graphpath, threshold, this);
  g->set_title ("Motion quantity");

  /*
   * Register all formats and codecs 
   */
  av_register_all ();

  if (av_open_input_file (&pFormatCtx, input_path.c_str (), NULL, 0, NULL) != 0)
    {
      string error_msg = "Impossible to open file";
      error_msg += input_path;
      shotlog (error_msg);
      return -1;		// Couldn't open file
    }

  /*
   * Retrieve stream information 
   */
  if (av_find_stream_info (pFormatCtx) < 0)
    return -1;			// Couldn't find stream information


  // dump_format (pFormatCtx, 0, path.c_str (), false);
  videoStream = -1;
  audioStream = -1;


  /*
   * Detect streams types 
   */
  for (int j = 0; j < pFormatCtx->nb_streams; j++)
    {
      switch (pFormatCtx->streams[j]->codec->codec_type)
	{
	case AVMEDIA_TYPE_VIDEO:
	  videoStream = j;
	  break;

	case AVMEDIA_TYPE_AUDIO:
	  audioStream = j;
	  break;

	default:
	  break;
	}
    }



  /*
   * Get a pointer to the codec context for the video stream 
   */
  if (audioStream != -1)
    {
          if (audio_set)
    {
      string xml_audio = graphpath + "/" + "audio.xml";
      init_xml (xml_audio);
    }

      pCodecCtxAudio = pFormatCtx->streams[audioStream]->codec;
      pCodecAudio = avcodec_find_decoder (pCodecCtxAudio->codec_id);

      if (pCodecAudio == NULL)
	return -1;		// Codec not found
      if (avcodec_open (pCodecCtxAudio, pCodecAudio) < 0)
	return -1;		// Could not open codec

    }
  update_metadata ();
  /*
   * Find the decoder for the video stream 
   */
  if (videoStream != -1)
    {
      pCodecCtx = pFormatCtx->streams[videoStream]->codec;
      pCodec = avcodec_find_decoder (pCodecCtx->codec_id);

      if (pCodec == NULL)
	return -1;		// Codec not found
      if (avcodec_open (pCodecCtx, pCodec) < 0)
	return -1;		// Could not open codec

      /*
       * Allocate video frame 
       */
      pFrame = avcodec_alloc_frame ();
      pFrameRGB = avcodec_alloc_frame ();
      pFrameRGBprev = avcodec_alloc_frame ();

      /*
       * Determine required buffer size and allocate buffer 
       */
      numBytes = avpicture_get_size (PIX_FMT_RGB24, width, height);

      buffer = (uint8_t *) malloc (sizeof (uint8_t) * numBytes);
      buffer2 = (uint8_t *) malloc (sizeof (uint8_t) * numBytes);

      /*
       * Assign appropriate parts of buffer to image planes in pFrameRGB 
       */
      avpicture_fill ((AVPicture *) pFrameRGB, buffer, PIX_FMT_RGB24, width, height);

      avpicture_fill ((AVPicture *) pFrameRGBprev, buffer2, PIX_FMT_RGB24, width, height);


      /*
       * Mise en place du premier plan 
       */
      s.fbegin = 0;
      s.msbegin = 0;
      s.myid = 0;
      shots.push_back (s);



    }


  checknumber = (samplerate * samplearg) / 1000;

  /*
   * Boucle de traitement principale du flux 
   */
  this->frame_number = 0;
  while (av_read_frame (pFormatCtx, &packet) >= 0)
    {
      if (packet.stream_index == videoStream)
	{
          AVPacket pkt;
          av_init_packet (&pkt);
          pkt.data = packet.data;
          pkt.size = packet.size;
          avcodec_decode_video2 (pCodecCtx, pFrame, &frameFinished, &pkt);

        if (frameFinished)
	    {
	      // Convert the image into RGB24
	      if (! img_convert_ctx)
		{
		  img_convert_ctx = sws_getContext(width, height, pCodecCtx->pix_fmt,
						   width, height, PIX_FMT_RGB24, SWS_BICUBIC, 
						   NULL, NULL, NULL);
		  if (! img_convert_ctx) 
		  {
		    fprintf(stderr, "Cannot initialize the conversion context!\n");
		    exit(1);
		  }
		}
	      
	      /* API: int sws_scale(SwsContext *c, uint8_t *src, int srcStride[], int srcSliceY, int srcSliceH, uint8_t dst[], int dstStride[] )
	       */
	      sws_scale(img_convert_ctx, pFrame->data, 
			pFrame->linesize, 0, 
			pCodecCtx->height,
			pFrameRGB->data, pFrameRGB->linesize);

	      /*
		Old API doc (cf http://www.dranger.com/ffmpeg/functions.html )
		int img_convert(AVPicture *dst, int dst_pix_fmt,
		                const AVPicture *src, int src_pix_fmt,
				int src_width, int src_height)
	      */
	      /*
	      img_convert ((AVPicture *) pFrameRGB, PIX_FMT_RGB24, (AVPicture *) pFrame, pCodecCtx->pix_fmt, width, height);
	      */

            this->frame_number ++;
	      /* Si ce n'est pas la permiere image */
        if ( this->frame_number  > 2)
		{
		  CompareFrame (pFrameRGB, pFrameRGBprev);
		}
	      else
		{
		  /*
		   * Cas ou c'est la premiere image, on cree la premiere image dans tous les cas 
		   */
		  image *begin_i = new image (this, width, height, s.myid, BEGIN);
		  begin_i->create_img_dir ();
		  begin_i->SaveFrame (pFrameRGB);
		  shots.back ().img_begin = begin_i;
		}
	     memcpy (buffer2, buffer, numBytes);
	    }
	}
      if (audio_set && (packet.stream_index == audioStream))
	{
	  process_audio ();
	}
      /*
       * Free the packet that was allocated by av_read_frame 
       */
      if (packet.data != NULL)
	av_free_packet (&packet);
    }

  if (videoStream != -1)
    {
      /* Mise en place de la dernière image */
      int lastFrame = this->frame_number;
      shots.back ().fduration = lastFrame - shots.back ().fbegin;
      shots.back ().msduration = int (((shots.back ().fduration) * 1000) / fps);
      duration.mstotal = int (shots.back ().msduration + shots.back ().msbegin);
	  image *end_i = new image (this, width, height, shots.back ().myid, END);
	  end_i->SaveFrame (pFrameRGB);
	  shots.back ().img_end = end_i;

      /*
       * Graphe de la qté de mvmt 
       */
      g->init_gd ();
      g->draw_all_canvas ();
      g->draw_color_datas ();
      g->draw_datas ();
      if (video_set)
	{
	  string xml_color = graphpath + "/" + "video.xml";
	  g->write_xml (xml_color);
	}
      g->save ();

      /*
       * Free the RGB images 
       */
      free (buffer);
      free (buffer2);
      av_free (pFrameRGB);
      av_free (pFrame);
      av_free (pFrameRGBprev);
      avcodec_close (pCodecCtx);
    }
  /*
   * Close the codec 
   */
  if (audioStream != -1)
    {
        /* Fermetrure du fichier xml */
      if (audio_set) close_xml ();
      avcodec_close (pCodecCtxAudio);
    }

  /*
   * Close the video file 
   */
  av_close_input_file (pFormatCtx);


}

void
film::init_xml (string filename)
{
  fd_xml_audio = fopen (filename.c_str (), "w");
  fprintf (fd_xml_audio, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<iri>\n<sound sampling=\"%d\" nchannels=\"%d\">", samplearg, nchannel);
}

int
film::close_xml ()
{
  fprintf (fd_xml_audio, "</sound>\n</iri>");
  return (fclose (fd_xml_audio));
}


void
film::process_audio ()
{
  int len1;
  int len;
  int data_size;
  static unsigned int samples_size = 0;
  int i;
  uint8_t *ptr;

  ptr = packet.data;
  len = packet.size;

  while (len > 0)
    {
      this->audio_buf = (short *) av_fast_realloc (this->audio_buf, &samples_size, FFMAX (packet.size, AVCODEC_MAX_AUDIO_FRAME_SIZE));
      data_size = samples_size;
      {
              AVPacket pkt;
              av_init_packet(&pkt);
              pkt.data = ptr;
              pkt.size = len;
              len1 = avcodec_decode_audio3 (pCodecCtxAudio, audio_buf, &data_size, &pkt);
      }


      if (len1 < 0)
	{
	  /*
	   * Error, breaking the frame 
	   */
	  len = 0;
	  break;
	}

      len -= len1;
      ptr += len1;

      if (data_size > 0)
	{

	  samples += data_size / (pCodecCtxAudio->channels * 2);
	  for (i = 0; i < data_size; i += 2 * pCodecCtxAudio->channels)
	    {
	      sample_right = *((signed short int *) (audio_buf + i));
	      /*
	       * Si un seul canal, le sample droit = sample gauche 
	       */
	      if (pCodecCtxAudio->channels >= 1)
		sample_left = *((signed short int *) (audio_buf + i + 2));
	      else
		sample_left = sample_right;

	      /*
	       * extraction des minimas et maximas 
	       */
	      if (minright > sample_right)
		minright = sample_right;
	      if (maxright < sample_right)
		maxright = sample_right;
	      if (minleft > sample_left)
		minleft = sample_left;
	      if (maxleft < sample_left)
		maxleft = sample_left;

	      /*
	       * Get sample 
	       */
	      if (ech++ == checknumber)
		{
		  if (minright > minleft)
		    minright = minleft;
		  if (maxright > maxleft)
		    maxright = maxleft;
		  fprintf (fd_xml_audio, "<v c1d =\"%d\" c1u=\"%d\" />\n", minright / RATIO, maxright / RATIO);
		  minright = MAX_INT;
		  maxright = MIN_INT;
		  minleft = MAX_INT;
		  maxleft = MIN_INT;

		  /*
		   * Reset sample number 
		   */
		  ech = 0;
		}
	    }
	}
    }
}

film::film()
{
  samplearg = 1000;
  samples = 0;
  minright = MAX_INT;
  maxright = MIN_INT;
  minleft = MAX_INT;
  maxleft = MIN_INT;
  ech = 0;
  nchannel = 1;
  audio_buf = NULL;
  this->first_img_set = false;
  this->last_img_set = false;
  this->audio_set = false;
  this->video_set = false;
  this->thumb_set = false;
  this->shot_set = false;

}
