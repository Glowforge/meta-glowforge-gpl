/*
 * lib4lconvert, video4linux2 format conversion lib
 *             (C) 2008 Hans de Goede <hdegoede@redhat.com>
 *
 * Additions to extract single color channels
 * are (C) 2018 Glowforge, Inc. <opensource@glowforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Note: original bayer_to_bgr24 code from :
 * 1394-Based Digital Camera Control Library
 *
 * Bayer pattern decoding functions
 *
 * Written by Damien Douxchamps and Frederic Devernay
 *
 * Note that the original bayer.c in libdc1394 supports many different
 * bayer decode algorithms, for lib4lconvert the one in this file has been
 * chosen (and optimized a bit) and the other algorithm's have been removed,
 * see bayer.c from libdc1394 for all supported algorithms
 */

/**************************************************************
 *     Color conversion functions for cameras that can        *
 * output raw-Bayer pattern images, such as some Basler and   *
 * Point Grey camera. Most of the algos presented here come   *
 * from http://www-ise.stanford.edu/~tingchen/ and have been  *
 * converted from Matlab to C and extended to all elementary  *
 * patterns.                                                  *
 **************************************************************/

/* insprired by OpenCV's Bayer decoding */
static void v4lconvert_border_bayer_line_to_bgr24(
  const unsigned char* bayer, const unsigned char* adjacent_bayer,
  unsigned char *bgr, int width, int start_with_green, int blue_line)
{
  int t0, t1;

  if (start_with_green) {
    /* First pixel */
    if (blue_line) {
      *bgr++ = bayer[1];
      *bgr++ = bayer[0];
      *bgr++ = adjacent_bayer[0];
    } else {
      *bgr++ = adjacent_bayer[0];
      *bgr++ = bayer[0];
      *bgr++ = bayer[1];
    }
    /* Second pixel */
    t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
    t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
    if (blue_line) {
      *bgr++ = bayer[1];
      *bgr++ = t0;
      *bgr++ = t1;
    } else {
      *bgr++ = t1;
      *bgr++ = t0;
      *bgr++ = bayer[1];
    }
    bayer++;
    adjacent_bayer++;
    width -= 2;
  } else {
    /* First pixel */
    t0 = (bayer[1] + adjacent_bayer[0] + 1) >> 1;
    if (blue_line) {
      *bgr++ = bayer[0];
      *bgr++ = t0;
      *bgr++ = adjacent_bayer[1];
    } else {
      *bgr++ = adjacent_bayer[1];
      *bgr++ = t0;
      *bgr++ = bayer[0];
    }
    width--;
  }

  if (blue_line) {
    for ( ; width > 2; width -= 2) {
      t0 = (bayer[0] + bayer[2] + 1) >> 1;
      *bgr++ = t0;
      *bgr++ = bayer[1];
      *bgr++ = adjacent_bayer[1];
      bayer++;
      adjacent_bayer++;

      t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
      t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
      *bgr++ = bayer[1];
      *bgr++ = t0;
      *bgr++ = t1;
      bayer++;
      adjacent_bayer++;
    }
  } else {
    for ( ; width > 2; width -= 2) {
      t0 = (bayer[0] + bayer[2] + 1) >> 1;
      *bgr++ = adjacent_bayer[1];
      *bgr++ = bayer[1];
      *bgr++ = t0;
      bayer++;
      adjacent_bayer++;

      t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
      t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
      *bgr++ = t1;
      *bgr++ = t0;
      *bgr++ = bayer[1];
      bayer++;
      adjacent_bayer++;
    }
  }

  if (width == 2) {
    /* Second to last pixel */
    t0 = (bayer[0] + bayer[2] + 1) >> 1;
    if (blue_line) {
      *bgr++ = t0;
      *bgr++ = bayer[1];
      *bgr++ = adjacent_bayer[1];
    } else {
      *bgr++ = adjacent_bayer[1];
      *bgr++ = bayer[1];
      *bgr++ = t0;
    }
    /* Last pixel */
    t0 = (bayer[1] + adjacent_bayer[2] + 1) >> 1;
    if (blue_line) {
      *bgr++ = bayer[2];
      *bgr++ = t0;
      *bgr++ = adjacent_bayer[1];
    } else {
      *bgr++ = adjacent_bayer[1];
      *bgr++ = t0;
      *bgr++ = bayer[2];
    }
  } else {
    /* Last pixel */
    if (blue_line) {
      *bgr++ = bayer[0];
      *bgr++ = bayer[1];
      *bgr++ = adjacent_bayer[1];
    } else {
      *bgr++ = adjacent_bayer[1];
      *bgr++ = bayer[1];
      *bgr++ = bayer[0];
    }
  }
}

