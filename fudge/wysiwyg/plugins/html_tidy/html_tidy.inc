<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: html_tidy.inc,v 1.4 2003/10/03 06:53:16 gsherwood Exp $
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
	* Constructor
	*
	* @return object HTML_Tidy
	* @access public
	*/
	function HTML_Tidy()
	{

	}//end constructor


	/**
	* Cleans the submitted HTML using HTML Tidy
	*
	* @param string	&html	the HTML that was submitted by the editor
	*
	* @return string
	* @access public
	*/
	function process(&$html)
	{
		$tidy = Array();
		$path_to_tidy = '/home/gsherwood/mysource_matrix/tidy';
		$command = "/bin/echo ".escapeshellarg($html)." | $path_to_tidy -iq --show-body-only y --show-errors 0 --show-warnings 0 --wrap 0 -asxml --word-2000 1 --force-output 1 --new-blocklevel-tags \"htmlarea_special\"";
		exec($command, $tidy);
		$html = implode("\n", $tidy);

	}//end process

}//end class

?>