How to create your own emojis pack
==================================

Requirements:
------------

- `extract_emoji.py` -> python 3.14 minimum + fonttools + Pillow + resvg-python
- `emoji_to_C.` -> php 7 mininum

1) Download a copy of [NotoColorEmoji.ttf](https://github.com/googlefonts/noto-emoji/raw/refs/heads/main/fonts/NotoColorEmoji.ttf)
2) Extract png icons (e.g. to 16x16 px images) by running `python extract_emoji.py NotoColorEmoji.ttf --resize 16`
3) Generate the data files for the sketch by running `php export_emoji_to_C.php`

Optional: edit the generated `emojis_packed.h` file to customize even more the emoji set, all groups/subgroups and emojis can be disabled independently or globally.

```c

// comment this out to disable the whole group
#define USE_GROUP_ANIMALS_NATURE

// comment this out to disable the whole subgroup
#define USE_SUBGROUP_ANIMAL_MAMMAL

// comment this out to disable a specific emoji
    { 0x1FACE, "🫎", GROUP_ANIMALS_NATURE, SUBGROUP_ANIMAL_MAMMAL, u1FACE_png, u1FACE_png_len}, // moose
    
```
