#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_freertos_hooks.h"
#include "hal/spi_types.h"
#include "lgfx/v1/lgfx_fonts.hpp"

#define LGFX_AUTODETECT
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <arabic_u8g2.c>

const char *TAG = "ARABIC_EXAMPLE";

LGFX lcd;

// ------- arabic_u8g2.c file generated using: -------
// 
// otf2bdf -r 72 -p 32 Amiri.ttf -o arabic.bdf
// bdfconv -v -f 1 -m "0-127,1536-1791,65136-65278" arabic.bdf -o arabic_u8g2.c -n arabic_u8g2 -d arabic.bdf
// 
// `0 - 127` is the unicode range for latin letters
// `1536 - 1791 is the unicode range for the Arabic (Letters in isolated form only)`
// `65136 - 65278 is the unicode range for the Arabic Presentation Forms-B` (Includes all letter forms)
// See https://unicodemap.com/ for more info
// The "x.tga" file is an image that shows all the bitmaps in the generated `u8g2.c` file
// I get an error because of `U8G2_FONT_SECTION("arabic_u8g2")` so I remove it from the `u8g2.c` file manually everytime I run bdfconv
lgfx::U8g2font arabic_font (arabic_u8g2);

struct ArabicWord {
    std::u16string word;
    uint16_t wordWidth = 0;
    std::vector<uint8_t> letterWidths;
};
struct ArabicMapping {
    uint16_t base, initial, medial, final;
};
static ArabicMapping arabic_map[] = {
    {0x0621, 0xFE80, 0xFE80, 0xFE80}, // Hamza
    {0x0622, 0xFE81, 0xFE82, 0xFE82}, // Alef Madda
    {0x0671, 0x0671, 0xFB51, 0xFB51}, // Alef Hamza Wasl
    {0x0623, 0xFE83, 0xFE84, 0xFE84}, // Alef Hamza Above
    {0x0624, 0xFE85, 0xFE86, 0xFE86}, // Waw Hamza Above
    {0x0625, 0xFE87, 0xFE88, 0xFE88}, // Alef Hamza Below
    {0x0626, 0xFE8B, 0xFE8C, 0xFE8A}, // Yeh Hamza
    {0x0627, 0xFE8D, 0xFE8E, 0xFE8E}, // Alef
    {0x0628, 0xFE91, 0xFE92, 0xFE90}, // Ba
    {0x0629, 0xFE93, 0xFE94, 0xFE94}, // Teh Marbuta
    {0x062A, 0xFE97, 0xFE98, 0xFE96}, // Ta
    {0x062B, 0xFE9B, 0xFE9C, 0xFE9A}, // Tha
    {0x062C, 0xFE9F, 0xFEA0, 0xFE9E}, // Jeem
    {0x062D, 0xFEA3, 0xFEA4, 0xFEA2}, // Hah
    {0x062E, 0xFEA7, 0xFEA8, 0xFEA6}, // Khah
    {0x062F, 0xFEA9, 0xFEAA, 0xFEAA}, // Dal
    {0x0630, 0xFEAB, 0xFEAC, 0xFEAC}, // Thal
    {0x0631, 0xFEAD, 0xFEAE, 0xFEAE}, // Ra
    {0x0632, 0xFEAF, 0xFEB0, 0xFEB0}, // Zain
    {0x0633, 0xFEB3, 0xFEB4, 0xFEB2}, // Seen
    {0x0634, 0xFEB7, 0xFEB8, 0xFEB6}, // Sheen
    {0x0635, 0xFEBB, 0xFEBC, 0xFEBA}, // Sad
    {0x0636, 0xFEBF, 0xFEC0, 0xFEBE}, // Dad
    {0x0637, 0xFEC3, 0xFEC4, 0xFEC2}, // Tah
    {0x0638, 0xFEC7, 0xFEC8, 0xFEC6}, // Zah
    {0x0639, 0xFECB, 0xFECC, 0xFECA}, // Ain
    {0x063A, 0xFECF, 0xFED0, 0xFECE}, // Ghain
    {0x0641, 0xFED3, 0xFED4, 0xFED2}, // Fa
    {0x0642, 0xFED7, 0xFED8, 0xFED6}, // Qaf
    {0x0643, 0xFEDB, 0xFEDC, 0xFEDA}, // Kaf
    {0x0644, 0xFEDF, 0xFEE0, 0xFEDE}, // Lam
    {0x0645, 0xFEE3, 0xFEE4, 0xFEE2}, // Meem
    {0x0646, 0xFEE7, 0xFEE8, 0xFEE6}, // Noon
    {0x0647, 0xFEEB, 0xFEEC, 0xFEEA}, // Ha
    {0x0648, 0xFEED, 0xFEEE, 0xFEEE}, // Waw
    {0x0649, 0xFEEF, 0xFEE0, 0xFEF0}, // Alef Layina
    {0x064A, 0xFEF3, 0xFEF4, 0xFEF2}, // Yeh
    {0xFEF5, 0xFEF5, 0xFEF6, 0xFEF6}, // Lam & Alef Hamza Madda
    {0xFEF7, 0xFEF7, 0xFEF8, 0xFEF8}, // Lam & Alef Hamza Above
    {0xFEF9, 0xFEF9, 0xFEFA, 0xFEFA}, // Lam & Alef Hamza Below
    {0xFEFB, 0xFEFB, 0xFEFC, 0xFEFC}, // Lam & Alef
};

