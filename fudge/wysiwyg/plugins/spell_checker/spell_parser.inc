<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: spell_parser.inc,v 1.1 2003/10/08 22:36:32 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

require_once "XML/Parser.php";

/**
* Spell_Parser
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Fudge
* @subpackage cms
*/
Class Spell_Parser extends XML_Parser {

	
	/**
	* Gets called when a start tag is parsed
	*
	* @param Object XML_Parser	$xp			the XML Parser
	* @param string				$element	the name of the element
	* @param &Array(string)		$attribs	the attributes in the start element
	*
	* @access public
	* @return boolean
	*/
	function startHandler($xp,$elem, &$attribs) 
	{
		if ($elem == "spellThis") return false;
		echo '<'.$elem.' ';
		foreach ($attribs as $key => $val) {
			echo $key.'="'.addslashes($val).'" ';
		}
		echo ' >';
		return true;

	} // end startHandler()


	/**
	* gets called by the parser at the end of a tag
	*
	* @param Object XML_Parser	$xp			the XML Parser
	* @param string				$element	the name of the element
	*
	* @access public
	* @return boolean
	*/
	function endHandler($xp, $elem) 
	{
		if ($elem == "spellThis") return false;
		echo '</'.$elem.'>';

		return true;

	} // end endHandler()
	

	/**
	* gets called for each of the words
	*
	* @param Array(String) $match the match array constructed by preg_match
	*
	* @access public
	* @return string
	*/
	function spellCallback($match = Array()) 
	{
		$word = $match[0];
		if (pspell_check($GLOBALS['spellerid'], $word)) {
			return utf8_encode(htmlspecialchars($word));
		} else {
			error_log($word);
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
	} // end spellCallback()


	/**
	* gets called by the parser to handle default data
	*
	* @param Object XML_Parser	$xp			the XML Parser
	* @param string				$element	the name of the element
	*
	* @access public
	* @return boolean
	*/
	function defaultHandler($xp, $data) 
	{
		echo preg_replace_callback('/\w+/',Array($this, 'spellCallback'), $data);
		return true;

	} // end defaultHandler()
	
	
	/**
	* set the language for pspell to use
	*
	* @param string $dict the name of the dictionary to use
	*
	* @access public 
	* @return boolean
	*/
	function setLanguage($dict = 'en') 
	{
		$GLOBALS['spellerid'] = pspell_new($GLOBALS['dict']);
		return pspell_config_mode($GLOBALS['spellerid'], PSPELL_NORMAL);

	} // end setLanguage

} // end class Spell_Parser
?>