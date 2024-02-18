#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>

LGFX tft;

/*\
 *
 *  ⚠️ Some of those features use a lot of maths, don't expect FTL performances :-)
 *
 *  Features:
 *
 *  - Antialiased thick lines, filled with plain color or gradient
 *  - Antialiased circles, filled with plain color or gradient
 *  - Rectangular fill with linear or radial gradient
 *
 *  LGFX Methods:
 *
 *    NOTE: COLOR can be any of uint8_t, uint16_t, uint32_t, RGBColor, rgb888_t, rgb565_t, rgb888_t, etc
 *
 *  - Draw thick+smooth lines:
 *      void drawWideLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, float thick, COLOR color)
 *
 *  - Draw thick+smooth lines (gradient fill):
 *      void drawWideLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, float thick, const colors_t gradient)
 *
 *  - Draw wedge line:
 *      void drawWedgeLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, float thick0, float thick1, COLOR color)
 *
 *  - Draw wedge line (gradient fill):
 *      void drawWedgeLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, float thick0, float thick1, const colors_t gradient)
 *
 *  - Fill smooth circle with single color (redundant with fillSmoothCircle(), kept for benchmark):
 *      void drawSpot(int32_t x, int32_t y, float radius, COLOR fg_color)
 *
 *  - Fill smooth circle, like fillSmoothCircle() but with gradient fill:
 *      void drawGradientSpot(int32_t x, int32_t y, float radius, const colors_t gradient)
 *
 *  - Draw antialiased line:
 *      void drawSmoothLine( LovyanGFX* gfx, int32_t x0, int32_t y0, int32_t x1, int32_t y1, COLOR color )
 *
 *  - Draw gradient line (not smoothed, used for filling):
 *      void drawGradientLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const colors_t gradient )
 *      void drawGradientHLine(int32_t x, int32_t y, uint32_t w, const colors_t gradient )
 *      void drawGradientVLine(int32_t x, int32_t y, uint32_t h, const colors_t gradient )
 *
 *  - Fill rect with radial/linear gradient (fill_style_t can be any of RADIAL, HLINEAR or VLINEAR):
 *      void fillGradientRect(int32_t x, int32_t y, uint32_t w, uint32_t h, COLOR start, COLOR end,  fill_style_t style=RADIAL )
 *      void fillGradientRect(int32_t x, int32_t y, uint32_t w, uint32_t h, const colors_t gradient, fill_style_t style=RADIAL )
 *
 *  - Create a gradient item from a sized array of colors or from a pointer to an array of colors:
 *      colors_t createGradient( rgb888_t[] colors )
 *      colors_t createGradient( rgb888_t* colors, uint32_t count )
 *
 *  - Example:
 *      lgfx::rgb888_t colors[] = { {0x00,0xff,0x00}, {0xff,0xff,0x00}, {0xff,0x80,0x00}, {0xff,0x00,0x00} };
 *      auto gradient = tft.createGradient( colors ); // create a gradient item
 *      tft.fillGradientRect( 10, 10, 200, 200, gradient, HLINEAR );
 *
 *
\*/

// a couple of colors array for the gradient examples
static constexpr const lgfx::rgb888_t heatMap[]   = { {0, 0xff, 0}, {0xff, 0xff, 0}, {0xff, 0x80, 0}, {0xff, 0, 0}, {0x80, 0, 0} }; // green => yellow => orange => red => dark red
static constexpr const lgfx::rgb888_t greyscale[] = { {0,0,0}, {0xff,0xff,0xff} }; // black => white
// a couple of gradient examples based on colors arrays
static constexpr const lgfx::colors_t heatMapGradient   = { heatMap,   sizeof(heatMap)/sizeof(lgfx::rgb888_t) };
static constexpr const lgfx::colors_t greyScaleGradient = { greyscale, sizeof(greyscale)/sizeof(lgfx::rgb888_t) };
// for degrees to radians conversions
static constexpr double deg2rad  = M_PI/180.0;