bool isLetter(uint16_t character) {
    for (auto &entry : arabic_map)
        if (entry.base == character) 
            return true;
    return false;
}
const ArabicMapping* getArabicMapping(uint16_t character) {
    for (auto &entry : arabic_map)
        if (entry.base == character) 
            return &entry;
    return nullptr;
}
std::vector<ArabicWord> shapeArabicText(const std::u16string &input) {
    lgfx::FontMetrics ch_metric;
    std::vector<ArabicWord> text;
    text.emplace_back();

    bool connectPrevious = false;
    bool connectNext = false;
    for (uint16_t i = 0; i < input.size(); i++) {
        uint16_t ch = input[i];
        if (ch == u' ') {
            if (!text.back().word.empty()) text.emplace_back();
            text.back().word.push_back(u' ');
            text.back().letterWidths.push_back(lcd.textWidth(" "));
            text.back().wordWidth = text.back().letterWidths.back();
            connectPrevious = false;
            text.emplace_back();
        } else {
            // All just for Lam & Alef
            if ((ch == u'ا' || ch == u'أ' || ch == u'إ' || ch == u'آ') && !text.back().word.empty()) {
                uint8_t word_size = text.back().word.size();
                bool lam_found = false;
                uint8_t j = 1;
                while (1) {
                    if (!lam_found && input[i - j] == u'ل') { 
                        text.back().wordWidth -= text.back().letterWidths[word_size - j];
                        text.back().word.erase(word_size - j); 
                        text.back().letterWidths.erase(text.back().letterWidths.begin() + (word_size - j), text.back().letterWidths.end()); 
                        switch (ch) {
                            case u'ا': ch = 0xFEFB; break;
                            case u'أ': ch = 0xFEF7; break;
                            case u'إ': ch = 0xFEF9; break;
                            case u'آ': ch = 0xFEF5; break;
                        }
                        lam_found = true;
                    }
                    else if (isLetter(input[i - j])) {
                        if (lam_found) {
                            switch (input[i - j]) {
                                case u'ٱ': case u'أ': case u'إ': case u'آ': case u'ا': case u'و': case u'ؤ': case u'ز': case u'ر': case u'ذ': case u'د': case u'ء':
                                    connectPrevious = false;
                            }
                        }
                        break;
                    } 
                    if (j == word_size) { if (lam_found) connectPrevious = false; break; }
                    j++;
                }
            }

            auto mapping = getArabicMapping(ch);
            if (!mapping) {
                text.back().word.push_back(ch);
                arabic_font.updateFontMetric(&ch_metric, ch);
                text.back().wordWidth += ch_metric.x_advance;
                text.back().letterWidths.push_back(ch_metric.x_advance);
                ESP_LOGW(TAG, "NOT letter: %x, %i", ch, ch_metric.x_advance);
                continue;
            }

            uint16_t j = i + 1;
            while (1) {
                if (j > input.size() || input[j] == ' ') { connectNext = false; break; }
                else if (isLetter(input[j])) { connectNext = true; break; }
                j++;
            }

            uint16_t shapedCh;
            if (!connectPrevious && !connectNext) shapedCh = mapping->base;
            else if (connectPrevious && connectNext) shapedCh = mapping->medial;
            else if (connectPrevious && !connectNext) shapedCh = mapping->final;
            else shapedCh = mapping->initial;

            text.back().word.push_back(shapedCh);
            arabic_font.updateFontMetric(&ch_metric, shapedCh);
            if (ch_metric.x_advance < 0) {
                ESP_LOGE(TAG, "%i", ch_metric.x_advance);
                ch_metric.x_advance = 20;
            }
            text.back().wordWidth += ch_metric.x_advance;
            text.back().letterWidths.push_back(ch_metric.x_advance);

            connectPrevious = true;
            ESP_LOGW(TAG, "%x, %x, %i", ch, shapedCh, ch_metric.x_advance);
            switch (ch) {
                case u'ٱ': case u'أ': case u'إ': case u'آ': case u'ا': case u'و': case u'ؤ': case u'ز': case u'ر': case u'ذ': case u'د': case u'ء':
                    connectPrevious = false;
            }
            switch (shapedCh) {
                case 0xFEF5: case 0xFEF6: case 0xFEF7: case 0xFEF8: case 0xFEF9: case 0xFEFA: case 0xFEFB: case 0xFEFC:
                    connectPrevious = false;
            }
        }
    }

    return text;
}

