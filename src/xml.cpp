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
 * Boston, MA 02110-1301 USA $Id: xml.cpp 139 2007-03-29 14:37:40Z
 * johmathe $ $Date: 2007-12-12 16:22:16 +0000 (Wed, 12 Dec 2007) $ 
 */

#include "xml.h"
#include "shot.h"

using namespace std;
class film;
class shot;

xmlChar *
xml::ConvertInput (const char *in, const char *encoding)
{
  xmlChar *out;
  int ret;
  int size;
  int out_size;
  int temp;
  xmlCharEncodingHandlerPtr handler;

  if (in == 0)
    return 0;

  handler = xmlFindCharEncodingHandler (encoding);

  if (!handler)
    {
      printf ("ConvertInput: no encoding handler found for '%s'\n", encoding ? encoding : "");
      return 0;
    }

  size = (int) strlen (in) + 1;
  out_size = size * 2 - 1;
  out = (unsigned char *) xmlMalloc ((size_t) out_size);

  if (out != 0)
    {
      temp = size - 1;
      ret = handler->input (out, &out_size, (const xmlChar *) in, &temp);
      if ((ret < 0) || (temp - size + 1))
	{
	  if (ret < 0)
	    {
	      printf ("ConvertInput: conversion wasn't successful.\n");
	    }
	  else
	    {
	      printf ("ConvertInput: conversion wasn't successful. converted: %i octets.\n", temp);
	    }

	  xmlFree (out);
	  out = 0;
	}
      else
	{
	  out = (unsigned char *) xmlRealloc (out, out_size + 1);
	  out[out_size] = 0;	/* null terminating out */
	}
    }
  else
    {
      printf ("ConvertInput: no mem\n");
    }

  return out;
}