// some constant values based on display width/height
struct constValues_t
{
  const uint32_t middleX = (tft.width()/2)-1;
  const uint32_t middleY = (tft.height()/2)-1;
  const uint32_t r1 = std::min(tft.width(),tft.height()) *.39;
  const uint32_t r2 = r1/5;
  const uint32_t r3 = std::min(tft.width(),tft.height()) *.45;
  const uint32_t r4 = std::min(tft.width(),tft.height()) *.37;
  const uint32_t thick1 = std::min(tft.width(),tft.height())/35;
  const uint32_t thick2 = std::min(tft.width(),tft.height())/75;
  const uint32_t thick3 = std::min(tft.width(),tft.height())/150;
  const uint32_t bw0 = std::max(tft.width(),tft.height())/4;
  const uint32_t bw1 = std::max(tft.width(),tft.height())/8;
  const uint32_t max_thickness = std::min(tft.width(),tft.height())/25;
  const lgfx::rgb888_t colors888[4] = { {0x00,0xff,0x00}, {0xff,0xff,0x00}, {0xff,0x80,0x00}, {0xff,0x00,0x00} };
  const lgfx::colors_t gradient888 = tft.createGradient( colors888 );
};


// some random values based on display width/height
struct randomValues_t
{
  constValues_t constValues;
  const uint32_t x0 = random()%tft.width();
  const uint32_t y0 = random()%tft.height();
  const uint32_t x1 = random()%tft.width();
  const uint32_t y1 = random()%tft.height();
  const uint32_t thickness0 = (random()%constValues.max_thickness)+2;
  const uint32_t thickness1 = (random()%constValues.max_thickness)+2;
  const RGBColor colorstart = RGBColor( uint8_t(rand() % 0xff), uint8_t(rand() % 0xff), uint8_t(rand() % 0xff) );
  const RGBColor colorend   = RGBColor( uint8_t(rand() % 0xff), uint8_t(rand() % 0xff), uint8_t(rand() % 0xff) );
  const lgfx::rgb888_t color24start = rand()%0x1000000u;
  const lgfx::rgb888_t color24end   = rand()%0x1000000u;
  const lgfx::rgb565_t color16start = rand()%0x10000;
  const lgfx::rgb565_t color16end   = rand()%0x10000;
  const lgfx::rgb332_t color8start  = rand()%256;
  const lgfx::rgb332_t color8end    = rand()%256;
  const lgfx::rgb888_t colors888[2] = { color24start, color24end };
  const lgfx::colors_t gradient888 = tft.createGradient( colors888 );
};



