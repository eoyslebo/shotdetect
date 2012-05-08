/* 
 * $Id: shot.h 117 2007-03-05 14:11:26Z johmathe $
 * $Date: 2007-03-05 15:11:26 +0100 (Mon, 05 Mar 2007) $
 */

#ifndef __SHOT_H__
#define __SHOT_H__
#include "image.h"

class image;

class shot
{
private:
public:
  int myid;

  /* Duration in frame */
  unsigned int fduration;
  /* Starting point (frame) */
  unsigned int fbegin;
  /* Duration in ms */
  double msduration;
  /* Beggining en ms */
  double msbegin;
  /* img */
  image *img_begin;
  image *img_end;
  shot ();
  ~shot ();
};

#endif /* !__SHOT_H__ */
