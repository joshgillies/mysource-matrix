<?php

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
		$command = "/bin/echo ".escapeshellarg($html)." | /home/gsherwood/mysource_matrix/tidy -iq --show-body-only y --show-errors 0 --show-warnings 0 --wrap 0 -asxml";
		exec($command, $tidy);
		$html = implode("\n", $tidy);

	}//end process

}//end class

?>