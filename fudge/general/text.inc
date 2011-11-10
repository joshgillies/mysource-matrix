<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: text.inc,v 1.58.2.1 2011/11/10 22:20:15 cupreti Exp $
*
*/


/**
* General Text Functions
*
* Purpose
*     Generic functions that are useful, but don't fall into any other category
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.58.2.1 $
* @package Fudge
* @subpackage Text
*/


/**
* Replaces keywords on a string or array using the keywords supplied in the $replacements array
* $text can either be an array or string containing keywords to be replaced
*
* (replace_keywords() is a wrapper function for _replace_keywords_recursive() - if
* the arguments pass validation, it passes them off to the latter function to do
* the actual replacement)
*
* @param mixed	&$text			the text or array to replace in
* @param array	&$replacements	an array of replacements (keyword => replacement)
*
* @return mixed string|array
* @access public
*/
function replace_keywords(&$text, &$replacements)
{
	// type verification
	$is_replacements_array = is_array($replacements);
	if (is_string($text)) {
		if (!$is_replacements_array) return '';
	} else if (is_array($text)) {
		if (!$is_replacements_array) return Array();
	} else {
		return '';
	}

	return _replace_keywords_recursive($text, $replacements);

}//end replace_keywords()


/**
* Replaces any occurance of "%keyword%" with the replacements text
*
* @param mixed	&$text			the text or array to replace in
* @param array	&$replacements	an array of replacements (keyword => replacement)
*
* @return mixed string|array
* @access private
*/
function _replace_keywords_recursive(&$text, &$replacements)
{
	if (is_array($text)) {
		$r = Array();
		foreach ($text as $key => $value) {
			$key = _replace_keywords_recursive($key, $replacements);
			$r[$key] = _replace_keywords_recursive($value, $replacements);
		}
		$text = $r;
		return $text;
	}
	foreach ($replacements as $keyword => $replacement) {
		if ($keyword == '') continue;
		$text = str_replace("%$keyword%", $replacement, $text);
	}
	return $text;

}//end _replace_keywords_recursive()


/**
 * Parses a keyword which may contain modifier syntax. 
 *
 * e.g. asset_contents~maxwords:10
 *
 * ..would return the plain keyword asset_contents, and modifiers
 * would contain one entry for maxwords.
 *
 * @return string
 * @access public
 */
function parse_keyword($keyword, &$modifiers = NULL)
{
	$modifiers = Array();
	$parts = explode('^', $keyword);
	$keyword = array_shift($parts);

	foreach ($parts as &$p) {
		$args = preg_split('/:{1}/', $p, -1);
		$modifier = array_shift($args);
		if (!empty($modifier)) {
			array_push($modifiers, Array('modifier' => $modifier, 'args' => $args));
		}
	}

	return $keyword;

}//end parse_keyword()


/**
 * Applies a set of modifiers against given text. $text is updated in place.
 *
 * @return void
 * @access public
 */
function apply_keyword_modifiers(&$text, $modifiers)
{
	foreach ($modifiers as $m) {
		$text = apply_keyword_modifier($text, $m);
	}

}//end apply_keyword_modifiers()


/**
 * Returns a version of $text piped through a predefined modifier.
 *
 * Mofifiers should be self explanatory.
 *
 * @return string
 * @access public
 */
