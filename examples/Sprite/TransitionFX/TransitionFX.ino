/*\
 *
 * Push your LGFX sprites with style!
 *
 * LGFX_SpriteFX: A LGFX Layer brought to you by @tobozo, copyleft (c+) 2023
 *
 * This demo demonstrates the different ways a sprite can
 * be printed to the display using transitions.
 *
 * Transition effects:
 * - Spin: appears in a zommed spin
 * - Slice: sliced and merged
 * - Stretch: spread from one edge
 *
 * Easing function: plenty!
 *
 * Note: "Strech" transition does not support easing, only min delay between frames.
 *
 * Simple usage:
 *
 * // create a sprite using "LGFX_SpriteFx" instead of "LGFX_Sprite" constructor.
 *
 * auto sprite = new LGFX_SpriteFx(&tft);
 *
 * // draw some stuff in the sprite...
 *
 * // when the sprite is ready to be pushed, instead of sprite->pushSprite( x, y ), use one of these these:
 *
 * sprite->spin( x, y );
 * sprite->slice( x, y );
 * sprite->stretch( x, y );
 *
 *
 * Advanced usage:
 *
 * See LGFX_SpriteFX.hpp and this sketch for more advanced examples.
 *
\*/

#define LGFX_AUTODETECT

#include "LGFX_SpriteFX.hpp"
#include "assets.h" // some example images for this demo


struct easingFuncDesc_t
{
  const char* name;
  lgfx::easing::easing_fn_t func;
};


struct img_t
{
  const unsigned char* data;
  const unsigned int len;
  const uint32_t w;
  const uint32_t h;
};


static LGFX tft;
static LGFX_SpriteFx *sptr;

const img_t dog      = img_t { dog_200_200_jpg,       dog_200_200_jpg_len,       200, 200 };
const img_t logo     = img_t { lgfx_logo_201x197_jpg, lgfx_logo_201x197_jpg_len, 201, 197 };
const img_t palette1 = img_t { palette_300x128_jpg,   palette_300x128_jpg_len,   300, 128 };
const img_t palette2 = img_t { palette_128x200_jpg,   palette_128x200_jpg_len,   128, 200 };


const img_t *images[] = { &palette1, &palette2, &dog, &logo };

const easingFuncDesc_t easingFunctions[] =
{
  { "easeOutBounce",    lgfx::easing::easeOutBounce    },
  { "easeOutElastic",   lgfx::easing::easeOutElastic   },
  // { "easeInElastic",    lgfx::easing::easeInElastic    },
  // { "easeInOutElastic", lgfx::easing::easeInOutElastic },
  // { "easeInQuad",       lgfx::easing::easeInQuad       },
  // { "easeOutQuad",      lgfx::easing::easeOutQuad      },
  // { "easeInOutQuad",    lgfx::easing::easeInOutQuad    },
  // { "easeInCubic",      lgfx::easing::easeInCubic      },
  // { "easeOutCubic",     lgfx::easing::easeOutCubic     },
  // { "easeInOutCubic",   lgfx::easing::easeInOutCubic   },
  // { "easeInQuart",      lgfx::easing::easeInQuart      },
  // { "easeOutQuart",     lgfx::easing::easeOutQuart     },
  // { "easeInOutQuart",   lgfx::easing::easeInOutQuart   },
  // { "easeInQuint",      lgfx::easing::easeInQuint      },
  // { "easeOutQuint",     lgfx::easing::easeOutQuint     },
  // { "easeInOutQuint",   lgfx::easing::easeInOutQuint   },
  // { "easeInSine",       lgfx::easing::easeInSine       },
  // { "easeOutSine",      lgfx::easing::easeOutSine      },
  // { "easeInOutSine",    lgfx::easing::easeInOutSine    },
  // { "easeInExpo",       lgfx::easing::easeInExpo       },
  // { "easeOutExpo",      lgfx::easing::easeOutExpo      },
  // { "easeInOutExpo",    lgfx::easing::easeInOutExpo    },
  // { "easeInCirc",       lgfx::easing::easeInCirc       },
  // { "easeOutCirc",      lgfx::easing::easeOutCirc      },
  // { "easeInOutCirc",    lgfx::easing::easeInOutCirc    },
  // { "easeInBack",       lgfx::easing::easeInBack       },
  // { "easeOutBack",      lgfx::easing::easeOutBack      },
  // { "easeInOutBack",    lgfx::easing::easeInOutBack    },
  // { "linear",           lgfx::easing::linear           },
};


void clearZone( int x, int y, int w, int h, uint32_t delay_ms )
{
  delay( delay_ms );
  tft.fillRect( x, y, w, h, TFT_WHITE );
}



void setup()
{
  Serial.begin( 115200 );
  tft.init();

  tft.setTextDatum( TL_DATUM );
  tft.setTextPadding( tft.width() );
  tft.setTextColor( TFT_WHITE, TFT_BLACK );
  tft.setTextSize(2);
  tft.setTextWrap( false );

  sptr = new LGFX_SpriteFx(&tft);
  sptr->setPsram( false );
}


void loop()
{
  static uint32_t imgidx = 0;
  static uint32_t imgcount = sizeof(images)/sizeof(img_t*);
  static uint32_t min_delay_ms = 8;

  auto imgmod = imgidx%imgcount;
  min_delay_ms = (imgmod==0) ? 8-min_delay_ms : min_delay_ms;

  auto img  = images[imgmod];
  auto posx = tft.width()/2  - img->w/2;
  auto posy = tft.height()/2 - img->h/2;

  if( ! sptr->createSprite( img->w, img->h ) ) return;

  // highlight zone
  tft.fillRect( posx,   posy,   sptr->width(),   sptr->height(),   TFT_WHITE );
  tft.drawRect( posx-1, posy-1, sptr->width()+2, sptr->height()+2, TFT_RED );

  // fill sprite with image
  sptr->drawJpg( img->data, img->len, 0, 0, img->w, img->h );

  sptr->stretchRTL( posx, posy, min_delay_ms );  // clearZone( posx, posy, img->w, img->h, 500 );
  sptr->stretchBTT( posx, posy, min_delay_ms );  // clearZone( posx, posy, img->w, img->h, 500 );
  sptr->stretchLTR( posx, posy, min_delay_ms );  // clearZone( posx, posy, img->w, img->h, 500 );
  sptr->stretchTTB( posx, posy, min_delay_ms );  // clearZone( posx, posy, img->w, img->h, 500 );

  size_t easing_functions_count = sizeof(easingFunctions)/sizeof(easingFuncDesc_t);

  for( int i=0;i<easing_functions_count;i++ ) {

    auto easingFunc = easingFunctions[i].func;
    auto funcName   = easingFunctions[i].name;

    Serial.printf("Easing: %s\n", funcName );
    tft.setCursor( 0, 0 );
    tft.printf("Easing: %-32s", funcName );


    sptr->spinC(  posx, posy, 1000, easingFunc );       // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->spinA(  posx, posy, 1000, easingFunc );       // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 0, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 0, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 4, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 4, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 8, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 8, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 16, 1000, easingFunc );  // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 16, 1000, easingFunc );  // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 255, 1000, easingFunc ); // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 255, 1000, easingFunc ); // clearZone( posx, posy, img->w, img->h, 500 );

  }

  tft.setCursor( 0, 0 );
  tft.printf("%64s", "" );

  // clear zone
  tft.fillRect( posx-1, posy-1, img->w+2, img->h+2, TFT_BLACK );

  imgidx++;
}