void drawGrid()
{
  randomValues_t v;
  auto cv = v.constValues;

  float theta = ( M_PI / 180.0f ) * 22.5; // angle
  float sin0 = sinf(theta);
  float cos0 = cosf(theta);
  float sin1 = sinf(M_PI-theta);
  float cos1 = cosf(M_PI-theta);

  for( int32_t y=-tft.height()*2; y<tft.height()*2; y+=8 ) {
    int32_t x0Pos = -tft.width();
    int32_t y0Pos = tft.height()-y;
    int32_t x1Pos = tft.width();
    int32_t y1Pos = tft.height()-y;
    float x0 = (x0Pos * cos0 - y0Pos * sin0)+cv.middleX;
    float y0 = (y0Pos * cos0 + x0Pos * sin0)+cv.middleY;
    float x1 = (x1Pos * cos0 - y1Pos * sin0)+cv.middleX;
    float y1 = (y1Pos * cos0 + x1Pos * sin0)+cv.middleY;
    tft.drawWideLine( x0, y0, x1, y1, 0.5f, heatMapGradient ); // gradient antialiased
  }

  for( int32_t x=-tft.width()*2; x<tft.width()*2; x+=8 ) {
    int32_t x0Pos = tft.width()-x;
    int32_t y0Pos = -tft.height();
    int32_t x1Pos = tft.width()-x;
    int32_t y1Pos = tft.height();
    float x0 = (x0Pos * cos0 - y0Pos * sin0)+cv.middleX;
    float y0 = (y0Pos * cos0 + x0Pos * sin0)+cv.middleY;
    float x1 = (x1Pos * cos0 - y1Pos * sin0)+cv.middleX;
    float y1 = (y1Pos * cos0 + x1Pos * sin0)+cv.middleY;
    tft.drawWideLine( x0, y0, x1, y1, 0.5f, heatMapGradient ); // gradient antialiased
  }

  for( int32_t y=-tft.height()*2; y<tft.height()*2; y+=20 ) {
    uint32_t color = y%100==0? 0xaaaaaau : 0xccccccu;
    int32_t x0Pos = -tft.width();
    int32_t y0Pos = tft.height()-y;
    int32_t x1Pos = tft.width();
    int32_t y1Pos = tft.height()-y;
    float x0 = (x0Pos * cos1 - y0Pos * sin1)+cv.middleX;
    float y0 = (y0Pos * cos1 + x0Pos * sin1)+cv.middleY;
    float x1 = (x1Pos * cos1 - y1Pos * sin1)+cv.middleX;
    float y1 = (y1Pos * cos1 + x1Pos * sin1)+cv.middleY;
    y%100==0
     ? tft.drawWideLine( x0, y0, x1, y1, 1.0f, color ) // slow but with thickness
     : tft.drawSmoothLine( x0, y0, x1, y1, color ) // just antialiased
    ;
  }

  for( int32_t x=-tft.width()*2; x<tft.width()*2; x+=20 ) {
    uint32_t color = x%100==0? 0xaaaaaau : 0xccccccu;
    int32_t x0Pos = tft.width()-x;
    int32_t y0Pos = -tft.height();
    int32_t x1Pos = tft.width()-x;
    int32_t y1Pos = tft.height();
    float x0 = (x0Pos * cos1 - y0Pos * sin1)+cv.middleX;
    float y0 = (y0Pos * cos1 + x0Pos * sin1)+cv.middleY;
    float x1 = (x1Pos * cos1 - y1Pos * sin1)+cv.middleX;
    float y1 = (y1Pos * cos1 + x1Pos * sin1)+cv.middleY;
    x%100==0
     ? tft.drawWideLine( x0, y0, x1, y1, 1.0f, color ) // slow but with thickness
     : tft.drawSmoothLine( x0, y0, x1, y1, color ) // just antialiased
    ;
  }
}


void runRandomLinesDemo()
{
  for( int i=0;i<256;i++) {
    randomValues_t v;
    switch( i++%5 ) {
      case 0: tft.drawWedgeLine( v.x0, v.y0, v.x1, v.y1, v.thickness0, v.thickness1, v.gradient888 );  break;
      case 1: tft.drawWedgeLine( v.x0, v.y0, v.x1, v.y1, v.thickness0, v.thickness1, v.colorstart );   break;
      case 2: tft.drawWedgeLine( v.x0, v.y0, v.x1, v.y1, v.thickness0, v.thickness1, v.color24start ); break;
      case 3: tft.drawWedgeLine( v.x0, v.y0, v.x1, v.y1, v.thickness0, v.thickness1, v.color16start ); break;
      case 4: tft.drawWedgeLine( v.x0, v.y0, v.x1, v.y1, v.thickness0, v.thickness1, v.color8start );  break;
    }
  }
}