function apply_keyword_modifier($text, $modifier)
{

	switch ($modifier['modifier']) {

	case "urlencode":
		return urlencode($text);

	case "urldecode":
		return urldecode($text);

	case "base64encode":
		return base64_encode($text);

	case "base64decode":
		return base64_decode($text);

	case "lowercase":
		return strtolower($text);

	case "uppercase":
		return strtoupper($text);

	case "nl2br":
		return nl2br($text);

	case "escapequotes":
		return addslashes($text);

	case "escapehtml":
		return htmlspecialchars($text, ENT_QUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);

	case "striphtml":
		return strip_tags($text);

	case "maxchars":
		// One argument; the max characters as an integer.
		$max = array_get_index($modifier['args'], 0, -1);
		
		if (SQ_CONF_DEFAULT_CHARACTER_SET == 'utf-8' && extension_loaded('mbstring')) 
			return mb_substr($text, 0, $max, SQ_CONF_DEFAULT_CHARACTER_SET);
		else
			return substr($text, 0, $max);

	case "maxwords":
		// One argument; the max words as an integer.
		$max = array_get_index($modifier['args'], 0, 0);
		if (is_numeric($max)) {
			$words = str_word_count($text, 2);
			$keys = array_keys($words);

			if ($max >= count($keys)) {
				return $text;
			} else {
				return substr($text, 0, $keys[$max]);
			}
		} else {
			return $text;
		}

	case "wordcount":
		return str_word_count($text);

	case "charcount":
		return strlen($text);

	case "capitalize":
		return ucfirst($text);

	case "titleize":
		return ucwords($text);

	case "trim":
		return trim($text);

	case "underscore":
		return preg_replace('/\s+/', '_', $text);

	case "xpath":
		// Args are delimeter, xpath
		//
		// Example (ordered list of RSS item titles, using tagif modifier to ol list): 
		//
		//    %asset_contents~xpath:</li><li>://item/title~tagif:li~tagif:ol%
		//
		// The path lists all titles in the feed, delimited by </li><li>, and then conditionally wrapped in <li>, 
		// which in turn is conditionally wrapped in <ol>.
		//
		if (empty($text)) return $text;

		$delim = array_get_index($modifier['args'], 0, '');
		$path  = array_get_index($modifier['args'], 1, NULL);

		// If there are more args, this means there were colons in the path. Fixup.
		for ($i = 2; $i < count($modifier['args']); $i++) {
			$path .= ':' . $modifier['args'][$i];
		}

		if ($path) {
			try {
				$xml = new SimpleXMLElement($text);

				// if namespaces are used, register them before using them in the xpath query
				$namespaces = explode('xmlns:', $path);
				$path = array_shift($namespaces);
				foreach ($namespaces as $namespace) {
					$ns = explode('=', $namespace);
					if (count($ns) == 2) {
						$xml->registerXPathNamespace($ns[0], trim($ns[1]));
					}
				}
							
				$res = $xml->xpath($path);
				if ($res) {
					return implode($delim, $xml->xpath($path));
				}
			} catch (Exception $e) {
				// Badly formed xml, most likely.
				trigger_error($e->getMessage());
			}

		}
		return $text;

	case "empty":
		// One argument; text to display if the keyword result is empty.
		if (empty($text)) {
			// We only want one arg, which might have : in. So implode.
			$text = implode(':', $modifier['args']);
		}
		return $text;

	case "notempty":
		// One argument; text to display if the keyword result is not empty.
		if (!empty($text)) {
			// We only want one arg, which might have : in. So implode.
			$text = implode(':', $modifier['args']);
		}
		return $text;

	case "tag":
		// One argument; the tag name to wrap around the text.
		$tag = array_get_index($modifier['args'], 0, NULL);
		if ($tag) {
			if (empty($text)) {
				return "<$tag />";
			} else {
				return "<$tag>$text</$tag>";
			}
		}
		return $text;

	case "tagif":
		// Same as above, except do nothing if the text is empty.
		$tag = array_get_index($modifier['args'], 0, NULL);
		if ($tag) {
			if (empty($text)) {
				return "";
			} else {
				return "<$tag>$text</$tag>";
			}
		}
		return $text;

	case "cdata":
		// Escape CDATA closing markup.
		$text = str_replace(']]>', ']]]]><![CDATA[>', $text);
		return "<![CDATA[$text]]>";

	case "stripdecl":
		// Strip XML declaration.
		$text = preg_replace('/^<\?xml\s.*\?>/', '', $text);
		return $text;

	case "increment":
		// Increment
		return ++$text;
	
	case "decrement":
		// Decrement
		return --$text;

	case "negate":
		// Negate
		return -$text;
	
	case "abs":
		// Returns the absolute number.
		return abs($text);

	case "sign":
		// Returns the sign of a number, +, -, or 0. Overriden in args 2, 3, and 4.
		$plus  = array_get_index($modifier['args'], 0, '+');
		$minus = array_get_index($modifier['args'], 1, '-');
		$zero  = array_get_index($modifier['args'], 2, '0');

		if ($text > 0) {
			return $plus;
		} else if ($text < 0) {
			return $minus;
		} else {
			return $zero;
		}
	
	case "ceil":
		// Rounds up to integer.
		return ceil($text);

	case "floor":
		// Rounds down to integer.
		return floor($text);

	case "round":
		// Round to specified precision (0 by default).
		$precision = array_get_index($modifier['args'], 0, 0);
		return round($text, $precision);

	case "number_format":
		// Format a number with grouped thousands and round to decimal point (0 by default).
		$decimals = array_get_index($modifier['args'], 0, 0);
		return number_format($text, $decimals);

	// Arithmetic modifiers.
	case "add":
	case "subtract":
	case "modulo":
	case "divide":
	case "multiply":
	case "squareroot":
		return _run_arithmetic_keyword_modifier($modifier, $text);

	// Comparison operator modifiers. By default these return '1' if true and '' if false.
	// Return values can be overriden with the subsequent args, e.g:
	//
	//  %asset_contents~eq:Hello:MATCH:NOMATCH%
	case "eq":
	case "neq":
	case "gt":
	case "lt":
	case "gte":
	case "lte":
		return _run_comparison_keyword_modifier($modifier, $text);

	// Hash generation modifiers.
	case "md5":
		return md5($text);

	case "sha1":
		return sha1($text);

	case "date_format":
		// Format a date string and only prints the field required
		$format = array_get_index($modifier['args'], 0, '');
		return date($format, strtotime($text));						   	 
	
	case "replace":
		$toReplace   = array_get_index($modifier['args'], 0, NULL);
		$replaceWith = array_get_index($modifier['args'], 1, NULL);

		if (!is_null($toReplace) && !is_null($replaceWith)) {
			return preg_replace('/'.$toReplace.'/', $replaceWith, $text);
		}
		return $text;
                               
	case "contains":
		$toMatch = array_get_index($modifier['args'], 0, NULL);
		$success = array_get_index($modifier['args'], 1, NULL);
		$failure = array_get_index($modifier['args'], 2, NULL);

		if (!is_null($toMatch) && strpos($toMatch, '/') !== FALSE) $toMatch = str_replace('/', '\/', $toMatch);

		if (!is_null($toMatch) && preg_match_all('/'.$toMatch.'/', $text, $matches)) {
			if (isset($matches[0]) && count($matches[0]) > 0) {
				return $success ? $success : count($matches[0]);
			}
		}
		return $failure ? $failure : 0;
		
	 case "as_asset":
		$asset_keyword = array_get_index($modifier['args'], 0, '');
		if ($asset_keyword != '') {
			if ($GLOBALS['SQ_SYSTEM']->am->assetExists($text)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($text);
				if ($asset->readAccess()) {
					return $asset->getKeywordReplacement($asset_keyword);
				}
			}
		}
		return '';


	default:
		return $text;

	}

}//end _run_keyword_modifier()


