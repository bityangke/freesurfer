/*
 *       FILE NAME:   mri.c
 *
 *       DESCRIPTION: utilities for MRI  data structure
 *
 *       AUTHOR:      Bruce Fischl
 *       DATE:        1/8/97
 *
*/

/*-----------------------------------------------------
                    INCLUDE FILES
-------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <memory.h>

#include "error.h"
#include "proto.h"
#include "mri.h"
#include "macros.h"
#include "diag.h"
#include "volume_io.h"
#include "filter.h"
#include "box.h"
#include "region.h"
#include "nr.h"

/*-----------------------------------------------------
                    MACROS AND CONSTANTS
-------------------------------------------------------*/

#define DEBUG_POINT(x,y,z)  (((x==21) && (y==14)) &&((z)==7))

/*-----------------------------------------------------
                    STATIC DATA
-------------------------------------------------------*/

/*-----------------------------------------------------
                    STATIC PROTOTYPES
-------------------------------------------------------*/
/*-----------------------------------------------------
                    GLOBAL FUNCTIONS
-------------------------------------------------------*/
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIunion(MRI *mri1, MRI *mri2, MRI *mri_dst)
{
  int     width, height, depth, x, y, z ;
  BUFTYPE *p1, *p2, *pdst, v1, v2 ;

  width = mri1->width ;
  height = mri1->height ;
  depth = mri1->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri1, NULL) ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      p1 = &MRIvox(mri1, 0, y, z) ;
      p2 = &MRIvox(mri2, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        v1 = *p1++ ;
        v2 = *p2++ ;
        *pdst++ = MAX(v1, v2) ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIintersect(MRI *mri1, MRI *mri2, MRI *mri_dst)
{
  int     width, height, depth, x, y, z ;
  BUFTYPE *p1, *p2, *pdst, v1, v2 ;

  width = mri1->width ;
  height = mri1->height ;
  depth = mri1->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri1, NULL) ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      p1 = &MRIvox(mri1, 0, y, z) ;
      p2 = &MRIvox(mri2, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        v1 = *p1++ ;
        v2 = *p2++ ;
        *pdst++ = MIN(v1, v2) ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIcomplement(MRI *mri_src, MRI *mri_dst)
{
  int     width, height, depth, x, y, z ;
  BUFTYPE *psrc, *pdst, b ;

  width = mri_src->width ;
  height = mri_src->height ;
  depth = mri_src->depth ;

  if (!mri_dst)
  {
    mri_dst = MRIalloc(width, height, depth, mri_src->type) ;
    MRIcopyHeader(mri_src, mri_dst) ;
  }

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      psrc = mri_src->slices[z][y] ;
      pdst = mri_dst->slices[z][y] ;
      for (x = 0 ; x < width ; x++)
      {
        b = *psrc++ ;
        *pdst++ = !b ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIxor(MRI *mri1, MRI *mri2, MRI *mri_dst, int t1, int t2)
{
  int     width, height, depth, x, y, z ;
  BUFTYPE *p1, *p2, *pdst, v1, v2 ;

  if ((mri1->type != MRI_UCHAR) || (mri2->type != MRI_UCHAR))
    ErrorReturn(NULL, 
                (ERROR_UNSUPPORTED, "MRIxor: inputs must be UCHAR")) ;

  width = mri1->width ;
  height = mri1->height ;
  depth = mri1->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri1, NULL) ;
  else if (mri_dst->type != MRI_UCHAR)
    ErrorReturn(NULL, 
                (ERROR_UNSUPPORTED, "MRIxor: destination must be UCHAR")) ;


  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      p1 = &MRIvox(mri1, 0, y, z) ;
      p2 = &MRIvox(mri2, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        v1 = *p1++ ;
        v2 = *p2++ ;
        v1 = ((v1 >= t1) && (v1 <= t2)) ;
        v2 = ((v2 >= t1) && (v2 <= t2)) ;
        *pdst++ = v1 ^ v2 ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
#define KLEN 3
MRI *
MRImorph(MRI *mri_src, MRI *mri_dst, int which)
{
  BUFTYPE kernel[KLEN][KLEN][KLEN] ;

  switch (which)
  {
  case FILTER_OPEN:
    memset(kernel, 1, sizeof(BUFTYPE)*KLEN*KLEN*KLEN) ;
    break ;
  case FILTER_CLOSE:
    break ;
  case FILTER_DILATE:
    memset(kernel, 1, sizeof(BUFTYPE)*KLEN*KLEN*KLEN) ;
    break ;
  case FILTER_ERODE:
    memset(kernel, 1, sizeof(BUFTYPE)*KLEN*KLEN*KLEN) ;
    break ;
  default:
    ErrorReturn(NULL, 
                (ERROR_UNSUPPORTED, "MRImorph: unknown type %d", which)) ;
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIerode(MRI *mri_src, MRI *mri_dst)
{
  int     width, height, depth, x, y, z, x0, y0, z0, xi, yi, zi, same ;
  BUFTYPE *pdst, min_val, val ;

  width = mri_src->width ;
  height = mri_src->height ;
  depth = mri_src->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri_src, NULL) ;

  if (mri_dst == mri_src)
  {
    same = 1 ;
    mri_dst = MRIclone(mri_src, NULL) ;
  }
  else
    same = 0 ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        min_val = 255 ;
        for (z0 = -1 ; z0 <= 1 ; z0++)
        {
          zi = mri_src->zi[z+z0] ;
          for (y0 = -1 ; y0 <= 1 ; y0++)
          {
            yi = mri_src->yi[y+y0] ;
            for (x0 = -1 ; x0 <= 1 ; x0++)
            {
              xi = mri_src->xi[x+x0] ;
              val = MRIvox(mri_src, xi,yi,zi) ;
              if (val < min_val)
                min_val = val ;
            }
          }
        }
        *pdst++ = min_val ;
      }
    }
  }
  if (same)
  {
    MRIcopy(mri_dst, mri_src) ;
    MRIfree(&mri_dst) ;
    mri_dst = mri_src ;
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIdilate(MRI *mri_src, MRI *mri_dst)
{
  int     width, height, depth, x, y, z, x0, y0, z0, xi, yi, zi, same,
          xmin, xmax, ymin, ymax, zmin, zmax;
  BUFTYPE *psrc, max_val, val ;

  width = mri_src->width ;
  height = mri_src->height ;
  depth = mri_src->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri_src, NULL) ;

  if (mri_dst == mri_src)
  {
    same = 1 ;
    mri_dst = MRIclone(mri_src, NULL) ;
  }
  else
    same = 0 ;

  xmax = 0 ; xmin = width-1 ;
  ymax = 0 ; ymin = height-1 ;
  zmax = 0 ; zmin = depth-1 ;
  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      psrc = &MRIvox(mri_src, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        if (*psrc++ > 0)
        {
          if (x-1 < xmin)
            xmin = x-1 ;
          if (x+1 > xmax)
            xmax = x+1 ;
          if (y-1 < ymin)
            ymin = y-1 ;
          if (y+1 > ymax)
            ymax = y+1 ;
          if (z-1 < zmin)
            zmin = z-1 ;
          if (z+1 > zmax)
            zmax = z+1 ;
        }
      }
    }
  }
  xmin = MAX(0, xmin) ; xmax = MIN(width-1, xmax) ;
  ymin = MAX(0, ymin) ; ymax = MIN(height-1, ymax) ;
  zmin = MAX(0, zmin) ; zmax = MIN(depth-1, zmax) ;
  for (z = zmin ; z <= zmax ; z++)
  {
    for (y = ymin ; y <= ymax ; y++)
    {
      for (x = xmin ; x <= xmax ; x++)
      {
        max_val = 0 ;
        for (z0 = -1 ; z0 <= 1 ; z0++)
        {
          zi = mri_src->zi[z+z0] ;
          for (y0 = -1 ; y0 <= 1 ; y0++)
          {
            yi = mri_src->yi[y+y0] ;
            for (x0 = -1 ; x0 <= 1 ; x0++)
            {
              xi = mri_src->xi[x+x0] ;
              val = MRIvox(mri_src, xi,yi,zi) ;
              if (val > max_val)
                max_val = val ;
            }
          }
        }
        MRIvox(mri_dst, x,y,z) = max_val ;
      }
    }
  }
  if (same)
  {
    MRIcopy(mri_dst, mri_src) ;
    MRIfree(&mri_dst) ;
    mri_dst = mri_src ;
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIopen(MRI *mri_src, MRI *mri_dst)
{
  MRI *mri_tmp ;

  mri_tmp = MRIerode(mri_src, NULL) ;
  mri_dst = MRIdilate(mri_tmp, mri_dst) ;
  MRIfree(&mri_tmp) ;
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIclose(MRI *mri_src, MRI *mri_dst)
{
  MRI *mri_tmp ;

  mri_tmp = MRIdilate(mri_src, NULL) ;
  mri_dst = MRIerode(mri_tmp, mri_dst) ;
  MRIfree(&mri_tmp) ;
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIerode6(MRI *mri_src, MRI *mri_dst)
{
  int     width, height, depth, x, y, z, x1, y1, z1, xi, yi, zi;
  BUFTYPE *pdst, min_val, val ;

  width = mri_src->width ;
  height = mri_src->height ;
  depth = mri_src->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri_src, NULL) ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        xi = mri_src->xi[x] ;
        yi = mri_src->yi[y] ;
        min_val = 255 ;
        for (z1 = -1 ; z1 <= 1 ; z1++)
        {
          zi = mri_src->zi[z+z1] ;
          val = MRIvox(mri_src, xi, yi, zi) ;
          if (val < min_val)
            min_val = val ;
        }
        zi = mri_src->zi[z] ;
        for (y1 = -1 ; y1 <= 1 ; y1++)
        {
          if (!y1)    /* already done */
            continue ;
          yi = mri_src->yi[y+y1] ;
          val = MRIvox(mri_src, xi, yi, zi) ;
          if (val < min_val)
            min_val = val ;
        }
        yi = mri_src->yi[y] ;
        for (x1 = -1 ; x1 <= 1 ; x1++)
        {
          if (!y1)    /* already done */
            continue ;
          xi = mri_src->xi[x+x1] ;
          val = MRIvox(mri_src, xi, yi, zi) ;
          if (val < min_val)
            min_val = val ;
        }
        *pdst++ = min_val ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIdilate6(MRI *mri_src, MRI *mri_dst)
{
  int     width, height, depth, x, y, z, x1, y1, z1, xi, yi, zi;
  BUFTYPE *pdst, max_val, val ;

  width = mri_src->width ;
  height = mri_src->height ;
  depth = mri_src->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri_src, NULL) ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        xi = mri_src->xi[x] ;
        yi = mri_src->yi[y] ;
        max_val = 0 ;
        for (z1 = -1 ; z1 <= 1 ; z1++)
        {
          zi = mri_src->zi[z+z1] ;
          val = MRIvox(mri_src, xi, yi, zi) ;
          if (val > max_val)
            max_val = val ;
        }
        zi = mri_src->zi[z] ;
        for (y1 = -1 ; y1 <= 1 ; y1++)
        {
          if (!y1)    /* already done */
            continue ;
          yi = mri_src->yi[y+y1] ;
          val = MRIvox(mri_src, xi, yi, zi) ;
          if (val > max_val)
            max_val = val ;
        }
        yi = mri_src->yi[y] ;
        for (x1 = -1 ; x1 <= 1 ; x1++)
        {
          if (!y1)    /* already done */
            continue ;
          xi = mri_src->xi[x+x1] ;
          val = MRIvox(mri_src, xi, yi, zi) ;
          if (val > max_val)
            max_val = val ;
        }
        *pdst++ = max_val ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIopen6(MRI *mri_src, MRI *mri_dst)
{
  MRI *mri_tmp ;

  mri_tmp = MRIerode6(mri_src, NULL) ;
  mri_dst = MRIdilate6(mri_tmp, mri_dst) ;
  MRIfree(&mri_tmp) ;
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIclose6(MRI *mri_src, MRI *mri_dst)
{
  MRI *mri_tmp ;

  mri_tmp = MRIdilate6(mri_src, NULL) ;
  mri_dst = MRIerode6(mri_tmp, mri_dst) ;
  MRIfree(&mri_tmp) ;
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIerodeRegion(MRI *mri_src, MRI *mri_dst, int wsize, MRI_REGION *region)
{
  int     width, height, depth, x, y, z, whalf, x0, y0, z0, val,
          xk, yk, zk, xi, yi, zi, min_val ;
  BUFTYPE *pdst ;

  whalf = wsize/2 ;
  width = region->x + region->dx ;
  if (width > mri_src->width)
    width = mri_src->width ;
  height = region->y + region->dy ;
  if (height > mri_src->height)
    height = mri_src->height ;
  depth = region->z + region->dz ;
  if (depth > mri_src->depth)
    depth = mri_src->depth ;
  x0 = region->x ;
  if (x0 < 0)
    x0 = 0 ;
  y0 = region->y ;
  if (y0 < 0)
    y0 = 0 ;
  z0 = region->z ;
  if (z0 < 0)
    z0 = 0 ;

  if (!mri_dst)
  {
    int  w, h, d ;
    
    w = width - region->x ;
    h = height - region->y ;
    d = depth - region->z ;
    mri_dst = MRIalloc(w, h, d, MRI_UCHAR) ;
    MRIcopyHeader(mri_src, mri_dst) ;
    mri_dst->xstart = mri_src->xstart + region->x * mri_src->xsize ;
    mri_dst->ystart = mri_src->ystart + region->y * mri_src->ysize ;
    mri_dst->zstart = mri_src->zstart + region->z * mri_src->zsize ;
    mri_dst->xend = mri_src->xstart + w * mri_src->xsize ;
    mri_dst->yend = mri_src->ystart + h * mri_src->ysize ;
    mri_dst->zend = mri_src->zstart + d * mri_src->zsize ;
  }

  for (z = z0 ; z < depth ; z++)
  {
    for (y = y0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y-y0, z-z0) ;
      for (x = x0 ; x < width ; x++)
      {
        min_val = 255 ;
        for (zk = -whalf ; zk <= whalf ; zk++)
        {
          zi = mri_src->zi[z+zk] ;
          for (yk = -whalf ; yk <= whalf ; yk++)
          {
            yi = mri_src->yi[y+yk] ;
            for (xk = -whalf ; xk <= whalf ; xk++)
            {
              xi = mri_src->xi[x+xk] ;
              val = (int)MRIvox(mri_src, xi, yi, zi) ;
              if (val < min_val)
                min_val = val ;
            }
          }
        }
        *pdst++ = min_val ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIcomputeResidual(MRI *mri1, MRI *mri2, MRI *mri_dst, int t1, int t2)
{
  int     width, height, depth, x, y, z ;
  BUFTYPE *p1, *p2, *pdst, v1, v2, out ;

  if ((mri1->type != MRI_UCHAR) || (mri2->type != MRI_UCHAR))
    ErrorReturn(NULL, 
                (ERROR_UNSUPPORTED, "MRIxor: inputs must be UCHAR")) ;

  width = mri1->width ;
  height = mri1->height ;
  depth = mri1->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri1, NULL) ;
  else if (mri_dst->type != MRI_UCHAR)
    ErrorReturn(NULL, 
                (ERROR_UNSUPPORTED, 
                 "MRIcomputeResidual: destination must be UCHAR")) ;


  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      p1 = &MRIvox(mri1, 0, y, z) ;
      p2 = &MRIvox(mri2, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        v1 = *p1++ ;
        v2 = *p2++ ;
        v1 = ((v1 >= t1) && (v1 <= t2)) ;
        v2 = ((v2 >= t1) && (v2 <= t2)) ;
        if (v1 && !v2)
          out = 255 ;   /* v2 off and v1 on - make it 'positive' */
        else if (!v1 && v2)
          out = 0 ;     /* v2 on and v1 off - make it 'negative' */
        else
          out = 128 ;   /* both on or both off */
        *pdst++ = out ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIminmax(MRI *mri_src, MRI *mri_dst, MRI *mri_dir, int wsize)
{
  int     width, height, depth, x, y, z, x0, y0, z0, xi, yi, zi, whalf, offset;
  BUFTYPE *pdst, max_val, val, *pdir, min_val ;

  whalf = (wsize-1) / 2 ;
  width = mri_src->width ;
  height = mri_src->height ;
  depth = mri_src->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri_src, NULL) ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      pdir = &MRIvox(mri_dir, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        offset = *pdir++ ;
        max_val = 0 ;
        min_val = 255 ;
        if (offset != OFFSET_ZERO) for (z0 = -whalf ; z0 <= whalf ; z0++)
        {
          zi = mri_src->zi[z+z0] ;
          for (y0 = -whalf ; y0 <= whalf ; y0++)
          {
            yi = mri_src->yi[y+y0] ;
            for (x0 = -whalf ; x0 <= whalf ; x0++)
            {
              xi = mri_src->xi[x+x0] ;
              val = MRIvox(mri_src, xi,yi,zi) ;
              if (val > max_val)
                max_val = val ;
              if (val < min_val)
                min_val = val ;
            }
          }
        }
        switch (offset)
        {
        case OFFSET_GRADIENT_DIRECTION:
          val = max_val ;  
          break ;
        case OFFSET_NEGATIVE_GRADIENT_DIRECTION:
          val = min_val ;
          break ;
        default:
        case OFFSET_ZERO:
          val = MRIvox(mri_src, x, y, z) ;
          break ;
        }
        *pdst++ = val ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRIreplaceValues(MRI *mri_src, MRI *mri_dst, BUFTYPE in_val, BUFTYPE out_val)
{
  int     width, height, depth, x, y, z;
  BUFTYPE *pdst, *psrc, val ;

  width = mri_src->width ;
  height = mri_src->height ;
  depth = mri_src->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri_src, NULL) ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      psrc = &MRIvox(mri_src, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        val = *psrc++ ;
        if (val == in_val)
          val = out_val ;
        *pdst++ = val ;
      }
    }
  }
  return(mri_dst) ;
}
/*-----------------------------------------------------
        Parameters:

        Returns value:

        Description
------------------------------------------------------*/
MRI *
MRImask(MRI *mri_src, MRI *mri_mask, MRI *mri_dst,BUFTYPE mask,BUFTYPE out_val)
{
  int     width, height, depth, x, y, z;
  BUFTYPE *pdst, *psrc, *pmask, val, mask_val ;

  width = mri_src->width ;
  height = mri_src->height ;
  depth = mri_src->depth ;

  if (!mri_dst)
    mri_dst = MRIclone(mri_src, NULL) ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      psrc = &MRIvox(mri_src, 0, y, z) ;
      pmask = &MRIvox(mri_mask, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        val = *psrc++ ;
        mask_val = *pmask++ ;
        if (mask_val == mask)
          *pdst++ = out_val ;
        else
          *pdst++ = val ;
      }
    }
  }
  return(mri_dst) ;
}
/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
MRI *
MRImaskThreshold(MRI *mri_src, MRI *mri_mask, MRI *mri_dst, float threshold,
                 int out_label)
{
  BUFTYPE   *pmask, *pdst, *psrc, out_val, mask, in_val ;
  int       width, height, depth, x, y, z, nchanged, noff, non ;
  float     nvox ;

  if (mri_mask->type != MRI_UCHAR)
    ErrorReturn(NULL, (ERROR_UNSUPPORTED, 
                       "MRI3Dthreshold: mask must be MRI_FLOAT")) ;

  if (!mri_dst)
    mri_dst = MRIclone(mri_src, NULL) ;

  width = mri_src->width ; height = mri_src->height ; depth = mri_src->depth ; 
  /* now apply the inverse morph to build an average wm representation
     of the input volume 
     */


  noff = non = nchanged = 0 ;
  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      pmask = &MRIvox(mri_mask, 0, y, z) ;
      psrc = &MRIvox(mri_src, 0, y, z) ;
      pdst = &MRIvox(mri_dst, 0, y, z) ;
      for (x = 0 ; x < width ; x++)
      {
        if (x == 125 && y == 148 && z == 100)
          DiagBreak() ;  /* marked as 255, should be 127 */
        out_val = 0 ;
        mask = *pmask++ ;   /* value from inverse morphed volume */
        in_val = *psrc++ ;
        if (mask < 100-threshold)        /* probably off */
          out_val = 0 ;
        else  if (mask > threshold)      /* probably on */
          out_val = out_label ;
        else                             /* not sure, use original fill val */
          out_val = in_val ;
        if (out_val != in_val)
        {
          if (out_val)
            non++ ;
          else
            noff++ ;
          nchanged++ ;
        }
        *pdst++ = out_val ;
      }
    }
  }

  nvox = (float)(width * height * depth) ;
  fprintf(stderr, "%8d of %8d voxels changed - %2.1f%%.\n",
          nchanged, (int)nvox, 100.0f*(float)nchanged/nvox) ;
  fprintf(stderr, "%8d of %8d voxels off     - %2.1f%%.\n",
          noff, (int)nvox,   100.0f*(float)noff/nvox) ;
  fprintf(stderr, "%8d of %8d voxels on      - %2.1f%%.\n",
          nchanged, (int)nvox, 100.0f*(float)non/nvox) ;
  return(mri_dst) ;
}
/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
int
MRIgrowLabel(MRI *mri, MRI *mri_filled, int in_label, int out_label)
{
  int      x, y, z, width, height, depth, xi, yi, zi, xk, yk, zk, nfilled,
           total_filled, xmin, xmax, ymin, ymax, zmin, zmax ;

  width = mri->width ; height = mri->height ; depth = mri->depth ;
  total_filled = 0 ;

  xmin = width ; ymin = height ; zmin = depth ;
  xmax = ymax = zmax = 0 ;
  if (in_label) for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      for (x = 0 ; x < width ; x++)
      {
        if (MRIvox(mri, x, y, z) == in_label)
        {
          if (x > xmax)
            xmax = x ;
          if (y > ymax)
            ymax = y ;
          if (z > zmax)
            zmax = z ;
          if (x < xmin)
            xmin = x ;
          if (y < ymin)
            ymin = y ;
          if (z < zmin)
            zmin = z ;
        }
      }
    }
  }
  else   /* filling bg - do outside first (hack, but much faster)*/
  {
    /* find bounding box for real data */
    for (z = 0 ; z < depth ; z++)
    {
      for (y = 0 ; y < height ; y++)
      {
        for (x = 0 ; x < width ; x++)
        {
          if (MRIvox(mri, x, y, z))
          {
            if (x > xmax)
              xmax = x ;
            if (y > ymax)
              ymax = y ;
            if (z > zmax)
              zmax = z ;
            if (x < xmin)
              xmin = x ;
            if (y < ymin)
              ymin = y ;
            if (z < zmin)
              zmin = z ;
          }
        }
      }
    }

    /* fill everything outside it */
    for (z = 0 ; z < depth ; z++)
    {
      for (y = 0 ; y < height ; y++)
      {
        for (x = 0 ; x < width ; x++)
        {
          if (z <= zmin || z >= zmax || 
              y <= ymin || y >= ymax || 
              x <= xmin || x >= xmax)
          {
            total_filled++ ;
            MRIvox(mri_filled, x, y, z) = out_label ;
          }
        }
      }
    }
  }

#if 0
  xmin = ymin = zmin = 0 ;
  xmax = width-1 ; ymax = height-1 ; zmax = depth-1 ;
#endif

  do
  {
    nfilled = 0 ;
    for (z = zmin ; z <= zmax ; z++)
    {
      for (y = ymin ; y <= ymax ; y++)
      {
        for (x = xmin ; x <= xmax ; x++)
        {
          if (MRIvox(mri_filled, x, y, z) == out_label)
          {
            for (zk = -1 ; zk <= 1 ; zk++)
            {
              zi = z+zk ;
              if (zi < 0 || zi >= depth)
                continue ;
              for (yk = -1 ; yk <= 1 ; yk++)
              {
                yi = y+yk ;
                if (yi < 0 || yi >= height)
                  continue ;
                for (xk = -1 ; xk <= 1 ; xk++)
                {
                  xi = x+xk ;
                  if (xi < 0 || xi >= width)
                    continue ;
                  if ((MRIvox(mri, xi, yi, zi) == in_label) &&
                      (MRIvox(mri_filled, xi, yi, zi) != out_label))
                  {
                    nfilled++ ;
                    MRIvox(mri_filled, xi, yi, zi) = out_label ;
                  }
                }
              }
            }
          }
        }
      }
    }
    total_filled += nfilled ;
    fprintf(stderr, "%d voxels filled, total = %d.\n", nfilled, total_filled);
  } while (nfilled > 0) ;
  return(NO_ERROR) ;
}
/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
int
MRIturnOnFG(MRI *mri, MRI *mri_fg)
{
  int    x, y, z, width, height, depth ;

  width = mri->width ; height = mri->height ; depth = mri->depth ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      for (x = 0 ; x < width ; x++)
      {
        if (MRIvox(mri_fg, x, y, z) > 0)
          MRIvox(mri, x, y, z) = MRIvox(mri_fg, x, y, z) ;
      }
    }
  }

  /* now remove islands */
  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      for (x = 0 ; x < width ; x++)
      {
        if ((MRIvox(mri_fg, x, y, z) == 0) && (MRIvox(mri,x,y,z) > 0))
          MRIvox(mri, x, y, z) = 0 ;
      }
    }
  }
  return(NO_ERROR) ;
}
static BUFTYPE findLabel(MRI *mri, int x0, int y0, int z0) ;
/*----------------------------------------------------------------------
            Parameters:

           Description:
             turn off all voxels which are set in the bg image 
----------------------------------------------------------------------*/
int
MRIturnOffBG(MRI *mri, MRI *mri_bg)
{
  int    x, y, z, width, height, depth ;

  width = mri->width ; height = mri->height ; depth = mri->depth ;

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      for (x = 0 ; x < width ; x++)
      {
        if (MRIvox(mri_bg, x, y, z) > 0)
          MRIvox(mri, x, y, z) = 0 ;
      }
    }
  }

  for (z = 0 ; z < depth ; z++)
  {
    for (y = 0 ; y < height ; y++)
    {
      for (x = 0 ; x < width ; x++)
      {
        if ((MRIvox(mri_bg, x, y, z) == 0) &&
            (MRIvox(mri, x, y, z) == 0))
          MRIvox(mri, x, y, z) = findLabel(mri, x, y, z) ;
      }
    }
  }
  return(NO_ERROR) ;
}
static BUFTYPE
findLabel(MRI *mri, int x, int y, int z)
{
  int   xi, yi, zi, xk, yk, zk, left_count, right_count, label,
        width, height, depth ;

  width = mri->width ; height = mri->height ; depth = mri->depth ;
  left_count = right_count = 0 ;
  for (zk = -1 ; zk <= 1 ; zk++)
  {
    zi = z + zk ;
    if (zi < 0 || zi >= depth)
      continue ;
    for (yk = -1 ; yk <= 1 ; yk++)
    {
      yi = y + yk ;
      if (yi < 0 || yi >= height)
        continue ;
      for (xk = -1 ; xk <= 1 ; xk++)
      {
        xi = x + xk ;
        if (xi < 0 || xi >= width)
          continue ;
        label = MRIvox(mri, xi, yi, zi) ;
        if (label == MRI_LEFT_HEMISPHERE)
          left_count++ ;
        else if (label == MRI_RIGHT_HEMISPHERE)
          right_count++ ;
      }
    }
  }
  return ((left_count > right_count) ? 
          MRI_LEFT_HEMISPHERE : 
          MRI_RIGHT_HEMISPHERE) ;
}
