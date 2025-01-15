
# Displaying Quran
**This is an example which showcases how to display Quran on a display using a font dedicated for Quran. The font uses it's own character set.**

**To run this using ESP-IDF:**
`cd LovyanGFX/examples/HowToUse/4_unicode_fonts/arabic_quran/`
`mkspiffs -c main/flash -s 0x1F0000 spiffs.bin`
`esptool.py --port /dev/ttyUSB0 --baud 921600 write_flash 0x210000 spiffs.bin`
`idf.py flash monitor`
> I hope it works

I have downloaded the font from this saudi arabian government website https://qurancomplex.gov.sa/en/techquran/dev/

When downloaded it has a "hafs.ttf" file and a "hafs.json" file. The json file contains each ayah, where it is, the ayah using the **usual unicode arabic letters**, and the ayah using the **custom unicode arabic letters**. The hafs.json file is 4.2MB big, which is too big for my esp32 so I removed all of the information in it I didn't need and split it into surahs using the "preprocess_surahs.py" file chat gpt nicely made for me. That takes it down to 1MB.

### Why custom unicode arabic letters instead of the usual.
1. There's a different unicode char for each tashkeel. eg. *ba* vs *be* vs *bo*. Each letter has it's tashkeel at a suitable place for it to look good
2. There's a different unicode char for each character in each state, isolated, connected from left, connected from right, connected from both sides. Keep in mind each state has it's own tashkeel.
3. Also letter than can be drawn on top of each other have their own unicode for each tashkeel.
4. Or just see this image to understand what I mean:

-- Note: Not all possible states are there, only the ones that are found in the quran are shown. (to keep the ttf file to a managable size (probably))

**Note**, currently the words where there is a sagdah aren't included because u8g2 doesn't work with glyphs this size. You can expand on this by splitting each of these to multiple ones and hardcode displaying them in the code. I might add that myself in a bit of time.

> Thank you for reading me.
