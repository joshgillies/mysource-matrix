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
* $Id: keyword_replace.inc,v 1.1 2003/12/08 03:06:13 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class Keyword_Replace extends WYSIWYG_Plugin {

	/**
	* Some text to put before and after the keyword
	*
	* @var string
	*/
	var $keyword_tag = '';

	/**
	* A format to define how the placeholder content should look
	* Must contain keyword_replace="%KEYWORD%" and %DEFAULT% and must be a span element
	*
	* @var string
	*/
	var $content_format = '<span contenteditable="false" keyword_replace="%KEYWORD%">%DEFAULT%</span>';

	/**
	* The keywords and their replacements
	* This array must be in form
	* <pre>
	* Array('keyword' => Array(
	*						'name' => short name,
	*						'default' => HTML to shwo to the user
	*							)
	*		)
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

	}//end contructor


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
			foreach ($this->keywords as $keyword => $info) {
				$default = preg_replace('|<p([^>]+)?>(.+?)</p>|is', '\\2', $info['default']);

				$default_string = str_replace('%KEYWORD%', $keyword, $this->content_format);
				$default_string = str_replace('%DEFAULT%', $default, $default_string);
				$string .= "\n".'Array("'.str_replace('"', '\"', $keyword).'", "'.str_replace('"', '\"', $info['name']).'", "'.str_replace('"', '\"', $default_string).'"),';
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
				options[keyword[1]] = keyword[2];
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


	/**
	* Replaces the default values with the real keywords
	*
	* @param string	&html	the HTML that was submitted by the editor
	*
	* @return string
	* @access public
	*/
	function process(&$html)
	{
		$html = preg_replace('|<span[^>]+keyword_replace\="?([^"]+)"?([^>]+)?>(.+?)</span>|is', $this->keyword_tag.'\\1'.$this->keyword_tag, $html);

	}//end process()


	/**
	* Sets the initial contents of the WYSIWYG editor
	*
	* @param string	&$contents	Some HTML content
	*
	* @return void
	* @access public
	*/
	function set_contents(&$contents)
	{
		// replace the keywords with the special tags
		$search = Array();
		$replace = Array();
		$counter = 0;
		foreach ($this->keywords as $keyword => $info) {
			$default = preg_replace('|<p([^>]+)?>(.+?)</p>|is', '\\2', $info['default']);

			$default_string = str_replace('%KEYWORD%', $keyword, $this->content_format);
			$default_string = str_replace('%DEFAULT%', $default, $default_string);

			$search[$counter]  = "|{$this->keyword_tag}{$keyword}{$this->keyword_tag}|is";
			$replace[$counter] = $default_string;
			$counter++;
		}
		$contents = preg_replace($search, $replace, $contents);

	}//end set_contents()


}//end class()

?>