<?php

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

?>