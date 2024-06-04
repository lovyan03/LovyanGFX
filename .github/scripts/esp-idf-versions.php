<?php

/*
 * esp-idf-versions.php
 *
 * copyleft (c+) tobozo 2024
 *
 * builds an array of esp-idf-fqbn for a 2D workflow matrix [idf version ^ esp32 chip]
 * from dl.espressif.com/dl/esp-idf/idf_versions.js javascript object
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

// hardcoded EOL versions, official versions will be added by the script
$hardcoded_fqbns = ['esp32@4.3.6', 'esp32@4.1.4'];

// restrict output to these idf targets, other targets will be ignored
$idf_boards     = ['esp32', 'esp32s2', 'esp32s3', 'esp32c6'/*, 'esp32h2', 'esp32p4', 'esp32c5'*/];

// get the official support matrix from the espressif website
// it contains a JavaScript object declaration with all necessary chip/version informations
// this allows associating each version with a set of supported chips in the CI matrix
$idf_versions_js_url = "https://dl.espressif.com/dl/esp-idf/idf_versions.js";
$js = file_get_contents($idf_versions_js_url) or php_die("Unable to fetch ".$idf_versions_js_url);
// convert the Javascript object to a PHP associative array
$idf_versions_json = JsConverter::convertToArray( $js );
// do some basic health checks
!empty($idf_versions_json) or php_die("unable to parse idf_versions.js at ".$idf_versions_js_url);
isset($idf_versions_json['VERSIONS']) or php_die("invalid JSON in ".$idf_versions_js_url);
!empty($idf_versions_json['VERSIONS']) or php_die("no VERSIONS found in ".$idf_versions_js_url);

$fqbns = [];

foreach($idf_versions_json['VERSIONS'] as $version)
{
  if(!isset($version['name']) || !isset($version['old']) || !isset($version['supported_targets']) )
    continue; // skip first entry, EOL, unsupported and preview versions
  if($version['old']!=='false')
    continue; // only keep current versions

  foreach($version['supported_targets'] as $board)
  {
    if(in_array($board, $idf_boards)) // only keep supported targets
    {
      // e.g. esp32@5.2.1
      $fqbns[] = $board.'@'.str_replace(["v","V"], "", $version['name']);
    }
  }

}

// merge collected fqbns with hardcoded fqbns
array_push($fqbns, ...$hardcoded_fqbns);

// print the json and exit
php_die( json_encode( [ "esp-idf-fqbn" => $fqbns ], JSON_PRETTY_PRINT ), 0 );

// same as die() with with end of line
function php_die($msg, $errcode=1)
{
  echo $msg.PHP_EOL;
  exit($errcode);
}


// grabbed from https://github.com/ovidigital/js-object-to-json/blob/master/src/JsConverter.php
// changes by tobozo:
// - added ::trimToBrackets(string $input):string
// - disabled the unquoting of booleans in ::convertToJson()
class JsConverter
{
  /**
   * Converts a JavaScript object string to a JSON formatted string.
   *
   * @param string $jsObjectString The JavaScript object
   * @return string
   */
  public static function convertToJson(string $jsObjectString): string
  {
    $replacedStringsList = [];
    // Remove single line comments
    $convertedString = self::removeSingleLineComments($jsObjectString);
    // Remove functions from objects
    $convertedString = self::removeFunctions($convertedString);
    // Replace all delimited string literals with placeholders
    $convertedString = self::escapeSingleQuoteBetweenDoubleQuotes($convertedString);
    $convertedString = self::replaceSectionsWithPlaceholders($convertedString, $replacedStringsList, "`");
    $convertedString = self::replaceSectionsWithPlaceholders($convertedString, $replacedStringsList, "'");
    self::fixQuoteEscapingForSingleQuoteDelimitedStrings($replacedStringsList);
    $convertedString = self::unescapeSingleQuoteBetweenDoubleQuotes($convertedString);
    $convertedString = self::replaceSectionsWithPlaceholders($convertedString, $replacedStringsList, '"');
    // Now is safe to remove all white space
    $convertedString = preg_replace('/\s+/m', '', $convertedString);
    // And remove all trailing commas in objects
    $convertedString = str_replace([',}', ',]'], ['}', ']'], $convertedString);
    // Add double quotes for keys
    $convertedString = preg_replace('/([^{}\[\]#,]+):/', '"$1":', $convertedString);
    // Add double quotes for values
    $convertedString = preg_replace_callback(
      '/:([^{}\[\]#,]+)/',
      static function ($matches)
      {
        if (is_numeric($matches[1]))
        {
          return ':' . $matches[1];
        }

        return ':"' . $matches[1] . '"';
      },
      $convertedString
    );
    // Make sure "true", "false" and "null" values get delimited by double quotes
    // Need to run the replacement twice, because not all values get replaced if they are adjacent
    $convertedString = preg_replace('/([^"])(true|false|null)([^"])/', '$1"$2"$3', $convertedString);
    $convertedString = preg_replace('/([^"])(true|false|null)([^"])/', '$1"$2"$3', $convertedString);
    // Replace the placeholders with the initial strings
    $deep = false;
    do
    {
      $convertedString = preg_replace_callback(
        '/###(\d+)###/',
        static function ($matches) use (&$replacedStringsList, $deep)
        {
          $replace = $replacedStringsList[$matches[1]];
          unset($replacedStringsList[$matches[1]]);
          return ($deep ? "'" . $replace . "'" : '"' . $replace . '"');
        },
        $convertedString
      );

      $deep = true;
    } while (!empty($replacedStringsList));
    // Note: unquoting booleans is disabled for the sakes of this script only
    // $convertedString = preg_replace('/:(")(true|false|null)(")/', ':$2', $convertedString);
    return self::trimToBrackets($convertedString);
  }

