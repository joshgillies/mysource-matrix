<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: matrix_apply_style.inc,v 1.1 2003/11/07 01:38:52 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');


/**
* WYSIWYG Plugin - Matrix Apply Style
*
* Purpose
*     A WYSIWYG plugin to apply a style from the effective design
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class Matrix_Apply_Style extends WYSIWYG_Plugin {

	/**
	* A list of classess to show in the drop down
	*
	* @var array
	*/
	var $_css_classes = Array();


	/**
	* Constructor
	*
	* @access public
	*/
	function Matrix_Apply_Style(&$wysiwyg)
	{
		  ///////////////////////////////
		 //  WORK OUT THE STLYE LIST  //
		///////////////////////////////
		require_once dirname(__FILE__).'/../../../../core/include/init.inc';

		if (isset($GLOBALS['sq_bodycopy_cssids'])) {
			foreach ($GLOBALS['sq_bodycopy_cssids'] as $css_assetid) {
				$css = &$GLOBALS['SQ_SYSTEM']->am->getAsset($css_assetid);
				$css_file = $css->_getExistingFile();
				$stylesheet_src = $css_file['path'];

				// set the stylesheet to use in the editor
				$wysiwyg->set_stylesheet($css->getURL());

				// if it exists, parse the stylesheet to get a list of styles we can use
				if (file_exists($stylesheet_src)) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					$stylesheet = preg_replace('%[\r\n]+%',' ',file_to_string($stylesheet_src));
					
					$matches = Array();
					preg_match_all("%[\s]+([^\{]+)\{[^\}]+\}%", $stylesheet, $matches);
					$class_strings = $matches[1];

					// $class_strings is now a list of all the class and their variations
					// in the stylesheet - so we need to parse this again to get unique
					// class names without variations
					foreach ($class_strings as $class_string) {
						$variations = explode(',', $class_string);
						foreach ($variations as $class) {
							// non-standard class begin with a '.'
							list($class) = explode(':',trim($class));
							if (substr($class,0,1) == '.') $this->_css_classes[trim($class,'.')] = trim($class,'.');
						}
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($css);
				unset($css);
			}
		}

		$this->_add_button('formatblock','','','','');
		if (!empty($this->_css_classes)) $this->_add_button('applystyle','','','','');

	}//end contructor


	/**
	* Set the options for the select boxes as vars of the object
	*
	* @return void
	* @access public
	*/
	function print_plugin_vars()
	{
		?>
		this.config.applystyle = {
		<?php

		if (!empty($this->_css_classes)) {
			?>"-- select css style --": "",<?php
			$string = '';
			foreach ($this->_css_classes as $class_name => $cn) echo '"     '.$cn.'": "'.$class_name.'",';
			?>"-- remove css style --": "no_style"<?php
		}

		?>
		};

		this.config.formatblock = {
			"Heading 1": "h1",
			"Heading 2": "h2",
			"Heading 3": "h3",
			"Heading 4": "h4",
			"Heading 5": "h5",
			"Heading 6": "h6",
			"Normal"   : "p",
			"Address"  : "address",
			"Formatted": "pre"
		};
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
		case "applystyle":
		case "formatblock":
			options = editor.config[txt];
			cmd = txt;
		break;
		<?php

	}//end print_plugin_create_select()


	/**
	* When the toolbar is updated, change the selected option to reflect the selected text
	*
	* @return void
	* @access public
	*/
	function print_plugin_update_toolbar()
	{
		?>
		case "applystyle":
			if (!text) {
				var options = this.config["applystyle"];
				var current_span = this.getClosest("span");
				var found = false;
				if (current_span) {
					var value = current_span.className;
					var k = 0;
					for (var j in options) {
						if (options[j].toLowerCase() == value) {
							btn.element.selectedIndex = k;
							found = true;
							break;
						}
						++k;
					}
				}
				if (!found) { btn.element.selectedIndex = 0; }
			}
		break;

		case "formatblock":
			if (!text) {
				if (!doc) break;
				var value = ("" + doc.queryCommandValue(cmd)).toLowerCase();
				if (!value) {
					// FIXME: what do we do here?
					break;
				}
				btn.element.selectedIndex = 0;
				var options = this.config["formatblock"]; // HACK!!
				var k = 0;
				for (var j in options) {
					// FIXME: the following line is scary.
					if ((j.toLowerCase() == value) ||
						(options[j].substr(0, value.length).toLowerCase() == value)) {
						btn.element.selectedIndex = k;
						break;
					}
					++k;
				}
			}
		break;
		<?php

	}//end print_plugin_update_toolbar()


	/**
	* When the select box option is changed, apply changes to selected HTML
	*
	* @return void
	* @access public
	*/
	function print_plugin_combo_selected()
	{
		?>
		case "applystyle":
			if (value != "") {
				var current_span = this.getClosest("span");
				if (current_span) {
					this.selectNodeContents(current_span);
					current_span.className = value;
				} else if (value != "no_style") {
					// if the value is not the no_style default, add the span tags
					// to apply the stlye to the selected text - otherwise we ignore it
					this.surroundHTML("<span class=\"" + value + "\">", "</span>");
				}
			}
		break;

		case "formatblock":
			if (HTMLArea.is_ie) { // sad but true
				value = "<" + value + ">";
			}
			this._execCommand(txt, false, value);
		break;
		<?php
	
	}//end print_plugin_combo_selected()


	/**
	* Removes all applied styles that are blank
	*
	* @param string	&html	the HTML that was submitted by the editor
	*
	* @return string
	* @access public
	*/
	function process(&$html)
	{
		$html = preg_replace('|<span[^>]+class\="?no_style"?([^>]+)?>([^<]+)?</span>|i', '\\2', $html);

	}//end process();


}//end class()

?>