/* From libdc1394, which on turn was based on OpenCV's Bayer decoding */
void v4lconvert_bayer_to_rgbbgr24(const unsigned char *bayer,
  unsigned char *bgr, int width, int height, unsigned int pixfmt,
  int start_with_green, int blue_line)
{
  /* render the first line */
  v4lconvert_border_bayer_line_to_bgr24(bayer, bayer + width, bgr, width,
    start_with_green, blue_line);
  bgr += width * 3;

  /* reduce height by 2 because of the special case top/bottom line */
  for (height -= 2; height; height--) {
    int t0, t1;
    /* (width - 2) because of the border */
    const unsigned char *bayerEnd = bayer + (width - 2);
    if (start_with_green) {
      /* OpenCV has a bug in the next line, which was
        t0 = (bayer[0] + bayer[width * 2] + 1) >> 1; */
      t0 = (bayer[1] + bayer[width * 2 + 1] + 1) >> 1;
      /* Write first pixel */
      t1 = (bayer[0] + bayer[width * 2] + bayer[width + 1] + 1) / 3;
      if (blue_line) {
        *bgr++ = t0;
        *bgr++ = t1;
        *bgr++ = bayer[width];
      } else {
        *bgr++ = bayer[width];
        *bgr++ = t1;
        *bgr++ = t0;
      }

      /* Write second pixel */
      t1 = (bayer[width] + bayer[width + 2] + 1) >> 1;
      if (blue_line) {
        *bgr++ = t0;
        *bgr++ = bayer[width + 1];
        *bgr++ = t1;
      } else {
        *bgr++ = t1;
        *bgr++ = bayer[width + 1];
        *bgr++ = t0;
      }
      bayer++;
    } else {
      /* Write first pixel */
      t0 = (bayer[0] + bayer[width * 2] + 1) >> 1;
      if (blue_line) {
        *bgr++ = t0;
        *bgr++ = bayer[width];
        *bgr++ = bayer[width + 1];
      } else {
        *bgr++ = bayer[width + 1];
        *bgr++ = bayer[width];
        *bgr++ = t0;
      }
    }

    if (blue_line) {
      for (; bayer <= bayerEnd - 2; bayer += 2) {
        t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
          bayer[width * 2 + 2] + 2) >> 2;
        t1 = (bayer[1] + bayer[width] +
          bayer[width + 2] + bayer[width * 2 + 1] +
          2) >> 2;
        *bgr++ = t0;
        *bgr++ = t1;
        *bgr++ = bayer[width + 1];

        t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
        t1 = (bayer[width + 1] + bayer[width + 3] +
          1) >> 1;
        *bgr++ = t0;
        *bgr++ = bayer[width + 2];
        *bgr++ = t1;
      }
    } else {
      for (; bayer <= bayerEnd - 2; bayer += 2) {
        t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
          bayer[width * 2 + 2] + 2) >> 2;
        t1 = (bayer[1] + bayer[width] +
          bayer[width + 2] + bayer[width * 2 + 1] +
          2) >> 2;
        *bgr++ = bayer[width + 1];
        *bgr++ = t1;
        *bgr++ = t0;

        t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
        t1 = (bayer[width + 1] + bayer[width + 3] +
          1) >> 1;
        *bgr++ = t1;
        *bgr++ = bayer[width + 2];
        *bgr++ = t0;
      }
    }

    if (bayer < bayerEnd) {
      /* write second to last pixel */
      t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
        bayer[width * 2 + 2] + 2) >> 2;
      t1 = (bayer[1] + bayer[width] +
        bayer[width + 2] + bayer[width * 2 + 1] +
        2) >> 2;
      if (blue_line) {
        *bgr++ = t0;
        *bgr++ = t1;
        *bgr++ = bayer[width + 1];
      } else {
        *bgr++ = bayer[width + 1];
        *bgr++ = t1;
        *bgr++ = t0;
      }
      /* write last pixel */
      t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
      if (blue_line) {
        *bgr++ = t0;
        *bgr++ = bayer[width + 2];
        *bgr++ = bayer[width + 1];
      } else {
        *bgr++ = bayer[width + 1];
        *bgr++ = bayer[width + 2];
        *bgr++ = t0;
      }
      bayer++;
    } else {
      /* write last pixel */
      t0 = (bayer[0] + bayer[width * 2] + 1) >> 1;
      t1 = (bayer[1] + bayer[width * 2 + 1] + bayer[width] + 1) / 3;
      if (blue_line) {
        *bgr++ = t0;
        *bgr++ = t1;
        *bgr++ = bayer[width + 1];
      } else {
        *bgr++ = bayer[width + 1];
        *bgr++ = t1;
        *bgr++ = t0;
      }
    }

    /* skip 2 border pixels */
    bayer += 2;

    blue_line = !blue_line;
    start_with_green = !start_with_green;
  }

  /* render the last line */
  v4lconvert_border_bayer_line_to_bgr24(bayer + width, bayer, bgr, width,
    !start_with_green, !blue_line);
}



