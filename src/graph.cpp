/*
 * Copyright (C) 2007 Johan MATHE - johan.mathe@tremplin-utc.net - Centre
 * Pompidou - IRI This library is free software; you can redistribute it 
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version
 * 2.1 of the License, or (at your option) any later version. This
 * library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details. You should have received a copy of the GNU
 * Lesser General Public License along with this library; if not, write to 
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA $Id: graph.cpp 131 2007-03-22 15:46:52Z
 * johmathe $ $Date: 2008-12-20 15:21:16 +0000 (Sat, 20 Dec 2008) $ 
 */
#include "graph.h"

using namespace std;


float graph::MAX (float a, float b, float c)
{
  if ((a >= b) && (a >= c)) {
      return a;
  }
  else if ((b >= a) && (b >= c)) {
      return b;
  }
  else {
      return c;
  }
}


float graph::MIN (float a, float b, float c)
{
  if ((a <= b) && (a <= c)) {
      return a;
  }
  else if ((b <= a) && (b <= c)) {
      return b;
  }
  else {
      return c;
  }
}


graph::graph (int x, int y, string path, int th, film * farg)
{
  this->f = farg;
  this->xsize = 600;
  this->ysize = y;
  this->xaxis_offset = 250;
  this->xoffset = 10;
  this->yoffset = 10;
  this->size_graduation = 3;
  this->grid_size = 10;
  this->threseold = th;
  this->global_path = path;
  string csvpath = f->get_opath() + "/motion.csv"  ;
  printf("%s\n",csvpath.c_str());
  fdcsv = fopen(csvpath.c_str(),"wb");
}


graph::~graph () { }


void
graph::save ()
{
  gdImagePng (im_colors, fdgraph_colors);
  gdImagePng (im_qte_mvmt, fdqte_mvmt);
  gdImagePng (im_hsv, fdgraph_hsv);
  gdImageDestroy (im_qte_mvmt);
  gdImageDestroy (im_colors);
  gdImageDestroy (im_hsv);
  fclose (fdcsv);
  fclose (fdgraph_hsv);
  fclose (fdgraph_colors);
  fclose (fdqte_mvmt);
}


void
graph::init_gd ()
{

  if (data.size () > xsize) {
      xsize = data.size () + 20;
  }

  // Create image buffers 
  
  im_colors = gdImageCreate (xsize, ysize);
  im_hsv = gdImageCreateTrueColor (xsize, ysize);
  im_qte_mvmt = gdImageCreate (xsize, ysize);

  // Declare color indexes 
  
  background_color = gdImageColorAllocate (im_qte_mvmt, 255, 255, 255);
  line_color = gdImageColorAllocate (im_qte_mvmt, 0, 0, 0);
  title_color = gdImageColorAllocate (im_qte_mvmt, 255, 255, 0);
  grid_color = gdImageColorAllocate (im_qte_mvmt, 0, 0, 0);
  threseold_color = gdImageColorAllocate (im_qte_mvmt, 255, 0, 0);

  // Declare color indexes for the color graph 
  
  background_color = gdImageColorAllocate (im_colors, 255, 255, 255);
  line_color = gdImageColorAllocate (im_colors, 0, 0, 0);
  title_color = gdImageColorAllocate (im_colors, 255, 255, 0);
  grid_color = gdImageColorAllocate (im_colors, 0, 0, 0);
  red_color = gdImageColorAllocate (im_colors, 255, 0, 0);
  green_color = gdImageColorAllocate (im_colors, 0, 255, 0);
  blue_color = gdImageColorAllocate (im_colors, 0, 0, 255);

  // Open file descriptors (POSIX C) 
  
  filename_colors = global_path + "/colors.png";
  filename_qte_mvmt = global_path + "/qte_mvmt.png";
  filename_hsv = global_path + "/hsv.png";
  fdgraph_hsv = fopen (filename_hsv.c_str (), "wb");
  fdgraph_colors = fopen (filename_colors.c_str (), "wb");
  fdqte_mvmt = fopen (filename_qte_mvmt.c_str (), "wb");
}

void graph::set_title (string title) { }

void graph::draw_all_canvas ()
{

  int back_true_color;
  string str = "Couleurs";
  draw_canvas (im_colors, str);
  str = "Motion quantity";
  draw_canvas (im_qte_mvmt, str);
  str = "Color spaces";

  back_true_color = gdTrueColor (255, 255, 255);
  gdImageFilledRectangle (im_hsv, 0, 0, xsize, ysize, back_true_color);

  int grid_true_color = gdTrueColor (0, 0, 0);
  string title = "Color in function of time";
  gdImageString (im_hsv, gdFontGetLarge (), im_hsv->sx / 2 - (title.size () * gdFontGetLarge ()->w / 2), 10, (unsigned char *) title.c_str (), grid_true_color);

  // Yaxis 
  gdImageLine (im_hsv, xoffset, yoffset, xoffset, ysize - yoffset, grid_true_color);

  // graduation 
  for (int i = yoffset; i < ysize - yoffset; i++) {
    if (!(i % grid_size)) {
	    gdImageLine (im_hsv, xoffset - 2, i, xoffset + 2, i, grid_true_color);
	  }
  }

  // Xaxis 
  gdImageLine (im_hsv, xoffset, xaxis_offset, xsize - xoffset, xaxis_offset, grid_color);

  for (int i = xoffset; i < xsize - xoffset; i++) {
    if (!(i % grid_size)) {
	    gdImageLine (im_hsv, i, xaxis_offset - 2, i, xaxis_offset + 2, grid_color);
	  }
  }
}

