<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: spell_parser.inc,v 1.2 2003/10/09 05:54:04 gsherwood Exp $
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
* @package MySource_Fudge
* @subpackage cms
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
			echo $key.'="'.addslashes($val).'" ';
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
	function spellCallback($match = Array())
	{
		$check_spelling = true;
		$word = $match[0];

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
			return utf8_encode(htmlspecialchars($word));
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

			$retv .= '</span>';
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
		echo preg_replace_callback('/\w+/',Array($this, 'spellCallback'), $data);
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
		$GLOBALS['spellerid'] = pspell_new($GLOBALS['dict']);
		return pspell_config_mode($GLOBALS['spellerid'], PSPELL_NORMAL);

	}// end setLanguage

}// end class

?>