/* ---------- Extract red channel only ---------- */
static void v4lconvert_border_bayer_line_to_r8(
  const unsigned char* bayer, const unsigned char* adjacent_bayer,
  unsigned char *red, int width, int start_with_green, int blue_line)
{
  int t0, t1;

  if (start_with_green) {
    /* First pixel */
    if (blue_line) {
      *red++ = bayer[1];
    } else {
      *red++ = adjacent_bayer[0];
    }
    /* Second pixel */
    t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
    t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
    if (blue_line) {
      *red++ = bayer[1];
    } else {
      *red++ = t1;
    }
    bayer++;
    adjacent_bayer++;
    width -= 2;
  } else {
    /* First pixel */
    t0 = (bayer[1] + adjacent_bayer[0] + 1) >> 1;
    if (blue_line) {
      *red++ = bayer[0];
    } else {
      *red++ = adjacent_bayer[1];
    }
    width--;
  }

  if (blue_line) {
    for ( ; width > 2; width -= 2) {
      t0 = (bayer[0] + bayer[2] + 1) >> 1;
      *red++ = t0;
      bayer++;
      adjacent_bayer++;

      t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
      t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
      *red++ = bayer[1];
      bayer++;
      adjacent_bayer++;
    }
  } else {
    for ( ; width > 2; width -= 2) {
      t0 = (bayer[0] + bayer[2] + 1) >> 1;
      *red++ = adjacent_bayer[1];
      bayer++;
      adjacent_bayer++;

      t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
      t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
      *red++ = t1;
      bayer++;
      adjacent_bayer++;
    }
  }

  if (width == 2) {
    /* Second to last pixel */
    t0 = (bayer[0] + bayer[2] + 1) >> 1;
    if (blue_line) {
      *red++ = t0;
    } else {
      *red++ = adjacent_bayer[1];
    }
    /* Last pixel */
    t0 = (bayer[1] + adjacent_bayer[2] + 1) >> 1;
    if (blue_line) {
      *red++ = bayer[2];
    } else {
      *red++ = adjacent_bayer[1];
    }
  } else {
    /* Last pixel */
    if (blue_line) {
      *red++ = bayer[0];
    } else {
      *red++ = adjacent_bayer[1];
    }
  }
}