/**
 * Returns a version of $text piped through a predefined arithmetic modifier.
 *
 * Mofifiers should be self explanatory.
 *
 * @return string
 * @access public
 */
function _run_arithmetic_keyword_modifier($modifier, $text)
{
	// Divide
	$n = array_get_index($modifier['args'], 0, 0);

	switch ($modifier['modifier']) {
	case "add":
		return $text + $n;
	case "subtract":
		return $text - $n;
	case "multiply":
		return $text * $n;
	case "divide":
		if ($n != 0) {
			return $text / $n;
		} else {
			return '';
		}
	case "modulo":
		if ($n != 0) {
			return $text % $n;
		} else {
			return '';
		}
	case "squareroot":
		if ($text > 0) {
			return sqrt($text);
		} else if ($text == '-1') {
			return 'i';
		} else {
			return '';
		}//end if
	default:
		return $text;
	}	

}//end _run_arithmetic_keyword_modifier()


/**
 * Returns a version of $text piped through a predefined comparison operator modifier.
 *
 * Mofifiers should be self explanatory.
 *
 * @return string
 * @access public
 */
function _run_comparison_keyword_modifier($modifier, $text)
{
	// Greater than. Returns '1' or ''.
	$n = array_get_index($modifier['args'], 0, 0);
	$t = array_get_index($modifier['args'], 1, '1');
	$f = array_get_index($modifier['args'], 2, '');

	$ret = FALSE;
	switch ($modifier['modifier']) {
	case "eq":
		$ret = $text == $n;
		break;
	case "neq":
		$ret = $text != $n;
		break;
	case "gt":
		$ret = $text > $n;
		break;
	case "gte":
		$ret = $text >= $n;
		break;
	case "lt":
		$ret = $text < $n;
		break;
	case "lte":
		$ret = $text <= $n;
		break;
	}

	if ($ret) {
		return $t;
	} else {
		return $f;
	}

}//end _run_comparison_keyword_modifier()