std::u16string utf8_to_utf16(const std::string& utf8_str) {
    std::u16string utf16_str;
    size_t i = 0;
    while (i < utf8_str.size()) {
        uint32_t codepoint = 0;
        unsigned char ch = utf8_str[i];

        if (ch <= 0x7F) {
            // 1-byte sequence (ASCII)
            codepoint = ch;
            i++;
        } else if ((ch & 0xE0) == 0xC0) {
            // 2-byte sequence
            codepoint = ((utf8_str[i] & 0x1F) << 6) | (utf8_str[i + 1] & 0x3F);
            i += 2;
        } else if ((ch & 0xF0) == 0xE0) {
            // 3-byte sequence
            codepoint = ((utf8_str[i] & 0x0F) << 12) |
                        ((utf8_str[i + 1] & 0x3F) << 6) |
                        (utf8_str[i + 2] & 0x3F);
            i += 3;
        } else if ((ch & 0xF8) == 0xF0) {
            // 4-byte sequence (rare for Arabic text)
            codepoint = ((utf8_str[i] & 0x07) << 18) |
                        ((utf8_str[i + 1] & 0x3F) << 12) |
                        ((utf8_str[i + 2] & 0x3F) << 6) |
                        (utf8_str[i + 3] & 0x3F);
            i += 4;
        } else {
            // Invalid UTF-8 sequence, handle error
            ESP_LOGW(TAG, "Invalid UTF-8 sequence");
            i++;
        }

        // Convert codepoint to UTF-16
        if (codepoint <= 0xFFFF) {
            utf16_str.push_back(static_cast<char16_t>(codepoint));
        } else {
            // Surrogate pair. Ignore
            ESP_LOGW(TAG, "Ignoring surrogate pair: %lx", codepoint);
        }
    }
    return utf16_str;
}

void loop(void*) {
    lcd.init();
    lcd.setRotation(1);
    lcd.setFont(&arabic_font);
    uint16_t scr_width = lcd.width();
    ESP_LOGI(TAG, "Screen initialized successfully, width: %i", scr_width);
    
    std::u16string text = u"السلام عليكم ورحمة الله وبركاته";
    std::vector<ArabicWord> shapedText = shapeArabicText(text);

    uint8_t margin = 10;
    std::vector<uint16_t> lineWidths;
    lineWidths.emplace_back();
    for (auto word : shapedText) {
        if (lineWidths.back() + word.wordWidth > scr_width - margin*2) {
            lineWidths.emplace_back();
        }
        lineWidths.back() += word.wordWidth;
        ESP_LOGW(TAG, "lineWidth: %i, wordWidth: %i", lineWidths.back(), word.wordWidth);
    }
 
    uint8_t j = 0;
    uint16_t y = 60;
    int16_t endX = (scr_width - lineWidths[j])/2;
    int16_t x = scr_width - endX;
    for (auto word : shapedText) {
        if (x <= endX + 1) {
            j++;
            endX = (scr_width - lineWidths[j])/2;
            x = scr_width - endX;
            y += 60;
        }

        for (uint16_t i=0; i < word.word.size(); i++) {
            lcd.drawChar(word.word[i], x - word.letterWidths[i], y);
            x -= word.letterWidths[i];
        }
        ESP_LOGW(TAG, "endX: %i, x: %i", endX, x);
    }

    vTaskDelete(NULL);
}

extern "C" void app_main(void) {
  xTaskCreate(loop, "app", 8192, NULL, 1, NULL);
}