void v4lconvert_bayer_to_r8(const unsigned char *bayer,
  unsigned char *red, int width, int height, unsigned int pixfmt,
  int start_with_green, int blue_line)
{
  /* render the first line */
  v4lconvert_border_bayer_line_to_r8(bayer, bayer + width, red, width,
    start_with_green, blue_line);
  red += width;

  /* reduce height by 2 because of the special case top/bottom line */
  for (height -= 2; height; height--) {
    int t0, t1;
    /* (width - 2) because of the border */
    const unsigned char *bayerEnd = bayer + (width - 2);
    if (start_with_green) {
      /* OpenCV has a bug in the next line, which was
        t0 = (bayer[0] + bayer[width * 2] + 1) >> 1; */
      t0 = (bayer[1] + bayer[width * 2 + 1] + 1) >> 1;
      /* Write first pixel */
      t1 = (bayer[0] + bayer[width * 2] + bayer[width + 1] + 1) / 3;
      if (blue_line) {
        *red++ = t0;
      } else {
        *red++ = bayer[width];
      }

      /* Write second pixel */
      t1 = (bayer[width] + bayer[width + 2] + 1) >> 1;
      if (blue_line) {
        *red++ = t0;
      } else {
        *red++ = t1;
      }
      bayer++;
    } else {
      /* Write first pixel */
      t0 = (bayer[0] + bayer[width * 2] + 1) >> 1;
      if (blue_line) {
        *red++ = t0;
      } else {
        *red++ = bayer[width + 1];
      }
    }

    if (blue_line) {
      for (; bayer <= bayerEnd - 2; bayer += 2) {
        t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
          bayer[width * 2 + 2] + 2) >> 2;
        t1 = (bayer[1] + bayer[width] +
          bayer[width + 2] + bayer[width * 2 + 1] +
          2) >> 2;
        *red++ = t0;

        t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
        t1 = (bayer[width + 1] + bayer[width + 3] +
          1) >> 1;
        *red++ = t0;
      }
    } else {
      for (; bayer <= bayerEnd - 2; bayer += 2) {
        t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
          bayer[width * 2 + 2] + 2) >> 2;
        t1 = (bayer[1] + bayer[width] +
          bayer[width + 2] + bayer[width * 2 + 1] +
          2) >> 2;
        *red++ = bayer[width + 1];

        t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
        t1 = (bayer[width + 1] + bayer[width + 3] +
          1) >> 1;
        *red++ = t1;
      }
    }

    if (bayer < bayerEnd) {
      /* write second to last pixel */
      t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
        bayer[width * 2 + 2] + 2) >> 2;
      t1 = (bayer[1] + bayer[width] +
        bayer[width + 2] + bayer[width * 2 + 1] +
        2) >> 2;
      if (blue_line) {
        *red++ = t0;
      } else {
        *red++ = bayer[width + 1];
      }
      /* write last pixel */
      t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
      if (blue_line) {
        *red++ = t0;
      } else {
        *red++ = bayer[width + 1];
      }
      bayer++;
    } else {
      /* write last pixel */
      t0 = (bayer[0] + bayer[width * 2] + 1) >> 1;
      t1 = (bayer[1] + bayer[width * 2 + 1] + bayer[width] + 1) / 3;
      if (blue_line) {
        *red++ = t0;
      } else {
        *red++ = bayer[width + 1];
      }
    }

    /* skip 2 border pixels */
    bayer += 2;

    blue_line = !blue_line;
    start_with_green = !start_with_green;
  }

  /* render the last line */
  v4lconvert_border_bayer_line_to_r8(bayer + width, bayer, red, width,
    !start_with_green, !blue_line);
}