/**
* An alias for extract keywords
*
* This function name is confusing because id does not deal with replacements, only keywords
*
* @param string	$text		the text or array to search
* @param string	$additional	additional valid keyword charaters for this match
*
* @return array
* @access public
* @see extract_keywords
*/
function retrieve_keywords_replacements($text, $additional='')
{
	return extract_keywords($text, $additional);

}//end retrieve_keywords_replacements()


/**
* Extract keywords from a given text container
*
* @param string	$text		the text or array to search
* @param string	$additional	additional valid keyword charaters for this match
*
* @return array
* @access public
*/
function extract_keywords($text, $additional='')
{
	$matches = Array();
	$additional = preg_replace('|(.)|', '\\\\\1', $additional);

	// Match any keyword including the filter portion.
	preg_match_all ('/%([a-zA-Z_]{2,}[a-zA-Z_\-0-9\.:'.$additional.']+(\^{1}[^%]+)*)%/', $text, $matches, PREG_PATTERN_ORDER);
	return array_unique($matches[1]);

}//end extract_keywords()


/**
* Extracts word counts for a given string.
*
* Takes a text string, splits it into words (according to $split_regexp), and extracts words
* from each split element (according to $word_match_regexp).
* Words must be larger than or equal to $min_length to be output.
* It return the array, Keys: words; values: the word counts
*
* @param string	$text				text
* @param string	$split_regexp		Regular expression delineating words
* @param string	$word_match_regexp	Regular expression for matching valid words
* @param int	$min_length			The minimum length for words matched
*
* @return array
* @access public
*/
function get_word_counts($text, $split_regexp="/\s+/", $word_match_regexp="/\S+/", $min_length=1)
{
	// type verification
	if (!is_string($text) || !is_string($split_regexp) || !is_string($word_match_regexp) || !is_int($min_length)) {
		return Array();
	} else if (!is_int($min_length) || ($min_length < 0)) {
		return Array();
	}

	// Remove silent characters
	$text = remove_silent_chars($text);

	$words = preg_split($split_regexp, $text, -1, PREG_SPLIT_NO_EMPTY);

	// Get rid of the leading and trailing white space
	// Dont have good enough reason to save them
	foreach ($words as $key => $word) {
		$words[$key] = trim($word);
	}

	$word_counts = array_count_values($words);
	foreach ($word_counts as $word => $count) {
		if (strlen($word) < $min_length || !preg_match($word_match_regexp, $word, $matches)) {
			unset($word_counts[$word]);
		}
	}

	return $word_counts;

}//end get_word_counts()


