<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: text.inc,v 1.5 2003/11/18 15:42:11 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* General Text Functions
*
* Purpose
*     Generic functions that are useful, but don't fall into any other category
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage Text
*/


/**
* Takes a string an associative array of keyword => replacements
* Replaces any occurance of "%keyword%" with the replacements text
*
* @param mixed [string|array]	$text			the text or array to replace in
* @param array					&$replacements	an array of replacements (keyword => replacement)
*
* @return mixed [string|array] - whatever the type of argument $text
* @access public
*/
function replace_keywords($text, &$replacements)
{
	if(is_array($text)) {
		$r = Array();
		foreach ($text as $key => $value) {
			$key = replace_keywords($key, $replacements);
			$r[$key] = replace_keywords($value, $replacements);
		}
		return $r;
	}
	if (!empty($replacements)) {
		foreach($replacements as $keyword => $replacement) {
			if($keyword == '') continue;
			$text = str_replace("%$keyword%", $replacement, $text);
		}
	}
	return $text;

}//end replace_keywords()


/**
* Takes a string and returns all occurances of "%keyword%"
*
* @param string	$text	the text or array to search
*
* @return array
* @access public
*/
function retrieve_keywords_replacements($text)
{
	$matches = Array();
	preg_match_all ('/%([a-zA-Z() _\-0-9]+)%/', $text, $matches, PREG_PATTERN_ORDER);
	return (count($matches)) ? $matches[1] : Array();

}//end retrieve_keywords_replacements()


/**
* Extracts word counts for a given string.
*
* Takes a text string, splits it into words (according to $split_regexp), and extracts words
* from each split element (according to $word_match_regexp).
* Words must be larger than or equal to $min_length to be output.
*
* @param string $text
* @param string $split_regexp		Regular expression delineating words
* @param string $word_match_regexp	Regular expression for matching valid words
* @param int	$min_length			The minimum length for words matched
*
* @return array Keys: words; values: the word counts
* @access public
*/

function get_word_counts($text, $split_regexp="[^A-z0-9]+", $word_match_regexp="[A-z0-9]+",$min_length=3)
{
	$words = split($split_regexp, $text);
	$word_counts = array_count_values($words);
	foreach ($word_counts as $word => $count) {
		if (strlen($word) < $min_length || !ereg($word_match_regexp, $word, $matches)) {
			unset($word_counts[$word]);
		}
	}
	return $word_counts;

}//end get_word_counts()

?>