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
* $Id: keyword_replace.inc,v 1.10 2005/07/19 07:20:08 skim Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');


/**
* WYSIWYG Plugin - Keyword Replace
*
* Purpose
*     A WYSIWYG plugin to add keywords to the editor using a nice interface
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package Fudge
* @subpackage wysiwyg
*/
class Keyword_Replace extends WYSIWYG_Plugin
{

	/**
	* Some text to put before and after the keyword
	*
	* @var string
	*/
	var $keyword_tag = '';

	/**
	* The keywords and their replacements
	* This array must be in form
	* <pre>
	* Array('keyword' => 'name')
	* </pre>
	*
	* @var Array()
	*/
	var $keywords = Array();


	/**
	* Constructor
	*
	* @access public
	*/
	function Keyword_Replace(&$wysiwyg)
	{
		$this->_add_button('addkeyword','','','','');

	}//end constructor


	/**
	* Set the options for the select box as vars of the object
	*
	* @return void
	* @access public
	*/
	function print_plugin_vars()
	{
		?>
		this.config.keywords = Array(
			<?php
			$string = '';
			foreach ($this->keywords as $keyword => $name) {
				$keyword = str_replace(Array('"', "\r", "\n"), Array('\"', '\r', '\n'), $keyword);
				if (empty($name)) {
					$name = ucwords(str_replace('_', ' ', $keyword));
				} else {
					$name = str_replace(Array('"', "\r", "\n"), Array('\"', '\r', '\n'), $name);

				}
				$string .= "\n".'Array("'.$this->keyword_tag.$keyword.$this->keyword_tag.'", "'.$name.'"),';
			}
			echo trim($string, ', ');
			?>
		);
		<?php

	}//end print_plugin_vars()


	/**
	* Populate the select boxes with the options stored in the WYSIWYGs vars (set ealier)
	*
	* @return void
	* @access public
	*/
	function print_plugin_create_select()
	{
		?>
		case "addkeyword":
			options = Array();
			options["-- select keyword --"] = "";
			for (var i in editor.config.keywords) {
				var keyword = editor.config.keywords[i];
				options[keyword[1]] = keyword[0];
			}
			cmd = txt;
		break;
		<?php

	}//end print_plugin_create_select()


	/**
	* When the select box option is changed
	*
	* @return void
	* @access public
	*/
	function print_plugin_combo_selected()
	{
		?>
		case "addkeyword":
			if (value != "") {
				this.insertHTML(value);
				el.selectedIndex = 0;
			}
		break;
		<?php

	}//end print_plugin_combo_selected()


}//end class()

?>
