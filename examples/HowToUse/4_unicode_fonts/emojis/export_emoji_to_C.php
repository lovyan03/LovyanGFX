<?php

/*\
 *
 *

  Copyright 2026 tobozo

  https://github.com/tobozo

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the “Software”), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

 *
 *
\*/

// Job: Generate C code from png images and emoji data

$pngDir = "png"; // folder containing all fonts generated with extract_emoji.py i.e. format("u%x.png", unicode)

if(!is_dir($pngDir) || empty($pngFiles=glob("$pngDir/*.png")))
{
  die("Please run extract_emoji.py first and make sure the output dir is '$pngDir'".PHP_EOL);
}

$c_data_file = "emojis_data.h";
$c_pack_file = "emojis_packed.h";

// create empty files
file_put_contents($c_data_file, "")!==false or die("Unable to create $c_data_file");
file_put_contents($c_pack_file, "")!==false or die("Unable to create $c_pack_file");;

$total = 0;
$ignored = 0;
$notfound = 0;

$emoji_groups = [];


// Download emojis meta information (e.g. code, group+subgroup and emoji names)
{
  if(!file_exists("all_emojis.json"))
  {
    $emojis_json_txt = file_get_contents("https://unpkg.com/emoji.json/emoji.json") or die("Unable to fetch https://unpkg.com/emoji.json/emoji.json");
    file_put_contents("all_emojis.json", $emojis_json_txt) or die("Unable to save all_emojis.json");
  } else{
    $emojis_json_txt = file_get_contents("all_emojis.json");
  }
  $emojis_json = json_decode($emojis_json_txt, true);

  if(empty($emojis_json))
    die("Invalid data in all_emojis.json, delete the file and try again");
}

echo sprintf("Initial Count: %d".PHP_EOL, count($emojis_json));


// Hydrate array with png data
{
  foreach($emojis_json as $num => $emoji)
  {
    if(!preg_match("/^[a-f0-9]{4,6}$/i", $emoji['codes']))
    { // emojis with code larger than 24bits (e.g. combined) aren't supported by LovyanGFX
      unset($emojis_json[$num]);
      continue;
    }

    $subgroupname = string_to_c_var($emoji['subgroup']);
    $groupname = string_to_c_var($emoji['group']);

    if(!isset($emoji_groups[$groupname]))
      $emoji_groups[$groupname] = [];

    if(!isset($emoji_groups[$groupname][$subgroupname]))
      $emoji_groups[$groupname][$subgroupname] = [];

    $fname = sprintf("$pngDir/u%s.png", strtoupper($emoji['codes']));
    if(!file_exists($fname))
    { // emoji is documented in json file but wasn't extracted by extract_emoji.py ?
      echo sprintf("%s [$groupname] %s, 0x%s (%s) not found".PHP_EOL, $emoji['char'], $fname, $emoji['codes'], $emoji['name']);
      $notfound++;
      continue;
    }

    $xxdname = preg_replace("/[^a-z0-9]+/i", "_", basename($fname));
    $emoji['c'] = sprintf("{ 0x%-5s, \"%s\", GROUP_%s, SUBGROUP_%s, %s, %s_len}, // %s", $emoji['codes'], $emoji['char'], $groupname, $subgroupname, $xxdname, $xxdname, $emoji['name']);
    $emoji['xxd'] = xxd($fname, $xxdname);
    $emoji['bytes'] = filesize($fname);
    $emoji_groups[$groupname][$subgroupname][] = $emoji;
    $total++;
  }
}


echo sprintf("Filtered Count: %d".PHP_EOL, count($emojis_json));

// Add png bytes to data file
{
  foreach($emoji_groups as $groupname => $subgroup)
  {
    foreach($subgroup as $subgroupname => $emojis )
    {
      foreach($emojis as $emoji)
      {
        file_put_contents("$c_data_file", $emoji['xxd'], FILE_APPEND);
      }
    }
  }
}