void runGradientDemo()
{

  drawGrid();

  randomValues_t v;
  auto cv = v.constValues;

  const lgfx::rgb888_t colors888[] = { {0x00,0xff,0x00}, {0xff,0xff,0x00}, {0xff,0x80,0x00}, {0xff,0x00,0x00} };
  auto gradient888 = tft.createGradient( colors888 );

  // horizontal linear gradient in landscape mode (top left)
  tft.fillGradientRect( cv.thick2, cv.thick2, cv.bw0, cv.bw1, heatMapGradient, lgfx::HLINEAR );
  // vertical linear gradient in portrait mode (top right)
  tft.fillGradientRect( tft.width()-(cv.bw1+cv.thick2+1), cv.thick2, cv.bw1, cv.bw0, heatMapGradient, lgfx::VLINEAR );

  // horizontal linear grayscale gradient (top center)
  tft.fillGradientRect( cv.middleX-(cv.bw0/2), cv.thick2, cv.bw0, cv.bw1/2, 0x000000u, 0xffffffu, lgfx::HLINEAR );
  // horizontal linear inverted grayscale gradient (bottom center)
  tft.fillGradientRect( cv.middleX-(cv.bw0/2), tft.height()-(1+cv.thick2+cv.bw1/2), cv.bw0, cv.bw1/2, 0xffffffu, 0x000000u, lgfx::HLINEAR );

  // radial gradient in landscape mode (bottom left)
  tft.fillGradientRect( cv.thick2, tft.height()-(1+cv.bw1+cv.thick2), cv.bw0, cv.bw1, heatMapGradient, lgfx::RADIAL );
  // radial gradient in portrait mode (bottom right)
  tft.fillGradientRect( tft.width()-(cv.bw1+cv.thick2+1), tft.height()-(1+cv.bw0+cv.thick2), cv.bw1, cv.bw0, heatMapGradient, lgfx::RADIAL );

  // draw a big circle filled with default (rgb888) colored gradient
  tft.drawGradientSpot( cv.middleX, cv.middleY, cv.r4, gradient888 );

  int32_t last_x=-1, last_y=-1;
  lgfx::rgb888_t last_color;

  for( int i=0; i<360; i+=15 ) {
    float a = i*deg2rad;

    float s = sin(a);
    float c = cos(a);

    int32_t x0 = s*cv.r1 + cv.middleX;
    int32_t y0 = c*cv.r1 + cv.middleY;
    int32_t x1 = s*cv.r2 + cv.middleX;
    int32_t y1 = c*cv.r2 + cv.middleY;

    int32_t x2 = s*cv.r3 + cv.middleX;
    int32_t y2 = c*cv.r3 + cv.middleY;

    switch( (i/15)%8 ) {
      case 0: tft.drawWedgeLine( x0, y0, x1, y1, cv.thick1, cv.thick3, heatMapGradient ); break;
      case 1: tft.drawWedgeLine( x0, y0, x1, y1, cv.thick1, cv.thick3, gradient888 ); break;
      case 2: tft.drawWedgeLine( x1, y1, x0, y0, cv.thick3, cv.thick1, gradient888 ); break;
      case 3: tft.drawWedgeLine( x1, y1, x0, y0, cv.thick3, cv.thick1, heatMapGradient ); break;
      case 4: tft.drawWedgeLine( x0, y0, x1, y1, cv.thick2, cv.thick3, 0xff0000u ); break; // red 24bits
      case 5: tft.drawWedgeLine( x0, y0, x1, y1, cv.thick2, cv.thick3, (lgfx::rgb332_t)0x82 ); break; // purple 8bits
      case 6: tft.drawWedgeLine( x0, y0, x1, y1, cv.thick2, cv.thick3, TFT_ORANGE ); break; // named colors default to 16bits rgb565_t
      case 7: tft.drawWedgeLine( x0, y0, x1, y1, cv.thick2, cv.thick3, RGBColor{0x00,0xff,0x00} ); break; // green
    }

    auto _color = tft.mapGradient( i, 0, 359, heatMapGradient );
    lgfx::rgb888_t color( _color.R8(), _color.G8(), _color.B8() );

    if( last_x>=0 && last_y>=0 && (i/15)%2==1 ) {
      lgfx::rgb888_t colorsRGB[] = { last_color, color };
      auto gradientRGB = tft.createGradient( colorsRGB );
      tft.drawWedgeLine( last_x, last_y, x2, y2, cv.thick1, cv.thick3, gradientRGB );
    }
    last_color = color;
    last_x = x2;
    last_y = y2;
  }
}


void setup()
{
  tft.init();
  tft.fillScreen(TFT_WHITE);
}

void loop()
{
  static bool toggler = true;
  toggler = !toggler;
  if( toggler ) {
    runRandomLinesDemo();
    vTaskDelay(1000);
  } else {
    runGradientDemo();
    vTaskDelay(5000);
  }
}

