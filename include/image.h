/* 
 * $Id: image.h 163 2007-07-03 21:13:08Z johmathe $
 * $Date: 2007-07-03 23:13:08 +0200 (Tue, 03 Jul 2007) $
 */


#ifndef __IMAGE_H__
#define __IMAGE_H__

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif 

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <string.h>
#include <string>
#include <stdlib.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "film.h"

#include <gd.h>

#define BEGIN true
#define END false
using namespace std;

class conf;
class film;


class image
{
  film* f;
public:
  int width;
  int height;
  int width_thumb;
  int height_thumb;
  string thumb;
  string img;
  int id;
  bool type;			// BEGIN || END
  int SaveFrame (AVFrame * pFrame);
  int create_img_dir ();
  image (film *, int, int, int, bool);
};

#endif /* !__IMAGE_H__ */