/* ---------- Extract blue channel only ---------- */
static void v4lconvert_border_bayer_line_to_b8(
  const unsigned char* bayer, const unsigned char* adjacent_bayer,
  unsigned char *blue, int width, int start_with_green, int blue_line)
{
  int t0, t1;

  if (start_with_green) {
    /* First pixel */
    if (blue_line) {
      *blue++ = adjacent_bayer[0];
    } else {
      *blue++ = bayer[1];
    }
    /* Second pixel */
    t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
    t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
    if (blue_line) {
      *blue++ = t1;
    } else {
      *blue++ = bayer[1];
    }
    bayer++;
    adjacent_bayer++;
    width -= 2;
  } else {
    /* First pixel */
    t0 = (bayer[1] + adjacent_bayer[0] + 1) >> 1;
    if (blue_line) {
      *blue++ = adjacent_bayer[1];
    } else {
      *blue++ = bayer[0];
    }
    width--;
  }

  if (blue_line) {
    for ( ; width > 2; width -= 2) {
      t0 = (bayer[0] + bayer[2] + 1) >> 1;
      *blue++ = adjacent_bayer[1];
      bayer++;
      adjacent_bayer++;

      t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
      t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
      *blue++ = t1;
      bayer++;
      adjacent_bayer++;
    }
  } else {
    for ( ; width > 2; width -= 2) {
      t0 = (bayer[0] + bayer[2] + 1) >> 1;
      *blue++ = t0;
      bayer++;
      adjacent_bayer++;

      t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
      t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
      *blue++ = bayer[1];
      bayer++;
      adjacent_bayer++;
    }
  }

  if (width == 2) {
    /* Second to last pixel */
    t0 = (bayer[0] + bayer[2] + 1) >> 1;
    if (blue_line) {
      *blue++ = adjacent_bayer[1];
    } else {
      *blue++ = t0;
    }
    /* Last pixel */
    t0 = (bayer[1] + adjacent_bayer[2] + 1) >> 1;
    if (blue_line) {
      *blue++ = adjacent_bayer[1];
    } else {
      *blue++ = bayer[2];
    }
  } else {
    /* Last pixel */
    if (blue_line) {
      *blue++ = adjacent_bayer[1];
    } else {
      *blue++ = bayer[0];
    }
  }
}


void v4lconvert_bayer_to_b8(const unsigned char *bayer,
  unsigned char *blue, int width, int height, unsigned int pixfmt,
  int start_with_green, int blue_line)
{
  /* render the first line */
  v4lconvert_border_bayer_line_to_b8(bayer, bayer + width, blue, width,
    start_with_green, blue_line);
  blue += width;

  /* reduce height by 2 because of the special case top/bottom line */
  for (height -= 2; height; height--) {
    int t0, t1;
    /* (width - 2) because of the border */
    const unsigned char *bayerEnd = bayer + (width - 2);
    if (start_with_green) {
      /* OpenCV has a bug in the next line, which was
        t0 = (bayer[0] + bayer[width * 2] + 1) >> 1; */
      t0 = (bayer[1] + bayer[width * 2 + 1] + 1) >> 1;
      /* Write first pixel */
      t1 = (bayer[0] + bayer[width * 2] + bayer[width + 1] + 1) / 3;
      if (blue_line) {
        *blue++ = bayer[width];
      } else {
        *blue++ = t0;
      }

      /* Write second pixel */
      t1 = (bayer[width] + bayer[width + 2] + 1) >> 1;
      if (blue_line) {
        *blue++ = t1;
      } else {
        *blue++ = t0;
      }
      bayer++;
    } else {
      /* Write first pixel */
      t0 = (bayer[0] + bayer[width * 2] + 1) >> 1;
      if (blue_line) {
        *blue++ = bayer[width + 1];
      } else {
        *blue++ = t0;
      }
    }

    if (blue_line) {
      for (; bayer <= bayerEnd - 2; bayer += 2) {
        t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
          bayer[width * 2 + 2] + 2) >> 2;
        t1 = (bayer[1] + bayer[width] +
          bayer[width + 2] + bayer[width * 2 + 1] +
          2) >> 2;
        *blue++ = bayer[width + 1];

        t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
        t1 = (bayer[width + 1] + bayer[width + 3] +
          1) >> 1;
        *blue++ = t1;
      }
    } else {
      for (; bayer <= bayerEnd - 2; bayer += 2) {
        t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
          bayer[width * 2 + 2] + 2) >> 2;
        t1 = (bayer[1] + bayer[width] +
          bayer[width + 2] + bayer[width * 2 + 1] +
          2) >> 2;
        *blue++ = t0;

        t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
        t1 = (bayer[width + 1] + bayer[width + 3] +
          1) >> 1;
        *blue++ = t0;
      }
    }

    if (bayer < bayerEnd) {
      /* write second to last pixel */
      t0 = (bayer[0] + bayer[2] + bayer[width * 2] +
        bayer[width * 2 + 2] + 2) >> 2;
      t1 = (bayer[1] + bayer[width] +
        bayer[width + 2] + bayer[width * 2 + 1] +
        2) >> 2;
      if (blue_line) {
        *blue++ = bayer[width + 1];
      } else {
        *blue++ = t0;
      }
      /* write last pixel */
      t0 = (bayer[2] + bayer[width * 2 + 2] + 1) >> 1;
      if (blue_line) {
        *blue++ = bayer[width + 1];
      } else {
        *blue++ = t0;
      }
      bayer++;
    } else {
      /* write last pixel */
      t0 = (bayer[0] + bayer[width * 2] + 1) >> 1;
      t1 = (bayer[1] + bayer[width * 2 + 1] + bayer[width] + 1) / 3;
      if (blue_line) {
        *blue++ = bayer[width + 1];
      } else {
        *blue++ = t0;
      }
    }

    /* skip 2 border pixels */
    bayer += 2;

    blue_line = !blue_line;
    start_with_green = !start_with_green;
  }

  /* render the last line */
  v4lconvert_border_bayer_line_to_b8(bayer + width, bayer, blue, width,
    !start_with_green, !blue_line);
}



