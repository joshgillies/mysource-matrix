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
* $Id: keyword_replace.inc,v 1.2 2003/12/12 03:36:38 gsherwood Exp $
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
	* A format to define how the placeholder content should look for inline keywords
	* Inline keywords ensure any formatting applied to the text is kept (bold, underline etc)
	* Must contain keyword_replace="%KEYWORD%" and %DEFAULT% and must be a SPAN element
	*
	* @var string
	*/
	var $content_format_inline = '<span style="background-color: #C2BFC4; color: #555555;" contenteditable="false" keyword_replace="%KEYWORD%">%DEFAULT%</span>';

	/**
	* A format to define how the placeholder content should look for block level keywords
	* Block level keywords do not keep any formatting - straight keyword replacement
	* Must contain keyword_replace="%KEYWORD%" and %DEFAULT% and must be a DIV element
	*
	* @var string
	*/
	var $content_format_block = '<div style="width: 100%; height: 200px; background-color: #F4F4EB; text-align: center; border: 1px solid #5A5A5A;" contenteditable="false" keyword_replace="%KEYWORD%"><table height="200px" align="center" contenteditable="false"><tr><td align="center" valign="middle"><b>%DEFAULT%</b></td></tr></table></div>';

	/**
	* A format to define how the placeholder content should look for special inline keywords
	* Special keywords do not keep any formatting - straight keyword replacement - but act as a SPAN
	* Must contain keyword_replace="%KEYWORD%" and %DEFAULT% and must be a SPAN element
	*
	* @var string
	*/
	var $content_format_special = '<span style="background-color: #C2BFC4; color: #555555;" contenteditable="false" keyword_replace="%KEYWORD%">%DEFAULT%</span>';

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
				if (!isset($info['type']) || $info['type'] == 'inline') {
					$default = preg_replace('|<p([^>]+)?>(.+?)</p>|is', '\\2', $info['default']);
					$default = '~ '.$default.' ~';
					$default_string = $this->content_format_inline;
				} else if ($info['type'] == 'block') {
					$default = $info['default'];
					$default_string = $this->content_format_block;
				} else {
					$default = preg_replace('|<p([^>]+)?>(.+?)</p>|is', '\\2', $info['default']);
					$default_string = $this->content_format_special;
				}
				$default_string = str_replace('%KEYWORD%', $keyword, $default_string);
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
		foreach ($this->keywords as $keyword => $info) {
			if (!isset($info['type']) || $info['type'] == 'inline') {
				// inline (SPAN)
				// we make sure any formatting made to the text is kept
				$default = preg_replace('|<p([^>]+)?>(.+?)</p>|is', '\\2', $info['default']);
				$default = preg_replace('|([\(\)\[\]\^\$\*\.\?])|', '\\\\\1', $default);
				$default = '~ '.$default.' ~';

				$pattern = '|<span[^>]+keyword_replace\="?'.$keyword.'"?([^>]+)?>([^~]+)??'.$default.'([^~]+)??</span>|is';
				$replace = $this->keyword_tag.'\\2'.$keyword.'\\3'.$this->keyword_tag;
			} else if ($info['type'] == 'block') {
				// block level (DIV)
				// ignore all formatting - this is a straight keyword substitution
				$pattern = '|<div[^>]+keyword_replace\="?'.$keyword.'"?([^>]+)?>(.+?)</div>|is';
				$replace = $this->keyword_tag.$keyword.$this->keyword_tag;
			} else {
				// special type of replacements (SPAN)
				// ignore all formatting - this is a straight keyword substitution
				$pattern = '|<span[^>]+keyword_replace\="?'.$keyword.'"?([^>]+)?>([^~]+?)</span>|is';
				$replace = $this->keyword_tag.$keyword.$this->keyword_tag;
			}
			$html = preg_replace($pattern, $replace, $html);
		}
		
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
			if (!isset($info['type']) || $info['type'] == 'inline') {
				$default = preg_replace('|<p([^>]+)?>(.+?)</p>|is', '\\2', $info['default']);
				$default = '~ '.$default.' ~';
				$default_string = $this->content_format_inline;
			} else if ($info['type'] == 'block') {
				$default = $info['default'];
				$default_string = $this->content_format_block;
			} else {
				$default = preg_replace('|<p([^>]+)?>(.+?)</p>|is', '\\2', $info['default']);
				$default_string = $this->content_format_special;
			}
			$default_string = str_replace('%KEYWORD%', $keyword, $default_string);
			$default_string = str_replace('%DEFAULT%', '\\1'.$default.'\\4', $default_string);

			$search[$counter]  = "|{$this->keyword_tag}((<([^>]+?)>)+)?{$keyword}((<([^>]+?)>)+)?{$this->keyword_tag}|is";
			$replace[$counter] = $default_string;
			$counter++;
		}
		$contents = preg_replace($search, $replace, $contents);

	}//end set_contents()


}//end class()

?>