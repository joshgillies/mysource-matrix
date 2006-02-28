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
* $Id: visual_aid.inc,v 1.1 2006/02/28 06:19:00 skim Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');


/**
* WYSIWYG Plugin - Visual Aid
*
* Purpose
*     A WYSIWYG plugin to allow the user to have visual
*     representation of html tags
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1 $
* @package Fudge
* @subpackage wysiwyg
*/
class visual_aid extends WYSIWYG_Plugin
{


	/**
	* Constructor
	*
	* @access public
	*/
	function visual_aid(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('visual_aid','Visual_Aid','Visual Aid','false','61');

	}//end constructor


	/**
	* Prints a generic insert image function that can be used by any editor
	* A URL must be entered for the image you want to insert. Alt text must
	* also be entered before the image will be inserted.
	*
	* @returns void
	* @access  public
	*/
	function paint_generic()
	{
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/wysiwyg/plugins/visual_aid/visual-aid.js' ?>"></script>
		<script type="text/javascript" language="Javascript">
			var visual_aid_mode = false;
			HTMLArea.prototype._visualAidToggle = function() {
				var elements = Array();
				if (HTMLArea.is_gecko) {
					for (var i = 0; i < visual_aid_types.length; i++) {
						elements[visual_aid_types[i]] = this._doc.getElementsByTagName(visual_aid_types[i]);
					}
				} else if (HTMLArea.is_ie) {
					for (var i = 0; i < visual_aid_types.length; i++) {
						elements[visual_aid_types[i]] = this._docContent.getElementsByTagName(visual_aid_types[i]);
					}
				}
				for (var tag in elements) {
					if (elements[tag].length > 0) {
						for (var j = 0; j < elements[tag].length; j++) {
							var obj = new visual_aid_factory(tag, elements[tag][j]);
							if (visual_aid_mode) {
								obj.turnOff();
							} else {
								obj.turnOn();
							}
						}
					}
				}
				visual_aid_mode = !visual_aid_mode;
			}
		</script>
		<?php

	}//end paint_generic()


	/**
	* Call the insert image function when the button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "visual_aid":
			this._visualAidToggle();
		break;
		<?php

	}//end print_plugin_button_click()


	function print_plugin_pre_get_html()
	{
		?>
		if (visual_aid_mode) {
			this._visualAidToggle();
		}
		<?php

	}//end print_plugin_pre_get_html()


}//end class()

?>
