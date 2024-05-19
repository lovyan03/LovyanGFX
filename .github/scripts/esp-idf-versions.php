<?php

/*
 * esp-idf-versions.php
 *
 * copyleft (c+) tobozo 2024
 *
 * builds an array of esp-idf-fqbn for a 2D workflow matrix [idf version ^ esp32 chip]
 * by querying the official espressif support matrix and the repository tags, e.g.
 *
 * {
 *    "esp-idf-fqbn": [
 *        "esp32@5.2.1",
 *        "esp32s2@5.2.1",
 *        "esp32s3@5.2.1",
 *        "esp32c6@5.2.1",
 *        "esp32@5.1.4",
 *        "esp32s2@5.1.4",
 *        "esp32s3@5.1.4",
 *        "esp32c6@5.1.4",
 *        "esp32@5.0.6",
 *        "esp32s2@5.0.6",
 *        "esp32s3@5.0.6",
 *        "esp32@4.4.7",
 *        "esp32s2@4.4.7",
 *        "esp32s3@4.4.7",
 *        "esp32@4.4.6",
 *        "esp32@4.3.6",
 *        "esp32@4.1.4"
 *    ]
 * }
 *
 */


$max_versions   = 4;
$idf_releases   = [];
$patch_versions = [];
$idf_fqbns      = [];
$idf_versions   = []; // extracted from git ls
$idf_official_versions = []; // extracted from their readme
$idf_boards     = ['esp32', 'esp32s2', 'esp32s3', 'esp32c6'/*, 'esp32h2', 'esp32p4', 'esp32c5'*/];


// get the official support matrix from the readme on the main espidf branch.
// it contains tabulated chips and their supported versions in markdown format
// this allows associating each version with a set of supported chips in the CI matrix
$idf_readme_url = 'https://raw.githubusercontent.com/espressif/esp-idf/master/README.md';
// fetch the readme or die
$readmeContents = file_get_contents($idf_readme_url) or die("Unable to fetch URL ".$idf_readme_url);
// split document into an array of lines
$lines = explode(PHP_EOL, $readmeContents);
foreach($lines as $line)
{
  // Markdown uses "|" as a column separator, at least 5 of those are expected.
  // It is also speculated that there is only one table in the document.
  $columns_count = substr_count($line, "|");
  if( $columns_count < 5 )
    continue;
  // remove/clean markdown from cells contents
  $cells = explode("|", $line);
  foreach($cells as $idx => $value)
  {
    $value = trim($value);
    $value = str_replace(array("!", "[", "]", "alt text", "(", ")", "Announcement"), "", $value);
    $cells[$idx] = $value;
  }
  // collect/validate cell values
  if(count($idf_official_versions)==0)
  {
    if($cells[1]!="Chip")
    {
      print_r($cells);
      die("can't find the support matrix! expects 'Chip' at index 1 but got ".$cells[1]);
    }
    foreach($cells as $idx => $cell)
    {
      if( preg_match("/^v[0-9.]+$/", $cell) )
      {
        $idf_official_versions[$idx] = [ "version" => $cell, "chips" => [] ];
      }
    }
  }
  else
  {
    if( count($idf_official_versions) == 0)
      die("failed to collect official versions");
    if(!preg_match("/^ESP32/", $cells[1]))
      continue; // separator or end of tabular data
    $idf_board_name = strtolower(str_replace("-", "", $cells[1])); // e.g. ESP32-C3 -> esp32c3
    for($i=2;$i<count($cells)-1;$i++)
    {
      if($cells[$i]=='supported')
        $idf_official_versions[$i]['chips'][] = $idf_board_name;
    }
  }
}

// matches $idf_version **minor/patch* version with its officially supported *major* version
// for a given $idf_board esp32 chip
function board_supported_by( $idf_board, $idf_version )
{
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


// get the idf version numbers associated with their release state.
// this allows tag based filtering if beta/rc/unstable releases while
// resolving to the latest stable patch version (e.g. 5.2.x => 5.2.1)
$git_remote = `git ls-remote https://github.com/espressif/esp-idf`;

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
      $idf_releases[] = end($line_parts);
}

!empty($idf_releases) or php_die("releases not found");

arsort( $idf_releases );

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
    if( !in_array( 'v'.$minor, $idf_releases ) )
      continue; // this tag is not listed in releases
      if( /*!empty($results[3]) &&*/ !in_array( $patch, $patch_versions ) )
        $patch_versions[] = $patch;
  }
}

!empty($patch_versions) or php_die("tags not found");

arsort( $patch_versions );

$max_boards = (count($idf_boards)*$max_versions);

// match release versions with tag versions
foreach( $idf_releases as $minor )
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
