#ifndef LGFX_PANEL_COMMON_HPP_
#define LGFX_PANEL_COMMON_HPP_

#include <stdint.h>
#include <type_traits>

namespace lgfx
{
  static constexpr uint8_t CMD_INIT_DELAY = 0x80;


  #define MEMBER_DETECTOR(member, classname, classname_impl, valuetype) struct classname_impl { \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, T::member> check(decltype(T::member)*); \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, V> check(...); \
  };template<class T, valuetype V> class classname : public decltype(classname_impl::check<T, V>(nullptr)) {};
  MEMBER_DETECTOR(spi_3wire    , get_spi_3wire    , get_spi_3wire_impl    , bool)
  MEMBER_DETECTOR(spi_mode     , get_spi_mode     , get_spi_mode_impl     , uint8_t)
  MEMBER_DETECTOR(spi_mode_read, get_spi_mode_read, get_spi_mode_read_impl, uint8_t)
  MEMBER_DETECTOR(spi_cs       , get_spi_cs       , get_spi_cs_impl       , int)
  MEMBER_DETECTOR(spi_dc       , get_spi_dc       , get_spi_dc_impl       , int)
  MEMBER_DETECTOR(freq_write   , get_freq_write   , get_freq_write_impl   , uint32_t)
  MEMBER_DETECTOR(freq_read    , get_freq_read    , get_freq_read_impl    , uint32_t)
  MEMBER_DETECTOR(freq_fill    , get_freq_fill    , get_freq_fill_impl    , uint32_t)
  MEMBER_DETECTOR(color_depth  , get_color_depth  , get_color_depth_impl  , color_depth_t)
  MEMBER_DETECTOR(invert       , get_invert       , get_invert_impl       , bool)
  MEMBER_DETECTOR(rotation     , get_rotation     , get_rotation_impl     , uint8_t)
  MEMBER_DETECTOR(rgb_order    , get_rgb_order    , get_rgb_order_impl    , bool)
  MEMBER_DETECTOR(panel_x      , get_panel_x      , get_panel_x_impl      , int)
  MEMBER_DETECTOR(panel_y      , get_panel_y      , get_panel_y_impl      , int)
  MEMBER_DETECTOR(ram_width    , get_ram_width    , get_ram_width_impl    , int)
  MEMBER_DETECTOR(ram_height   , get_ram_height   , get_ram_height_impl   , int)
  MEMBER_DETECTOR(panel_width  , get_panel_width  , get_panel_width_impl  , int)
  MEMBER_DETECTOR(panel_height , get_panel_height , get_panel_height_impl , int)
  #undef MEMBER_DETECTOR

  class PanelCommon
  {
  public:

    static uint32_t getWindowAddr32(uint16_t H, uint16_t L) { return ((H)<<8 | (H)>>8) | (((L)<<8 | (L)>>8)<<16 ); }
    static uint32_t getWindowAddr16(uint16_t H, uint16_t L) { return H | L<<8; }

    PanelCommon() {}
    virtual ~PanelCommon() {}

    template <class CFG>
    void setConfig(void)
    {
      spi_3wire = get_spi_3wire<CFG, false>::value;
      spi_mode      = get_spi_mode     <CFG,  0>::value;
      spi_mode_read = get_spi_mode_read<CFG, get_spi_mode<CFG, 0>::value>::value;
      spi_cs        = get_spi_cs       <CFG, -1>::value;
      spi_dc        = get_spi_dc       <CFG, -1>::value;
      freq_write    = get_freq_write   <CFG, 0>::value;
      freq_read     = get_freq_read    <CFG, 0>::value;
      freq_fill     = get_freq_fill    <CFG, 0>::value;

      write_depth   = get_color_depth  <CFG, rgb565_2Byte>::value;
      invert        = get_invert       <CFG, false>::value;
      rotation      = get_rotation     <CFG, 0>::value;

      _rgb_order    = get_rgb_order    <CFG, false>::value;
      _panel_x      = get_panel_x      <CFG, 0>::value;
      _panel_y      = get_panel_y      <CFG, 0>::value;
      _ram_width    = get_ram_width    <CFG, 0>::value;
      _ram_height   = get_ram_height   <CFG, 0>::value;
      _panel_width  = get_panel_width  <CFG, 0>::value;
      _panel_height = get_panel_height <CFG, 0>::value;

      setConfig_impl();
    }

    virtual const uint8_t* getInitCommands(uint8_t listno = 0) const { return nullptr; }

    virtual const uint8_t* getColorDepthCommands(uint8_t* buf, color_depth_t depth) {
      write_depth = getAdjustBpp(depth);
      return buf;
    }

    virtual const uint8_t* getRotationCommands(uint8_t* buf, uint8_t r)
    {
      r = r & 7;
      rotation = r;
      switch (r & 3) {
      default:
        colstart = _panel_x;
        rowstart = _panel_y;
        break;
      case 1:
        colstart = _panel_y;
        rowstart = _ram_width - (_panel_width + _panel_x);
        break;
      case 2:
        colstart = _ram_width  - (_panel_width  + _panel_x);
        rowstart = _ram_height - (_panel_height + _panel_y);
        break;
      case 3:
        colstart = _ram_height - (_panel_height + _panel_y);
        rowstart = _panel_x;
        break;
      }
      if (r & 1) {
        width  = _panel_height;
        height = _panel_width;
      } else {
        width  = _panel_width;
        height = _panel_height;
      }
      return buf;
    }

    uint32_t len_setwindow;
    uint32_t len_dummy_read_pixel;
    uint32_t len_dummy_read_rddid;
    uint32_t cmd_caset;
    uint32_t cmd_raset;
    uint32_t cmd_ramrd;
    uint32_t cmd_ramwr;
    uint32_t cmd_invon;
    uint32_t cmd_invoff;
    uint32_t cmd_rddid;

    uint32_t freq_write;
    uint32_t freq_read;
    uint32_t freq_fill;
    int32_t colstart;
    int32_t rowstart;
    int32_t width;
    int32_t height;
    uint16_t spi_cs;
    uint16_t spi_dc;
    uint8_t spi_mode;
    uint8_t spi_mode_read;
    uint8_t rotation;
    color_depth_t write_depth;
    color_depth_t read_depth;

    bool spi_3wire;
    bool invert;

  protected:
    virtual void setConfig_impl(void) = 0;
    virtual color_depth_t getAdjustBpp(color_depth_t bpp) const { return (bpp > 16) ? rgb888_3Byte : rgb565_2Byte; }

    int32_t _panel_x;
    int32_t _panel_y;
    int32_t _ram_width;
    int32_t _ram_height;
    int32_t _panel_width;
    int32_t _panel_height;
    bool _rgb_order;
  };

//----------------------------------------------------------------------------

}
#endif
