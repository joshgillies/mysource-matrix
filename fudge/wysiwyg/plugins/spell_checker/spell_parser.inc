<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: spell_parser.inc,v 1.9 2003/12/16 12:42:56 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Spell_Parser
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
Class Spell_Parser {

	/**
	* Gets called when a start tag is parsed
	*
	* @param object XML_HTMLSax	&$parser	the XML_HTMLSax Parser
	* @param string				$element	the name of the element
	* @param array(string)		$attribs	the attributes in the start element
	*
	* @access public
	* @return boolean
	*/
	function openHandler(&$parser, $elem, $attribs)
	{
		echo '<'.$elem.' ';
		foreach ($attribs as $key => $val) {
			echo $key.'="'.str_replace('"', '\\"', $val).'" ';
		}
		echo ' >';
		return true;

	}// end openHandler()


	/**
	* Gets called by the parser at the end of a tag
	*
	* @param object XML_HTMLSax	&$parser	the XML_HTMLSax Parser
	* @param string				$elem		the name of the element
	*
	* @access public
	* @return boolean
	*/
	function closeHandler(&$parser, $elem)
	{
		echo '</'.$elem.'>';
		return true;

	}// end closeHandler()


	/**
	* Gets called for each of the words to check the spelling and add suggestions
	*
	* @param array(string)	$match	the match array constructed by preg_match
	*
	* @access public
	* @return string
	*/
	function spellCallback($match)
	{
		$check_spelling = true;
		$word = $match[1];

		// ignore numbers
		if ($check_spelling) {
			$converted_word = (int)$word;
			if ((string)$converted_word == $word) $check_spelling = false;
		}

		// ugnore words in all uppercase
		if ($check_spelling) {
			$converted_word = strtoupper($word);
			if ($converted_word == $word) $check_spelling = false;
		}

		if (!$check_spelling || pspell_check($GLOBALS['spellerid'], $word)) {
			return utf8_encode(htmlspecialchars($match[1]).$match[2]);
		} else {
			$retv = '<span class="HA-spellcheck-error">'.$word.'</span>';
			$suggestions = pspell_suggest($GLOBALS['spellerid'], $word);
			$retv.= '<span class="HA-spellcheck-suggestions">';

			for ($i = 0; $i < count($suggestions); $i++) {

				$retv .= utf8_encode(htmlspecialchars($suggestions[$i]));
				if ($i != count($suggestions)) {
					$retv .= ",";
				}
			}

			$retv .= '</span>'.$match[2];
			return $retv;
		}

	}// end spellCallback()


	/**
	* Gets called by the parser to handle default data
	*
	* @param object XML_HTMLSax	&$parser	the XML_HTMLSax Parser
	* @param string				$data		the characters between the tags
	*
	* @access public
	* @return boolean
	*/
	function dataHandler(&$parser, $data)
	{
		$matches = preg_match('|[^\w]|', $data);
		if ($matches) {
			// we add an extra character to the end (which we strip out later)
			// to make sure the regular expression matches the last word in the line
			$data .= '-';
			ob_start();
			echo preg_replace_callback('|(\w+)([^;\w])|', Array($this, 'spellCallback'), $data);
			$contents = ob_get_contents();
			ob_end_clean();
			echo substr($contents, 0, -1);
		} else {
			echo $this->spellCallback(Array($data, $data, ''));
		}
		return true;

	}// end dataHandler()


	/**
	* Set the language for pspell to use
	*
	* @param string	$dict	the name of the dictionary to use
	*
	* @access public
	* @return boolean
	*/
	function setLanguage($dict='en')
	{
		$pspell_config = pspell_config_create($GLOBALS['dict']);
		pspell_config_mode($pspell_config, PSPELL_FAST);
		$GLOBALS['spellerid'] = pspell_new_config($pspell_config);
		return true;

	}// end setLanguage

}// end class

?>