  /**
   * Convert the given JS object to JSON and json_decode it
   *
   * @param string $input
   * @return array|null
   */
  public static function convertToArray(string $input): ?array
  {
    return json_decode(self::convertToJson($input), true);
  }

  // remove (var/let/const) root declaration and trailing semicolon
  // so the output is really enclosed by curly brackets and no javascript
  // artefact remains
  public static function trimToBrackets(string $input): string
  {
    $input = trim($input);
    if($input[0] !== '{')
    {
      $input = substr($input, strpos($input, '{'));
    }
    if($input[-1] == ';' )
      $input[-1] = ' ';
    //echo $input;
    return trim($input);
  }

  /**
   * Remove all JS functions by counting the number of braces between open and
   * close (excl strings which should be placeholder-ed at this point).
   *
   * Removes shorthand and longhand functions whether they're single or multi-line:
   *     key: (var) => 'Test',
   *     key: var => 'Test',
   *     key: () => 'Test',
   *     key: () => { return 'Test'; },
   *     key: (var) => {
   *         return 'Test';
   *     },
   *     key: () => {
   *         return 'Test';
   *     },
   *     key: () => {
   *         if (complex) {
   *             return 'Test';
   *         }
   *
   *         return 'Test';
   *     },
   *     key() {
   *         return 'Test';
   *     },
   *
   * @param string $input
   * @param boolean $debug (optional table view of the logic being broken down)
   * @return string
   */
  protected static function removeFunctions(string $input, bool $debug = false): string
  {
    $functionLines = '/^(\s*)(?:([\'"]?\w+[\'"]?):\s*(function\s*\([^)]*\)\s*{|\s*(?:\([^)]*\)|[a-z0-9]+)\s*=>\s*)|[a-z0-9]+\([^)]*\)\s*{)/i';
    $lines = preg_split('/[\n\r]/', $input);

    $delete = false;

    $opens = 0;
    $closes = 0;
    $table = [];

    foreach ($lines as $index => $line)
    {
      $row = [
        'line' => $line,
        'mode' => 'standard',
        'action' => 'Keep',
        'opens' => $opens,
        'closes' => $closes,
      ];

      if (preg_match($functionLines, $line))
      {
        $opens = substr_count($line, '{');
        $closes = substr_count($line, '}');

        $row['mode'] = 'Start (found opens: ' . substr_count($line, '{') . ')';
        $row['opens'] = $opens;
        $row['closes'] = $closes;
        unset($lines[$index]);

        if ($opens === $closes)
        {
          $row['action'] = 'Delete (Final)';
        }
        else
        {
          $delete = true;
          $row['action'] = 'Delete (Continue)';
        }


        $table[] = $row;
        continue;
      }

      if ($delete)
      {
        $opens += substr_count($line, '{');
        $closes += substr_count($line, '}');

        $row['opens'] = $opens;
        $row['closes'] = $closes;
        $row['action'] = 'Delete';

        unset($lines[$index]);

        if ($opens === $closes)
        {
          $row['action'] = 'Delete (Final)';
          $delete = false;
        }
      }

      if ($debug)
      {
        $table[] = $row;
      }
    }

    if ($debug)
    {
      print '<table><tr><th>Index</th><th>Line</th><th>Mode</th><th>Action</th><th>Opens</th><th>Closes</th></tr>';

      foreach ($table as $index => $row)
      {
        print '<tr>'
        . '<td>' . $index . '</td>'
        . '<td><pre>'. $row['line'] . '</pre></td>'
        . '<td>'. $row['mode'] . '</td>'
        . '<td>'. $row['action'] . '</td>'
        . '<td>'. $row['opens'] . '</td>'
        . '<td>'. $row['closes'] . '</td>'
        . '</tr>';
      }

      print '</table>';
      die();
    }

    return implode("\n", $lines);

  }

