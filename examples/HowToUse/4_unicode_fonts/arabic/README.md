# Displaying Arabic
This example is to showcase displaying arabic using u8g2. This is a hard task since the script needs to display a different character depending on if the letter is isolated or connected from right, left or both sides.

**To run this using ESP-IDF:**
`cd LovyanGFX/examples/HowToUse/4_unicode_fonts/arabic/`
`idf.py flash monitor`
> I hope it works

I found Amiri.ttf decent so that's the font I used.

> Note, an addition that could be made to this is by displaying letters that can combine, (e.g. "seen" & "meem" (at the end of the word)) in their combined form. Only few fonts have the bitmap for these though. They can be found at `Arabic Presentation Forms-A` in https://unicodemap.com/

> Thank you for reading me.