/**
* Remove a set of special characters from the given text.
*
* @param string		$text			text
* @param boolean	$not_indexing	default is FALSE, when TRUE this function does not remove a number of certain characters.
* @param boolean	$oracle_keyword when TRUE, apostrophes and email address pattern are not altered in given string. This is relevent when
									search plugin for Oracle db calls this function to filter the silent chars in search keyword
									
*
* @return string
* @access public
*/
function remove_silent_chars($text, $not_indexing=FALSE, $oracle_keyword=FALSE)
{
	// Replace period (.) and comma (,) in number, e.g. 1,234.5 => 12345
	$number_pattern = '/(\d{1})[,.](\d{1})/'; // use that pattern instead of /(\d+)[,.](\d+)/ because this pattern can not replace both period and comma in one number. For example, 1,234.5 => 1234.5
	$text = preg_replace($number_pattern, '$1$2', $text);

	if (!$oracle_keyword) {
		// Remove at (@) and period (.) in email addresses, e.g. everyone@squiz.net => everyonesquiznet
		// The following pattern matches most email addresses, but it will need to be review to match all email addresses
		$email_pattern = '/([^@\s]+)@(([-a-z0-9]+\.)+[a-z]{2,})/e';
		$text = preg_replace($email_pattern, '"$1".str_replace(".", "", "$2")', $text);

		// Remove apostrophe ('), e.g. you've => youve
		$text = str_replace('\'', '', $text);
	}

	$silent_chars = Array('.', ':', ';', ',', '!', '?', '`', '!', '@', '#', '$', '^', '*', '-', '=', '+', '{', '}', '[', ']', '\\', '<', '>', '/');
	if ($not_indexing == FALSE) {
		$oracle_search_operators = Array('(',')','"','~','&','|','%');
		$silent_chars = array_merge($silent_chars, $oracle_search_operators);
		//Remove the underscore ('_') that is usually used in metadata select field, e.g. select_1 => select1
		$text = str_replace('_', '', $text); // just move underscore ('_') from the above line to this line so that it won't be removed when search in oracle
	}

	// Replace the rest of silent characters with space (' '). This will create more indexed words but it is safer, e.g. Hello,world => Hello world
	$text = str_replace($silent_chars, ' ', $text);

	return $text;

}//end remove_silent_chars()


/**
* Escape double quotes by reference
*
* @param string	&$subject	subject
*
* @return void
* @access public
*/
function escape_double_quotes(&$subject)
{
	$subject = str_replace('"', '\"', $subject);

}//end escape_double_quotes()


/**
* Escape dangerous content (eg. <?php, <?, <script language="php">) by reference
* Intended to be used on cache files that are eval()'d or include()'d
*
* @param string	&$subject	subject
*
* @return string
* @access public
*/
function escape_php(&$subject)
{
	// not necessary to escape the php end tag when outside of a PHP block
	$subject = str_replace('<?', "<?php echo '<'.'?'; ?>", $subject);
	$subject = str_replace('<%', "<?php echo '<'.'%'; ?>", $subject);
	$subject = preg_replace(
				'/
					<script[\s]*
					language[\s]*=[\s]*
					["\']?php["\']?
					[^>]*>
				/xi',
				"<?php echo '<'.'script language=\"php\">'; ?>",
				$subject
			   );
	return $subject;

}//end escape_php()


/**
* Removes spaces and capitalises first letters of words
*
* Given a string 'my best_string' will return 'MyBestString'
*
* @param string	$string			string to modify
* @param string	$word_separator	character representing a word split (in addition to space)
*
* @return string
* @access public
*/
function ucwords_no_space($string, $word_separator='_')
{
	$tmp_word = strtr($string, $word_separator, ' ');
	$tmp_word = preg_replace('/\s+/', '', ucwords($tmp_word));

	return $tmp_word;

}//end ucwords_no_space()


/**
* Makes a human-readable list of words
*
* Given an array of strings like A,B,C create a human readable list like A, B and C
* where and is an ending delimeter that can be supplied
*
* @param array	$string_array	array of words to format
* @param string	$end_delimiter	delimiter used to join last word to the list
*
* @return string
* @access public
*/
function make_readable_list($string_array, $end_delimiter='and')
{
	if (empty($string_array) || !is_array($string_array)) {
		return '';
	}

	$last = array_pop($string_array);

	if (empty($string_array)) return $last;

	$retval = implode(', ', $string_array);
	$retval .= ' '.$end_delimiter.' '.$last;

	return $retval;

}//end make_readable_list()


/**
* Escapes special regular expression chars
*
* @param string	$text
*
* @return void
* @access public
*/
function escape_regex_chars($text)
{
	return preg_replace('|([\{\}\(\)\^\$\.\*\?\+\-\[\]\\\])|', '\\\$1', $text);

}//end escape_regex_chars()

?>