<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: snippet_keyword_replace.inc,v 1.2 2012/08/30 00:56:52 ewang Exp $
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
* @version $Revision: 1.2 $
* @package Fudge
* @subpackage wysiwyg
*/
class Snippet_Keyword_Replace extends WYSIWYG_Plugin
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
	function Snippet_Keyword_Replace(&$wysiwyg)
	{
		$this->_add_button('addsnippetkeyword','','','','');
		$this->_add_button('snippethelp','snippethelp',translate('Snippet Keyword Information'),'true','0');

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
		this.config.snippetkeywords = Array(
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
		case "addsnippetkeyword":
			options = Array();
			options["-- select snippet keyword --"] = "";
			for (var i in editor.config.snippetkeywords) {
				var keyword = editor.config.snippetkeywords[i];
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
		case "addsnippetkeyword":
			if (value != "") {
				this.insertHTML(value);
				el.selectedIndex = 0;
			}
		break;
		<?php

	}//end print_plugin_combo_selected()


	/**
	* Build and show a list of snippet description
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_click()
	{

		?>
		case "snippethelp":
			strPage = "<?php echo $this->get_popup_href('snippet_info_popup.php'); ?>";
			this._popupDialog("snippetInfo", strPage, 790, 500, true, function() {}, 0);
		break;
		<?php

	}//end print_plugin_button_click()


}//end class()
?>
