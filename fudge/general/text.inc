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
* $Id: text.inc,v 1.31.2.1 2007/05/04 06:01:48 skim Exp $
*
*/


/**
* General Text Functions
*
* Purpose
*     Generic functions that are useful, but don't fall into any other category
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.31.2.1 $
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
	preg_match_all ('/%([a-zA-Z_\-0-9\.'.$additional.']+)%/', $text, $matches, PREG_PATTERN_ORDER);
	return (count($matches)) ? $matches[1] : Array();

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
function get_word_counts($text, $split_regexp="/[\t\n\r ]+/", $word_match_regexp="/[^ \t\n\r]+/", $min_length=1)
{
	// type verification
	if (!is_string($text) || !is_string($split_regexp) || !is_string($word_match_regexp) || !is_int($min_length)) {
		return Array();
	} else if (!is_int($min_length) || ($min_length < 0)) {
		return Array();
	}

	$words = preg_split($split_regexp, $text);

	foreach ($words as $key => $word) {
		$words[$key] = trim(remove_silent_chars($word));
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
* @param string	$text	text
*
* @return string
* @access public
*/
function remove_silent_chars($text)
{
	$silent_chars = Array('.', ':', ';', ',', '!', '?', '\'', '"', '(', ')', '`', '~', '!', '@', '#', '$', '%', '^', '&', '*', '-', '_', '=', '+', '{', '}', '[', ']', '|', '\\', '<', '>', '/');
	return str_replace($silent_chars, '', $text);

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
*
* @param string	&$subject	subject
*
* @return void
* @access public
*/
function escape_php(&$subject)
{
	// not necessary to escape the php end tag when outside of a PHP block
	$subject = str_replace('<?', "<?php echo '<'.'?'; ?>", $subject);
	$subject = preg_replace('/<script[^>]*language=[^>]*php[^>]*>/i', "<?php echo '<'.'script language=\"php\">\'; ?>", $subject);

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


?>