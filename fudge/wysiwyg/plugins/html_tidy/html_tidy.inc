<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: html_tidy.inc,v 1.10 2003/11/10 21:34:12 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - HTML Tidy
*
* Purpose
*     A WYSIWYG plugin to clean the HTML produced by
*     the editor with HTML Tidy
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class HTML_Tidy extends Wysiwyg_Plugin {

	/**
	* Speical inline tags that HTMLTidy needs to know about
	*
	* @var array
	* @see new_inline_tag()
	*/
	var $_new_inline_tags = Array();

	/**
	* Show this plugin in the toolbar or not
	*
	* @var array
	*/
	var $_show_in_toolbar = false;


	/**
	* Constructor
	*
	* @return object HTML_Tidy
	* @access public
	*/
	function HTML_Tidy()
	{

	}//end constructor


	/**
	* Let HTMLTidy known about a special inline tag during cleaning
	*
	* @param string	$tag_name	the name of the new tag
	*
	* @return boolean
	* @access public
	*/
	function new_inline_tag($tag_name)
	{
		if (in_array($tag_name, $this->_new_inline_tags)) return true;
		$this->_new_inline_tags[] = $tag_name;
		return true;

	}//end new_inline_tag()


	/**
	* Cleans the submitted HTML using HTML Tidy
	*
	* @param string	&$html	the HTML that was submitted by the editor
	*
	* @return string
	* @access public
	*/
	function process(&$html)
	{
		if (empty($html)) return;

		  //////////////////////////////
		 //  UNSUPPORTED CHARACTERS  //
		//////////////////////////////

		// Remove the following unsupported character codes that do not get replaced
		// back to their original characters through the encoding process (mostly MS Word chars)
		$search  = Array("%u2018","%u2019","%u2013");
		$replace = Array("'"     , "'"    ,"-"     );
		$html = str_replace($search, $replace, $html);


		  /////////////////
		 //  HTML TIDY  //
		/////////////////

		// tidy the HTML produced using the HTMLTidy program
		$tidy = Array();
		$path_to_tidy = '/home/gsherwood/mysource_matrix/tidy';
		$command = "/bin/echo ".escapeshellarg($html)." | $path_to_tidy -iq --show-body-only y --show-errors 0 --show-warnings 0 --wrap 0 -asxml --word-2000 1 --force-output 1";

		// add any inline tags that other plugins may use (possibly for encapsulating plugin specific data)
		// so HTMLTidy doesnt strip them out
		if (!empty($this->_new_inline_tags)) $command .= ' --new-inline-tags "'.implode('","',$this->_new_inline_tags).'"';
		
		exec($command, $tidy);
		if (empty($tidy)) return;
		$html = implode("\n", $tidy);

	}//end process

}//end class

?>
