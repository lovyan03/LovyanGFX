<?php

$max_versions   = 4;
$releases       = [];
$patch_versions = [];
$idf_fqbns      = [];
$idf_versions   = []; // extracted from git ls
$idf_official_versions = []; // extracted from their readme
$idf_boards     = ['esp32', 'esp32s2', 'esp32s3', 'esp32c6', 'esp32h2', 'esp32p4', 'esp32c5'];

$git_remote = `git ls-remote https://github.com/espressif/esp-idf`;

$esp_idf_readme_url = 'https://raw.githubusercontent.com/espressif/esp-idf/master/README.md';
$readme_anchor_begin = '# ESP-IDF Release and SoC Compatibility';
$readme_anghor_end = '# Developing With ESP-IDF';

$readmeContents = file_get_contents($esp_idf_readme_url) or die("Unable to fetch URL ".$esp_idf_readme_url);
$lines = explode(PHP_EOL, $readmeContents);

$matrix = [];

foreach($lines as $line)
{
  $columns_count = substr_count($line, "|");
  if( $columns_count < 5 )
    continue;
  $columns = explode("|", $line);
  foreach($columns as $idx => $value)
  {
    $value = trim($value);
    $value = str_replace(array("!", "[", "]", "alt text", "(", ")", "Announcement"), "", $value);
    $columns[$idx] = $value;
  }

  if(count($matrix)==0)
  {
    if($columns[1]!="Chip")
    {
      print_r($columns);
      die("can't find the support matrix! expects 'Chip' at index 1 but got ".$columns[1]);
    }
    foreach($columns as $idx => $cell)
    {
      if( preg_match("/^v[0-9.]+$/", $cell) )
      {
        $idf_official_versions[$idx] = [ "version" => $cell, "chips" => [] ];
      }
    }
  } else
  {
    if( count($idf_official_versions) == 0)
      die("failed to collect official versions");
    if(!preg_match("/^ESP32/", $columns[1]))
      continue; // separator
    $idf_board_name = strtolower(str_replace("-", "", $columns[1]));
    for($i=2;$i<count($columns)-1;$i++)
    {
      if($columns[$i]=='supported')
        $idf_official_versions[$i]['chips'][] = $idf_board_name;
    }
  }
  $matrix[count($matrix)] = $columns;
}

//print_r($idf_official_versions);

function board_supported_by( $idf_board, $idf_version )
{
  //echo sprintf("%s, %s".PHP_EOL, $idf_board, $idf_version);
  global $idf_official_versions;
  foreach( $idf_official_versions as $idx => $chipsPerVersion )
  {
    if( str_starts_with('v'.$idf_version, $chipsPerVersion['version']) )
    {
      foreach( $chipsPerVersion['chips'] as $chip )
      {
        if( $chip == $idf_board ) return true;
      }
    }
  }
  return false;
}


!empty($git_remote) or php_die("bad github response");

$lines = explode("\n", $git_remote);

// get version numbers from enumerated releases
foreach( $lines as $num => $line )
{
  if( !preg_match("/release/", $line ) )
    continue; // tag or commit
    $line = trim($line);
  if( empty($line) )
    continue; // EOL or separator
    $line_parts = explode("/", trim($line)); // tag name is the last part
    if( !empty( $line_parts ) )
      $releases[] = end($line_parts);
}

!empty($releases) or php_die("releases not found");

arsort( $releases );

// get version numbers from enumerated tags
foreach( $lines as $num => $line )
{
  if( !preg_match("/tags/", $line ) )
    continue;
  $line = trim($line);
  $tag_parts = explode("/", $line );
  $tag_name = end( $tag_parts );
  if(  substr( $tag_name, 0, 1 ) == 'v' // esp-idf official tag names are prefixed with "v"
    && substr( $tag_name, -3 ) != '^{}' // ignore commit pointers returned by git ls-remote
    /*&& !preg_match( '/beta|dev|rc|head|merge/i', $tag_name)*/ ) // ignore beta/dev/rc and other non significant tags
  {
    if(! preg_match("/^v?(0|(?:[1-9]\d*))(?:\.(0|(?:[1-9]\d*))(?:\.(0|(?:[1-9]\d*)))?(?:\-([\w][\w\.\-_]*))?)?$/i", $tag_name, $results ) )
    {
      php_die("Bad semver with entry $num: $tag_name");
    }
    unset($results[0]);
    $semver = "v".implode('.', $results );
    if( $semver != $tag_name )
      continue; // pattern matching failed with $semver
      //php_die("uh oh pattern matching failed with $semver/$tag_name");
      $minor = $results[1].'.'.$results[2];
    $patch = !empty($results[3]) ? $results[1].'.'.$results[2].'.'.$results[3] : $minor;
    if( !in_array( 'v'.$minor, $releases ) )
      continue; // this tag is not listed in releases
      if( /*!empty($results[3]) &&*/ !in_array( $patch, $patch_versions ) )
        $patch_versions[] = $patch;
  }
}

!empty($patch_versions) or php_die("tags not found");

arsort( $patch_versions );

$max_boards = (count($idf_boards)*$max_versions);

// match release versions with tag versions
foreach( $releases as $minor )
{
  $top_version = '';
  foreach( $patch_versions as $patch )
  {
    if( str_starts_with( 'v'.$patch, $minor ) )
    {
      if( $patch > $top_version ) // SEQ comparator on a string is just cheap semver, what could go wrong ? :)
      {
        $top_version = $patch;
      }
    }
  }
  if( $top_version == '' )
    continue;

  $idf_versions[] = str_replace('v', '', $top_version );
  if( count( $idf_versions ) == $max_versions )
    break;
}

!empty($idf_versions) or php_die("latest versions not found");
!empty($idf_boards) or php_die("no board selected");

// finally fill matrix json array with jobs
foreach( $idf_versions as $idx => $idf_version )
{
  if( count( $idf_fqbns ) >= $max_boards ) {
    break;
  }
  foreach( $idf_boards as $idf_board ) {
    if( board_supported_by( $idf_board, $idf_version ) ) $idf_fqbns[] = $idf_board.'@'.$idf_version;
  }
}

// add hardcoded versions
$idf_fqbns[] = 'esp32@4.4.6';
$idf_fqbns[] = 'esp32@4.3.6';
$idf_fqbns[] = 'esp32@4.1.4';

$json_array = [ "esp-idf-fqbn" => $idf_fqbns ];

echo json_encode( $json_array, JSON_PRETTY_PRINT );


function php_die($msg)
{
  echo $msg.PHP_EOL;
  exit(1);
}