/* ---------- Extract green channel only ---------- */
static void v4lconvert_border_bayer_line_to_g8(
  const unsigned char* bayer, const unsigned char* adjacent_bayer,
  unsigned char *green, int width, int start_with_green)
{
  int t0;

  if (start_with_green) {
    /* First pixel */
    *green++ = bayer[0];
    /* Second pixel */
    t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
    *green++ = t0;
    bayer++;
    adjacent_bayer++;
    width -= 2;
  } else {
    /* First pixel */
    t0 = (bayer[1] + adjacent_bayer[0] + 1) >> 1;
    *green++ = t0;
    width--;
  }

  for ( ; width > 2; width -= 2) {
    t0 = (bayer[0] + bayer[2] + 1) >> 1;
    *green++ = bayer[1];
    bayer++;
    adjacent_bayer++;

    t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
    *green++ = t0;
    bayer++;
    adjacent_bayer++;
  }

  if (width == 2) {
    /* Second to last pixel */
    t0 = (bayer[0] + bayer[2] + 1) >> 1;
    *green++ = bayer[1];
    /* Last pixel */
    t0 = (bayer[1] + adjacent_bayer[2] + 1) >> 1;
    *green++ = t0;
  } else {
    /* Last pixel */
    *green++ = bayer[1];
  }
}


void v4lconvert_bayer_to_g8(const unsigned char *bayer,
  unsigned char *green, int width, int height, unsigned int pixfmt,
  int start_with_green)
{
  /* render the first line */
  v4lconvert_border_bayer_line_to_g8(bayer, bayer + width, green, width,
    start_with_green);
  green += width;

  /* reduce height by 2 because of the special case top/bottom line */
  for (height -= 2; height; height--) {
    int t1;
    /* (width - 2) because of the border */
    const unsigned char *bayerEnd = bayer + (width - 2);
    if (start_with_green) {
      /* Write first pixel */
      t1 = (bayer[0] + bayer[width * 2] + bayer[width + 1] + 1) / 3;
      *green++ = t1;

      /* Write second pixel */
      t1 = (bayer[width] + bayer[width + 2] + 1) >> 1;
      *green++ = bayer[width + 1];
      bayer++;
    } else {
      /* Write first pixel */
      *green++ = bayer[width];
    }

    for (; bayer <= bayerEnd - 2; bayer += 2) {
      t1 = (bayer[1] + bayer[width] +
        bayer[width + 2] + bayer[width * 2 + 1] +
        2) >> 2;
      *green++ = t1;
      *green++ = bayer[width + 2];
    }


    if (bayer < bayerEnd) {
      /* write second to last pixel */
      t1 = (bayer[1] + bayer[width] +
        bayer[width + 2] + bayer[width * 2 + 1] +
        2) >> 2;
      *green++ = t1;
      /* write last pixel */
      *green++ = bayer[width + 2];
      bayer++;
    } else {
      /* write last pixel */
      t1 = (bayer[1] + bayer[width * 2 + 1] + bayer[width] + 1) / 3;
      *green++ = t1;
    }

    /* skip 2 border pixels */
    bayer += 2;

    start_with_green = !start_with_green;
  }

  /* render the last line */
  v4lconvert_border_bayer_line_to_g8(bayer + width, bayer, green, width,
    !start_with_green);
}