void graph::draw_canvas (gdImagePtr im, string title)
{
  gdImageString (im, gdFontGetLarge (), im->sx / 2 - (title.size () * gdFontGetLarge ()->w / 2), 10, (unsigned char *) title.c_str (), grid_color);
  
  //  Yaxis 
  gdImageLine (im, xoffset, yoffset, xoffset, ysize - yoffset, grid_color);

  // graduation 
  for (int i = yoffset; i < ysize - yoffset; i++) {
    if (!(i % grid_size)) {
	  gdImageLine (im, xoffset - 2, i, xoffset + 2, i, grid_color);
	  }
  }

  // Xaxis 
  gdImageLine (im, xoffset, xaxis_offset, xsize - xoffset, xaxis_offset, grid_color);

  for (int i = xoffset; i < xsize - xoffset; i++) {
    if (!(i % grid_size)) {
	    gdImageLine (im, i, xaxis_offset - 2, i, xaxis_offset + 2, grid_color);
	  }
  }
}


void
graph::draw_datas ()
{
  // data drawing
  for (int i = 1; i < data.size () - 1; i++) {
    gdImageLine (im_qte_mvmt, i - 1 + xoffset, (-data[i - 1].global) +xaxis_offset, i + xoffset, (-data[i].global) +xaxis_offset, line_color);
    fprintf(fdcsv,"%d,",data[i].global);
  }

  // Display thresold
  gdImageLine (im_qte_mvmt, xoffset, xaxis_offset - threseold, xsize - xoffset, xaxis_offset - threseold, threseold_color);
}


void graph::hsv_to_rgb (float *r, float *g, float *b, float h, float s, float v)
{
  int i;
  float f, p, q, t;

  if (s == 0) {
    // achromatic (grey)
    *r = *g = *b = v;
    return;
  }

  h /= 60;			// sector 0 to 5
  i = int (h);
  f = h - i;			// factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));

  switch (i)
  {
    case 0:
      *r = v;
      *g = t;
      *b = p;
      break;
    case 1:
      *r = q;
      *g = v;
      *b = p;
      break;
    case 2:
      *r = p;
      *g = v;
      *b = t;
      break;
    case 3:
      *r = p;
      *g = q;
      *b = v;
      break;
    case 4:
      *r = t;
      *g = p;
      *b = v;
      break;
    default:			// case 5:
      *r = v;
      *g = p;
      *b = q;
      break;
    }

}


void
graph::rgb_to_hsv (float r, float g, float b, float *h, float *s, float *v)
{
  float min, max, delta;

  min = MIN (r, g, b);
  max = MAX (r, g, b);
  *v = max;			// v

  delta = max - min;

  if (max != 0)
    *s = delta / max;		// s
  else
    {
      // r = g = b = 0 // s = 0, v is undefined
      *s = 0;
      *h = -1;
      return;
    }

  if (r == max)
    *h = (g - b) / delta;	// between yellow & magenta
  else if (g == max)
    *h = 2 + (b - r) / delta;	// between cyan & yellow
  else
    *h = 4 + (r - g) / delta;	// between magenta & cyan

  *h *= 60;			// degrees
  if (*h < 0)
    *h += 360;

}


void graph::draw_color_datas () {
  int hsv_color;
  float r, v, b;
  // Red 
  for (int i = 1; i < data.size () - 1; i++) {
      gdImageLine (im_colors, i - 1 + xoffset, (-data[i - 1].red) + xaxis_offset, i + xoffset, (-data[i].red) + xaxis_offset, red_color);
      gdImageLine (im_colors, i - 1 + xoffset, (-data[i - 1].green) + xaxis_offset, i + xoffset, (-data[i].green) + xaxis_offset, green_color);
      gdImageLine (im_colors, i - 1 + xoffset, (-data[i - 1].blue) + xaxis_offset, i + xoffset, (-data[i].blue) + xaxis_offset, blue_color);
      hsv_to_rgb (&r, &v, &b, data[i].hue, float (1), float (1));
      hsv_color = gdTrueColor (int (r * 255), int (v * 255), int (b * 255));
      gdImageLine (im_hsv, i + xoffset, (0) + xaxis_offset - 1, i + xoffset, (int ((-data[i].saturation) * 255)) +xaxis_offset - 1, hsv_color);
  }
}

void
graph::write_xml (string filename)
{
  fd_xml = fopen (filename.c_str (), "w");
  fprintf (fd_xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<iri>\n<frame>");

  for (int i = 0; int (double (i) * (f->fps)) < data.size () - 1; i++) {
      float r;
      float v;
      float b;

      hsv_to_rgb (&r, &v, &b, data[int (double (i) * (f->fps))].hue, float (1), float (1));

      fprintf (fd_xml, "<v m=\"%d\" r=\"%d\" g=\"%d\" b=\"%d\" s=\"%d\" />\n", data[i].global, int (r * 255), int (v * 255), int (b * 255), int (data[int (double (i) * (f->fps))].saturation * 100));
    }
  fprintf (fd_xml, "</frame>\n</iri>");
  fclose (fd_xml);

}