  /**
   * Replaces sections enclosed by a specified delimiter with placeholders of form '###<PLACEHOLDER_INDEX>###'.
   *
   * @param string $input The string input
   * @param array $replacedSectionsList The replaced sections will be added to this array
   * @param string $delimiter The delimiter that encloses the sections (e.g. "'" - single quote)
   * @param bool $removeDelimitersFromSections Flag to remove or preserve the delimiters for the sections
   * @return string
   */
  protected static function replaceSectionsWithPlaceholders(
    string $input,
    array &$replacedSectionsList,
    string $delimiter,
    bool $removeDelimitersFromSections = true
  ): string
  {
    $output = $input[0];
    $sectionStartPos = $sectionEndPos = -1;
    $contentCopiedUntilPos = 0;

    for ($i = 1; $i < strlen($input) - 1; $i++)
    {
      $char = $input[$i];
      $prevChar = $input[$i - 1];

      if ($char === $delimiter && $prevChar !== '\\')
      {
        if ($sectionStartPos === -1)
        {
          $sectionStartPos = $i;
        }
        elseif ($sectionEndPos === -1)
        {
          $sectionEndPos = $i;
        }
      }

      // If a section has been identified
      if ($sectionEndPos > -1)
      {
        $output .= substr($input, $contentCopiedUntilPos + 1, $sectionStartPos - $contentCopiedUntilPos - 1);

        // Replace section with placeholder
        $output .= '###' . count($replacedSectionsList) . '###';

        // Extract the section and add it to the replaced sections list
        if ($removeDelimitersFromSections)
        {
          $section = substr($input, $sectionStartPos + 1, $sectionEndPos - $sectionStartPos - 1);
        }
        else
        {
          $section = substr($input, $sectionStartPos, $sectionEndPos - $sectionStartPos);
        }
        $replacedSectionsList[] = $section;

        // Update relevant local vars
        $contentCopiedUntilPos = $sectionEndPos;
        $sectionStartPos = -1;
        $sectionEndPos = -1;
      }
    }

    $output .= substr($input, $contentCopiedUntilPos + 1);

    return $output;
  }

  /**
   * Fix the escaping for quotes inside strings that were initially delimited by single quotes.
   *
   * For example:
   * ```
   *  'string containing \' single quote' => "string containing ' single quote"
   *  'string containing " double quote' => "string containing \" double quote"
   * ```
   * @param array $strings
   */
  protected static function fixQuoteEscapingForSingleQuoteDelimitedStrings(array &$strings): void
  {
    foreach ($strings as &$string)
    {
      // Escape contained double quotes
      $string = preg_replace('/([^\\\])"/', '$1\"', $string);
      // Unescape contained single quotes
      $string = preg_replace("/\\\\'/", "'", $string);
    }
  }

  protected static function escapeSingleQuoteBetweenDoubleQuotes(string $jsonString): string
  {
    return preg_replace_callback(
      '/("[^\'"\n]*\'[^\'"\n]*")/',
      static function ($matches)
      {
        return str_replace("'", "\'", $matches[1]);
      },
      $jsonString
    );
  }

  protected static function unescapeSingleQuoteBetweenDoubleQuotes(string $jsonString): string
  {
    return preg_replace_callback(
      '/("[^\'"\n]*\'[^\'"\n]*")/',
      static function ($matches)
      {
        return str_replace("\'", "'", $matches[1]);
      },
      $jsonString
    );
  }

  /**
   * Removes single line comments (// ) from the given string.
   *
   * In order to avoid conflicts with URIs, the comments are only removed if the two forward slashes (//)
   * are immediately followed by a white space character.
   *
   * @param string $jsonString
   * @return string
   */
  protected static function removeSingleLineComments(string $jsonString): string
  {
    return preg_replace("@//\s.*$@m", "", $jsonString);
  }
}
