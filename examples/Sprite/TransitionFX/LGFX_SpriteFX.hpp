
#include <LovyanGFX.hpp>

/*\
 *
 * Push your LGFX sprites with style!
 *
 * LGFX_SpriteFX: A LGFX Layer brought to you by @tobozo, copyleft (c+) 2023
 *
\*/


namespace lgfx
{

  namespace easing
  {
   /*\
    * Easing Functions - inspired from :
    *   - http://gizma.com/easing
    *   - https://easings.net/
    * Only considering the t value for the range [0, 1] => [0, 1]
   \*/
    typedef float (*easing_fn_t)(float in);
    // for trigo
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    const float c3 = c1 + 1.0f;
    const float c4 = (2.0f * PI) / 3.0f;
    const float c5 = (2.0f * PI) / 4.5f;
    // for bounce
    const float n1 = 7.5625f;
    const float d1 = 2.75f;

    float linear(float t)           { return t; }
    float easeInQuad(float t)       { return powf(t,2.0f); }
    float easeOutQuad(float t)      { return t*(2.0f-t); }
    float easeInOutQuad(float t)    { return t<.5f ? 2.0f*powf(t,2) : -1.0f+(4.0f-2.0f*t)*t; }
    float easeInCubic(float t)      { return powf(t,3.0f); }
    float easeOutCubic(float t)     { return 1.0f - powf(1.0f - t, 3.0f); }
    float easeInOutCubic(float t)   { return t<.5f ? 4.0f*powf(t,3) : (t-1.0f)*(2.0f*t-2.0f)*(2.0f*t-2.0f)+1.0f; }
    float easeInQuart(float t)      { return powf(t,4.0f); }
    float easeOutQuart(float t)     { return 1.0f - powf(1.0f - t, 4.0f); }
    float easeInOutQuart(float t)   { return t < 0.5 ? 8.0f * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 4.0f) / 2.0f; }
    float easeInQuint(float t)      { return powf(t,5.0f); }
    float easeOutQuint(float t)     { return 1.0f - powf(1.0f - t, 5.0f); }
    float easeInOutQuint(float t)   { return t < 0.5 ? 16.0f * t * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f;/*t<.5f ? 16.0f*powf(t,5) : 1.0f+16.0f*(--t)*powf(t,4);*/ }
    float easeInSine(float t)       { return 1.0f - cosf((t * PI) / 2.0f); }
    float easeOutSine(float t)      { return sinf((t * PI) / 2.0f); }
    float easeInOutSine(float t)    { return -(cosf(PI * t) - 1.0f) / 2.0f; }
    float easeInExpo(float t)       { return t==0 ? 0 : powf(2, 10.0f * t - 10.0f); }
    float easeOutExpo(float t)      { return t==1.0f ? 1.0f : 1.0f - powf(2, -10.0f * t); }
    float easeInOutExpo(float t)    { return t==0 ? 0 : t==1.0f ? 1.0f : t < 0.5f ? powf(2, 20.0f * t - 10.0f) / 2.0f : (2.0f - powf(2, -20.0f * t + 10.0f)) / 2.0f; }
    float easeInCirc(float t)       { return 1.0f - sqrtf(1.0f - powf(t, 2.0f)); }
    float easeOutCirc(float t)      { return sqrtf(1.0f - powf(t - 1, 2.0f)); }
    float easeInOutCirc(float t)    { return t < 0.5f ? (1.0f - sqrtf(1.0f - powf(2.0f * t, 2.0f))) / 2.0f : (sqrtf(1 - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f; }
    float easeInBack(float t)       { return c3 * t * t * t - c1 * t * t; }
    float easeOutBack(float t)      { return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f); }
    float easeInOutBack(float t)    { return t < 0.5f ? (powf(2.0*t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f : (powf(2.0*t-2, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f; }
    float easeInElastic(float t)    { return t==0 ? 0 : t==1.0f ? 1.0f : -powf(2, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * c4); }
    float easeOutElastic(float t)   { return t==0 ? 0 : t==1.0f ? 1.0f : powf(2, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1; }
    float easeInOutElastic(float t) { return t==0 ? 0 : t==1 ? 1 : t<0.5 ? -(powf(2, 20.0f*t-10.0f)*sinf((20.0f*t-11.125f)*c5))/2.0f : (powf(2,-20.0f*t+10.0f)*sinf((20.0f*t-11.125f)*c5))/2.0f+1.0f; }
    float easeOutBounce(float t)    { return t<1.0f/d1 ? n1*t*t : t<2.0f/d1 ? n1*(t-=1.5f/d1)*t+0.75f : t<2.5f/d1 ? n1*(t-=2.25f/d1)*t+0.9375f : n1*(t-=2.625f/d1)*t+0.984375f; }

  }; // end namespace easing


  namespace transition_fx
  {

    enum trans_type_t // all transitions effects supported by this addon are listed here
    {
      PIXEL_STRETCH, // pixel stretch animation, vertical/horizontal both directions
      PIXEL_SLICE,   // scrolled portions
      PIXEL_SPIN,
    };

    struct anim_param_t // abstract item to hold transition pointer, used by pushAnimated()
    {
      const trans_type_t type;
      const void *ptr;
    };


    enum trans_spindir_t
    {
      SPIN_RIGHT,
      SPIN_LEFT
    };

    struct spin_params_t
    {
      const int32_t x;
      const int32_t y;
      const trans_spindir_t dir; // spin direction
      const uint32_t duration_ms;  // easing duration
      const easing::easing_fn_t easingFunc;
    };


    enum trans_axis_t // transition orientation for PIXEL_SLICE
    {
      AXIS_H,
      AXIS_V
    };

    struct slice_params_t // transition parameters for PIXEL_SLICE
    {
      const int32_t x;
      const int32_t y;
      const trans_axis_t dir;   // slicing direction
      const uint32_t slices;      // slices amount
      const uint32_t duration_ms; // easing duration
      const easing::easing_fn_t easingFunc;
    };

    struct sliceRect_t // grouped properties for PIXEL_SLICE
    {
      sliceRect_t() {};
      sliceRect_t(int32_t x_, int32_t y_, int32_t clipx_, int32_t clipy_, int32_t clipw_, int32_t cliph_ ) :
      x(x_), y(y_), clipx(clipx_), clipy(clipy_), clipw(clipw_), cliph(cliph_) { }
      int32_t x{0};
      int32_t y{0};
      int32_t clipx{0};
      int32_t clipy{0};
      int32_t clipw{0};
      int32_t cliph{0};
    };



    enum trans_datum_t // transition direction for PIXEL_STRETCH
    {
      LTR_DATUM, // left to righ
      RTL_DATUM, // right to left
      TTB_DATUM, // top to bottom
      BTT_DATUM  // bottom to top
    };

    struct stretch_params_t // transition parameters for PIXEL_STRETCH
    {
      const int32_t x;
      const int32_t y;
      const trans_datum_t dir; // stretch direction
      const uint32_t delay_ms;   // mininal delay per frame
    };

    struct coords_float_t // x/y coords as float for PIXEL_STRETCH
    {
      template <typename T> coords_float_t(T x_, T y_ ) : x(x_+0.0f), y(y_+0.0f) { }
      float x;
      float y;
    };

    struct pixel_stretch_fx_t // direction and loop controls for PIXEL_STRETCH
    {
      public:
        pixel_stretch_fx_t( coords_float_t pos_, uint32_t w_, uint32_t h_, trans_datum_t direction_=LTR_DATUM, uint32_t delay_ms_=1 )
        : pos(pos_), clipWidth(w_), clipHeight(h_), direction(direction_), delay_ms(delay_ms_)
        {
          swapx = (direction==RTL_DATUM||direction==BTT_DATUM);
          swapy = (direction==TTB_DATUM||direction==BTT_DATUM);

          auto clip1     = !swapy ? clipHeight : clipWidth;
          auto clip2     = !swapy ? clipWidth  : clipHeight;
          auto &_pos     = !swapy ? pos.y      : pos.x;
          auto &_middle  = !swapy ? middle.y   : middle.x;
          auto &_topleft = !swapy ? topleft.y  : topleft.x;
          auto &_scan    = !swapy ? scanheight : scanwidth;

          _middle   = _pos + clip1/2.0f;
          _topleft  = _pos;
          _scan     = clip1;

          loopstart = swapx ? clip2+1.0f : 0;
          loopend   = swapx ? -1.0f : clip2;
          loopdir   = swapx ?-1.0f : 1.0f;
          tick();
          last_ms = anim_start = millis();
        };

        void tick()
        {
          auto &_scan    = !swapy ? scanpos.x : scanpos.y;
          auto &_zoom    = !swapy ? zoom.x    : zoom.y;
          auto &_middle  = !swapy ? middle.x  : middle.y;
          auto &_topleft = !swapy ? topleft.x : topleft.y;
          auto &_pos     = !swapy ? pos.x     : pos.y;
          auto _clip     = !swapx ? !swapy ? clipWidth  : clipHeight : 0;

          _scan    = -loopstart;
          _zoom    = (!swapx ? _clip-loopstart  : loopstart) - 0.5f;
          _middle  = ((!swapx ? _pos+loopstart : _pos) + _zoom/2.0f) - 0.5f;
          _topleft = _pos + loopstart;
        }

        bool next()
        {
          loopstart += loopdir;
          handleTimer();
          tick();
          return loopstart!=loopend;
        }

        void handleTimer()
        {
          uint32_t render_ms_tmp = millis() - last_ms;
          if( delay_ms > 1 ) {
            if( render_ms_tmp < delay_ms ) {
              vTaskDelay( delay_ms - render_ms_tmp );
            }
          }
          last_ms = millis();
        }

        // scanline
        coords_float_t scanpos = { 0, 0 };
        float scanwidth  = 1;
        float scanheight = 1;

        // projection zone
        coords_float_t middle  = { 0, 0 };
        coords_float_t zoom    = { 1, 1 };
        coords_float_t topleft = { 0, 0 };

        // timer helpers
        uint32_t anim_start = 0;
        uint32_t last_ms = 0;
        const float rotate = 0;

      private:

        const coords_float_t pos;
        const float clipWidth;
        const float clipHeight;
        const trans_datum_t direction;
        const uint32_t delay_ms;// 15=66fps

        float loopstart = 0;
        float loopend   = 0;
        float loopdir   = 1;

        bool swapx = false;
        bool swapy = false;
    };

  }; // end namespace transition_fx


  using namespace transition_fx;
  using namespace easing;


  class LGFX_SpriteFx : public LGFX_Sprite
  {
    public:

      LGFX_SpriteFx(LovyanGFX* parent) : LGFX_Sprite(parent) { };

      void spin(  int32_t x=0, int32_t y=0, trans_spindir_t dir=SPIN_RIGHT, uint32_t duration=255, easing_fn_t easing=linear ) { spin_impl({x,y,dir,duration,easing}); }
      void spinC( int32_t x=0, int32_t y=0, uint32_t duration=255, easing_fn_t easing=linear ) { spin_impl({x,y,SPIN_RIGHT,duration,easing}); }
      void spinA( int32_t x=0, int32_t y=0, uint32_t duration=255, easing_fn_t easing=linear ) { spin_impl({x,y,SPIN_LEFT,duration,easing}); }

      void slice(  int32_t x=0, int32_t y=0, trans_axis_t axis=AXIS_V, uint32_t slices=8,
                          uint32_t duration=255, easing_fn_t easing=linear ) { slice_impl({x,y,axis,slices,duration,easing}); }
      void sliceH( int32_t x=0, int32_t y=0, uint8_t slices=8,
                          uint32_t duration=255, easing_fn_t easing=linear ) { slice_impl({x,y,AXIS_H,slices,duration,easing}); }
      void sliceV( int32_t x=0, int32_t y=0, uint8_t slices=8,
                          uint32_t duration=255, easing_fn_t easing=linear ) { slice_impl({x,y,AXIS_V,slices,duration,easing}); }

      void stretch(    int32_t x=0, int32_t y=0, trans_datum_t dir=LTR_DATUM, uint32_t delay_ms=0 ) { stretch_impl({x,y,dir,delay_ms}); }
      void stretchLTR( int32_t x=0, int32_t y=0, uint32_t delay_ms=0 ) { stretch_impl({x,y,LTR_DATUM,delay_ms}); }
      void stretchRTL( int32_t x=0, int32_t y=0, uint32_t delay_ms=0 ) { stretch_impl({x,y,RTL_DATUM,delay_ms}); }
      void stretchTTB( int32_t x=0, int32_t y=0, uint32_t delay_ms=0 ) { stretch_impl({x,y,TTB_DATUM,delay_ms}); }
      void stretchBTT( int32_t x=0, int32_t y=0, uint32_t delay_ms=0 ) { stretch_impl({x,y,BTT_DATUM,delay_ms}); }

      void pushAnimated( anim_param_t* params )
      {
        assert( params );
        assert( params->ptr );
        switch( params->type ) {
          case PIXEL_STRETCH: {
            auto pixel_stretch = (stretch_params_t*) params->ptr;
            stretch_impl( {pixel_stretch->x, pixel_stretch->y, pixel_stretch->dir, pixel_stretch->delay_ms} );
          }
          break;
          case PIXEL_SLICE: {
            auto pixel_slice = (slice_params_t*) params->ptr;
            slice_impl( {pixel_slice->x, pixel_slice->y, pixel_slice->dir, pixel_slice->slices, pixel_slice->duration_ms, pixel_slice->easingFunc} );
          }
          break;
          case PIXEL_SPIN: {
            auto pixel_spin = (spin_params_t*) params->ptr;
            spin_impl( {pixel_spin->x, pixel_spin->y, pixel_spin->dir, pixel_spin->duration_ms, pixel_spin->easingFunc} );
          }
        }
      }

    private:

      void spin_impl( spin_params_t params )
      {
        if( _parent == nullptr ) {
          log_e("Sprite has no parent, can't guess display");
          return;
        }

        auto x          = params.x;
        auto y          = params.y;
        auto dir        = params.dir;
        auto duration   = params.duration_ms;
        auto easingFunc = params.easingFunc;

        const uint32_t timer_start = millis();
        const uint32_t timer_end   = timer_start + duration;
        const uint32_t max_steps   = 1000;

        const float middlex = float(x) + float(this->width()/2.0f);// - float(x/2.0f);
        const float middley = float(y) + float(this->height()/2.0f);// - float(y/2.0f);
        const int anglestart = dir==SPIN_RIGHT ? -360:360;
        const int angleend   = -anglestart;

        uint32_t timer_pos = 0;
        uint32_t timer_now = 0;

        _parent->setClipRect( x, y, this->width(), this->height() );
        do {
          timer_now = millis();
          if( timer_now>timer_end ) timer_now=timer_end;
          timer_pos = map( timer_now, timer_start, timer_end, 0, max_steps );
          float ifloat  = float(timer_pos)/float(max_steps);
          float zoom = easingFunc( ifloat );
          int angle = map( int(zoom*max_steps), 0, max_steps, anglestart, angleend );
          this->pushRotateZoom( _parent, middlex, middley, angle, zoom, zoom );
        } while( timer_now<timer_end );
        _parent->clearClipRect();
      }


      void slice_impl( slice_params_t params )
      {
        if( _parent == nullptr ) {
          log_e("Sprite has no parent, can't guess display");
          return;
        }

        auto x          = params.x;
        auto y          = params.y;
        auto slices     = params.slices;
        auto axis       = params.dir;
        auto duration   = params.duration_ms;
        auto easingFunc = params.easingFunc;

        const uint32_t timer_start = millis();
        const uint32_t timer_end   = timer_start + duration;
        const uint32_t max_steps   = 1000;

        const bool is_vertical = axis==AXIS_V;
        const int32_t scroll_pan_max = is_vertical ? this->width() : this->height();
        const int32_t scroll_pos_max = is_vertical ? this->height() : this->width();

        if( slices < 2 ) slices = 2;
        if( slices > scroll_pan_max/2 ) slices = scroll_pan_max/2;

        const int32_t slice_pad = scroll_pan_max/slices; // slice width or slice height depending axis

        int32_t scroll_pos = 0;
        uint32_t steps = 1000;

        uint32_t timer_pos = 0;
        uint32_t timer_now = 0;

        do {
          timer_now = millis();
          if( timer_now>timer_end ) timer_now=timer_end;
          timer_pos = map( timer_now, timer_start, timer_end, 0, max_steps );
          float ifloat = float(timer_pos)/float(steps);
          float eased = easingFunc( ifloat );
          scroll_pos = map( int(eased*steps), 0, steps, 0, scroll_pos_max );
          const int32_t slice1 = is_vertical ? (y-this->height())+scroll_pos : (x-this->width())+scroll_pos;
          const int32_t slice2 = is_vertical ? (y+this->height())-scroll_pos : (x+this->width())-scroll_pos;
          int32_t slice_num = 0;
          do { // secondary axis (opposite slicing)
            const int32_t offset = slice_num*slice_pad;
            const auto rect = (is_vertical)
              ? sliceRect_t( x, slice_num%2==0 ? slice2 : slice1, x+offset, y, slice_pad,   this->height() )
              : sliceRect_t( slice_num%2==0 ? slice2 : slice1, y, x, y+offset, this->width(), slice_pad    )
            ;
            _parent->setClipRect( rect.clipx, rect.clipy, rect.clipw, rect.cliph );
            this->pushSprite( _parent, rect.x, rect.y );
            slice_num++;
          } while( slice_num <= slices );
        } while( timer_now<timer_end );
        _parent->clearClipRect();
      }


      void stretch_impl( stretch_params_t params )
      {
        if( _parent == nullptr ) {
          log_e("Sprite has no parent, can't guess display");
          return;
        }

        auto x         = params.x;
        auto y         = params.y;
        auto direction = params.dir;
        auto delay_ms  = params.delay_ms;

        pixel_stretch_fx_t fx( {x, y}, this->width(), this->height(), direction, delay_ms );

        LGFX_Sprite *scanline = new LGFX_Sprite();
        scanline->setColorDepth( this->getColorDepth() );
        scanline->setPsram( false ); // use heap for faster frame rate

        if( !scanline->createSprite( fx.scanwidth, fx.scanheight ) ) {
          log_e("Not enough ram to create stretch effect");
          this->pushSprite( _parent, x, y );
          delete scanline;
          return;
        }

        _parent->setClipRect( x, y, this->width(), this->height() );
        do {
          this->pushSprite( scanline, fx.scanpos.x, fx.scanpos.y );
          scanline->pushRotateZoom( _parent, fx.middle.x, fx.middle.y, fx.rotate, fx.zoom.x, fx.zoom.y );
        } while( fx.next() );
        _parent->clearClipRect();

        scanline->deleteSprite();
        delete scanline;
      }

  };


};

using lgfx::LGFX_SpriteFx;

