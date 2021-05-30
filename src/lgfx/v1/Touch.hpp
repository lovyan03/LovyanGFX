/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct touch_point_t
  {
    std::int32_t x = -1;
    std::int32_t y = -1;
    std::uint16_t id   = 0;
    std::uint16_t size = 0;
  };

//----------------------------------------------------------------------------

  struct ITouch
  {
    struct config_t
    {
      std::uint32_t freq = 1000000;
      std::uint16_t x_min = 0;
      std::uint16_t x_max = 3600;
      std::uint16_t y_min = 0;
      std::uint16_t y_max = 3600;
      bool bus_shared = false;          /// パネルとタッチが同じバスに繋がっている場合true
      std::int16_t pin_int = -1;
      std::uint8_t offset_rotation = 0;
      union
      {
        struct
        {
          std::int8_t spi_host; // ESP32:spi_host_device_t VSPI_HOST or HSPI_HOST
          std::int16_t pin_sclk;
          std::int16_t pin_mosi;
          std::int16_t pin_miso;
          std::int16_t pin_cs = -1;
        };
        struct
        {
          std::int8_t i2c_port; // ESP32:i2c_port_t I2C_NUM_0 or I2C_NUM_1
          std::int16_t pin_scl;
          std::int16_t pin_sda;
          std::int16_t i2c_addr;
        };
      };
    };

    virtual ~ITouch(void) = default;

    config_t config(void) const { return _cfg; }
    void config(const config_t& config) { _cfg = config; }

    inline bool isSPI(void) const { return _cfg.pin_cs >= 0; }

    virtual bool init(void) = 0;
    virtual void wakeup(void) = 0;
    virtual void sleep(void) = 0;
    virtual bool isEnable(void) { return true; };
    virtual std::uint_fast8_t getTouchRaw(touch_point_t* tp, std::uint_fast8_t number) = 0;
/*
    void calibrate(std::int32_t w, std::int32_t h)
    {
      std::uint16_t parameters[8] =
        { _cfg.x_min, _cfg.y_min
        , _cfg.x_min, _cfg.y_max
        , _cfg.x_max, _cfg.y_min
        , _cfg.x_max, _cfg.y_max };
      setCalibrate(parameters, w, h);
    }

    std::uint_fast8_t getTouch(touch_point_t* tp, std::uint_fast8_t number)
    {
      auto res = getTouchRaw(tp, number);
      std::int32_t tx = (_affine[0] * (float)tp->x + _affine[1] * (float)tp->y) + _affine[2];
      std::int32_t ty = (_affine[3] * (float)tp->x + _affine[4] * (float)tp->y) + _affine[5];
      tp->x = tx;
      tp->y = ty;
      return res;
    }

    void setCalibrateAffine(float affine[6])
    {
      memcpy(_affine, affine, sizeof(float) * 6);
    }

    void setCalibrate(std::uint16_t *parameters, std::int32_t w, std::int32_t h)
    {
      std::uint32_t vect[6] = {0,0,0,0,0,0};
      float mat[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
      float a;
      --w;
      --h;
      for ( int i = 0; i < 4; ++i ) {
        std::int32_t tx = w * ((i>>1) & 1);
        std::int32_t ty = h * ( i     & 1);
        std::int32_t px = parameters[i*2  ];
        std::int32_t py = parameters[i*2+1];
        a = px * px;
        mat[0][0] += a;
        a = px * py;
        mat[0][1] += a;
        mat[1][0] += a;
        a = px;
        mat[0][2] += a;
        mat[2][0] += a;
        a = py * py;
        mat[1][1] += a;
        a = py;
        mat[1][2] += a;
        mat[2][1] += a;
        mat[2][2] += 1;

        vect[0] += px * tx;
        vect[1] += py * tx;
        vect[2] +=      tx;
        vect[3] += px * ty;
        vect[4] += py * ty;
        vect[5] +=      ty;
      }

      {
        float det = 1;
        for ( int k = 0; k < 3; ++k )
        {
          float t = mat[k][k];
          det *= t;
          for ( int i = 0; i < 3; ++i ) mat[k][i] /= t;

          mat[k][k] = 1 / t;
          for ( int j = 0; j < 3; ++j )
          {
            if ( j == k ) continue;

            float u = mat[j][k];

            for ( int i = 0; i < 3; ++i )
            {
              if ( i != k ) mat[j][i] -= mat[k][i] * u;
              else mat[j][i] = -u / t;
            }
          }
        }
      }

      float v0 = vect[0];
      float v1 = vect[1];
      float v2 = vect[2];
      _affine[0] = mat[0][0] * v0 + mat[0][1] * v1 + mat[0][2] * v2;
      _affine[1] = mat[1][0] * v0 + mat[1][1] * v1 + mat[1][2] * v2;
      _affine[2] = mat[2][0] * v0 + mat[2][1] * v1 + mat[2][2] * v2;
      float v3 = vect[3];
      float v4 = vect[4];
      float v5 = vect[5];
      _affine[3] = mat[0][0] * v3 + mat[0][1] * v4 + mat[0][2] * v5;
      _affine[4] = mat[1][0] * v3 + mat[1][1] * v4 + mat[1][2] * v5;
      _affine[5] = mat[2][0] * v3 + mat[2][1] * v4 + mat[2][2] * v5;
    }
*/
  protected:
    config_t _cfg;
    bool _inited = false;
//    float _affine[6] = {1,0,0,0,1,0};
  };
/*
  struct Touch_NULL : public ITouch
  {
    bool init(void) override { return false; }
    void wakeup(void) override {}
    void sleep(void) override {}
    bool isEnable(void) override { return false; };
    std::uint_fast8_t getTouchRaw(touch_point_t *tp, std::uint_fast8_t number) override { return 0; }
  };
//*/
//----------------------------------------------------------------------------
 }
}