// Add define flags to header file
foreach($emoji_groups as $groupname => $subgroup)
{
  $groupcount = 0;
  $groupbytes =  0;

  $subgroupDefine = sprintf('#if defined USE_GROUP_%s'.PHP_EOL, $groupname);

  foreach($subgroup as $subgroupname => $emojis )
  {
    if(count($emojis)==0)
    {
      unset($emoji_groups[$groupname]);
      continue;
    }

    $subgroupbytes = 0;
    $subgroupcount = count($emojis);
    $groupcount += $subgroupcount;

    foreach($emojis as $emoji)
    {
      $subgroupbytes += $emoji['bytes'];
      $groupbytes    += $emoji['bytes'];
    }

    $subgroupDefine .= sprintf("  #define USE_SUBGROUP_%s // %d items (%d bytes)".PHP_EOL, $subgroupname, $subgroupcount, $subgroupbytes);
  }

  $subgroupDefine .= '#endif'.PHP_EOL;

  $groupDefine = sprintf("#define USE_GROUP_%s // %d items (%d bytes)".PHP_EOL, $groupname, $groupcount, $groupbytes);

  file_put_contents("$c_pack_file", $groupDefine.$subgroupDefine.PHP_EOL, FILE_APPEND);
}



// Add emoji data struct to header file
{
  $header = implode(PHP_EOL, [
    PHP_EOL,
    '#include "'.$c_data_file.'"',
    PHP_EOL,
    "struct emoji_png_t {",
    "  uint32_t code;",
    "  const char* emoji;",
    "  int group_id;",
    "  int subgroup_id;",
    "  const uint8_t* data;",
    "  unsigned int data_len;",
    "};",
    PHP_EOL,
  ]);

  file_put_contents("$c_pack_file", $header, FILE_APPEND);
}


// Add emoji groups enumeration to header file
{
  $groupenums = "enum emoji_group_id_t {".PHP_EOL;
  $subgroupenums = "enum emoji_subgroup_id_t {".PHP_EOL;

  foreach($emoji_groups as $groupname => $subgroup)
  {
    $groupenums .= sprintf("  GROUP_%s,".PHP_EOL, $groupname);
    foreach($subgroup as $subgroupname => $emojis )
    {
      $subgroupenums .= sprintf("  SUBGROUP_%s,".PHP_EOL, $subgroupname);
    }
  }

  $groupenums .= "};".PHP_EOL.PHP_EOL;
  $subgroupenums .= "};".PHP_EOL.PHP_EOL;

  file_put_contents("$c_pack_file", $groupenums.$subgroupenums, FILE_APPEND);
}




// Add emoji data to header file
{
  file_put_contents("$c_pack_file", 'emoji_png_t emojis[] = {'.PHP_EOL, FILE_APPEND);

  foreach($emoji_groups as $groupname => $subgroup)
  {
    $emojis_code = sprintf(PHP_EOL."#if defined USE_GROUP_%s".PHP_EOL, $groupname );

    foreach($subgroup as $subgroupname => $emojis )
    {
      $emojis_code .= sprintf(PHP_EOL."  #if defined USE_SUBGROUP_%s".PHP_EOL, $subgroupname );
      foreach($emojis as $emoji)
      {
        $emojis_code .= '    '.$emoji['c'].PHP_EOL;
      }
      $emojis_code .= sprintf("  #endif // defined USE_SUBGROUP_%s".PHP_EOL, $subgroupname);
    }

    $emojis_code .= sprintf(PHP_EOL."#endif // defined USE_GROUP_%s".PHP_EOL, $groupname);
    file_put_contents("$c_pack_file", $emojis_code, FILE_APPEND);
  }

  file_put_contents("$c_pack_file", '};'.PHP_EOL, FILE_APPEND);
}


echo sprintf("Total: %d, Ignored: %d, Not Found: %d".PHP_EOL, $total, $ignored, $notfound);


exit;



function string_to_c_var($string)
{
  $string = preg_replace("/[^a-z0-9]+/i", "_", strtoupper($string));
  $string = preg_replace("/_+/", "_", $string);
  return $string;
}


function xxd($filename, $cname="", $cols=16)
{
  if(!file_exists($filename))
    return '';

  if($cols<8)
    $cols = 8;

  if($cname=='')
    $cname=basename($filename);

  $hex_array = unpack("C*",file_get_contents($filename));

  $chunked_array = array_chunk($hex_array, $cols);

  $ret = sprintf("const unsigned char %s[] = {".PHP_EOL, $cname);

  foreach($chunked_array as $array_col)
  {
    $line = "  ";
    foreach($array_col as $hexcol)
    {
      $line .= sprintf("0x%02x, ", $hexcol);
    }
    $ret .= $line.PHP_EOL;
  }

  $ret .= "};".PHP_EOL.PHP_EOL;
  $ret .= sprintf("unsigned int %s_len = %d;".PHP_EOL.PHP_EOL, $cname, count($hex_array));

  return $ret;
}