void
xml::write_data (string & filename)
{
  int rc;
  xmlTextWriterPtr writer;
  xmlChar *tmp;
  xmlDocPtr doc;
  stringstream strflx;

  /*  Create a new XmlWriter for DOM, with no compression. */
  list < shot >::iterator il;
  writer = xmlNewTextWriterDoc (&doc, 0);

  rc = xmlTextWriterStartDocument (writer, NULL, MY_ENCODING, NULL);
  rc = xmlTextWriterStartElement (writer, BAD_CAST "shotdetect");
  tmp = ConvertInput ("IRI ShotDetect ", MY_ENCODING);
  rc = xmlTextWriterWriteComment (writer, tmp);

  if (tmp != NULL)
    xmlFree (tmp);

  xmlTextWriterStartElement (writer, BAD_CAST "content");

  xmlTextWriterStartElement (writer, BAD_CAST "head");


  xmlTextWriterStartElement (writer, BAD_CAST "media");
  xmlTextWriterWriteAttribute (writer, BAD_CAST "src", BAD_CAST f->get_ipath().c_str ());
  strflx.str ("");
  strflx << f->fps;
  xmlTextWriterWriteElement (writer, BAD_CAST "fps", BAD_CAST strflx.str ().c_str ());
  strflx.str ("");
  strflx << f->height;
  xmlTextWriterWriteElement (writer, BAD_CAST "height", BAD_CAST strflx.str ().c_str ());
  strflx.str ("");
  strflx << f->width;
  xmlTextWriterWriteElement (writer, BAD_CAST "width", BAD_CAST strflx.str ().c_str ());

  strflx.str ("");
  strflx << int (f->duration.mstotal);
  xmlTextWriterWriteElement (writer, BAD_CAST "duration", BAD_CAST strflx.str ().c_str ());

  strflx.str ("");
  strflx << f->nchannel;
  xmlTextWriterWriteElement (writer, BAD_CAST "nchannel", BAD_CAST strflx.str ().c_str ());
  xmlTextWriterStartElement (writer, BAD_CAST "codec");
  xmlTextWriterWriteElement (writer, BAD_CAST "video", BAD_CAST f->codec.video.c_str ());
  xmlTextWriterWriteElement (writer, BAD_CAST "audio", BAD_CAST f->codec.audio.c_str ());
  xmlTextWriterEndElement (writer);
  xmlTextWriterEndElement (writer);

  xmlTextWriterEndElement (writer);

  xmlTextWriterStartElement (writer, BAD_CAST "body");
  xmlTextWriterStartElement (writer, BAD_CAST "shots");

  /* Mise en place des elements shots */
  for (il = f->shots.begin (); il != f->shots.end (); il++)
    {
      strflx.str ("");
      strflx << (*il).myid;
      rc = xmlTextWriterStartElement (writer, BAD_CAST "shot");
      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "id", BAD_CAST strflx.str ().c_str ());

      strflx.str ("");
      strflx << (*il).fduration;
      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "fduration", BAD_CAST strflx.str ().c_str ());

      strflx.str ("");
      strflx << int ((*il).msduration);
      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "msduration", BAD_CAST strflx.str ().c_str ());

      strflx.str ("");
      strflx << (*il).fbegin;
      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "fbegin", BAD_CAST strflx.str ().c_str ());

      strflx.str ("");
      strflx << int ((*il).msbegin);
      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "msbegin", BAD_CAST strflx.str ().c_str ());

      /*
       * Element image 
       */

      if ((*il).img_begin != NULL)
	{
	  if (f->get_shot())
	    {
	      rc = xmlTextWriterStartElement (writer, BAD_CAST "img");
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "size", BAD_CAST "original");
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "type", BAD_CAST "in");

	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "src", BAD_CAST (*il).img_begin->img.c_str ());

	      strflx.str ("");
	      strflx << (*il).img_begin->width;
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "width", BAD_CAST strflx.str ().c_str ());

	      strflx.str ("");
	      strflx << (*il).img_begin->height;
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "height", BAD_CAST strflx.str ().c_str ());
	      rc = xmlTextWriterEndElement (writer);
	    }
	  /*
	   * Element thumb 
	   */
	  if (f->get_thumb())
	    {
	      rc = xmlTextWriterStartElement (writer, BAD_CAST "img");
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "size", BAD_CAST "thumb");
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "type", BAD_CAST "in");

	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "src", BAD_CAST (*il).img_begin->thumb.c_str ());

	      strflx.str ("");
	      strflx << (*il).img_begin->width_thumb;
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "width", BAD_CAST strflx.str ().c_str ());

	      strflx.str ("");
	      strflx << (*il).img_begin->height_thumb;
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "height", BAD_CAST strflx.str ().c_str ());
	      rc = xmlTextWriterEndElement (writer);
	    }
	}


      if ((*il).img_end != NULL)
	{
	  /*
	   * Element image 
	   */
	  if (f->get_shot())
	    {
	      rc = xmlTextWriterStartElement (writer, BAD_CAST "img");
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "size", BAD_CAST "original");
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "type", BAD_CAST "out");

	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "src", BAD_CAST (*il).img_end->img.c_str ());

	      strflx.str ("");
	      strflx << (*il).img_end->width;
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "width", BAD_CAST strflx.str ().c_str ());

	      strflx.str ("");
	      strflx << (*il).img_end->height;
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "height", BAD_CAST strflx.str ().c_str ());
	      rc = xmlTextWriterEndElement (writer);
	    }
	  /*
	   * Element thumb 
	   */
	  if (f->get_shot())
	    {
	      rc = xmlTextWriterStartElement (writer, BAD_CAST "img");
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "size", BAD_CAST "thumb");
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "type", BAD_CAST "out");

	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "src", BAD_CAST (*il).img_end->thumb.c_str ());

	      strflx.str ("");
	      strflx << (*il).img_end->width_thumb;
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "width", BAD_CAST strflx.str ().c_str ());

	      strflx.str ("");
	      strflx << (*il).img_end->height_thumb;
	      rc = xmlTextWriterWriteAttribute (writer, BAD_CAST "height", BAD_CAST strflx.str ().c_str ());
	      rc = xmlTextWriterEndElement (writer);
	    }

	}
      xmlTextWriterEndElement (writer);

    }

  rc = xmlTextWriterEndDocument (writer);
  if (rc < 0)
    {
      printf ("testXmlwriterDoc: Error at xmlTextWriterEndDocument\n");
      return;
    }

  xmlFreeTextWriter (writer);
  string xml_path;
  xml_path = f->get_opath();
  xml_path += "/result.xml";
  xmlSaveFileEnc (xml_path.c_str (), doc, MY_ENCODING);
  xmlFreeDoc (doc);

}

xml::xml (film * t)
{
  this->f = t;
}

xml::~xml ()
{
}
