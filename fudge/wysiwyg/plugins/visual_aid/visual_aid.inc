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
* $Id: visual_aid.inc,v 1.4 2012/08/30 00:56:53 ewang Exp $
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
* @version $Revision: 1.4 $
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
		$this->_add_button('visual_aid','Visual_Aid', translate('Visual Aid'),'false','61');

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
			var visual_aid_image_path = "<?php echo sq_web_path('fudge') ?>/wysiwyg/plugins/visual_aid/images";
			var visual_aid_mode = false;

			HTMLArea.prototype._visualAidToggle = function(target, change_mode) {
				var editor = this;

				if (target.length == 1 && target[0] == "all") {
					var tag_types = visual_aid_types;
				} else {
					var tag_types = target;
				}

				// html elements with the start and end tags
				var elements = Array();
				if (HTMLArea.is_gecko) {
					for (var i = 0; i < tag_types.length; i++) {
						elements[tag_types[i]] = this._doc.getElementsByTagName(tag_types[i]);
					}
				} else if (HTMLArea.is_ie) {
					for (var i = 0; i < tag_types.length; i++) {
						elements[tag_types[i]] = this._docContent.getElementsByTagName(tag_types[i]);
					}
				}
				for (var tag in elements) {
					if (elements[tag].length > 0) {
						for (var j = 0; j < elements[tag].length; j++) {
						//for (var j = elements[tag].length - 1; j >= 0; j--) {
							var obj = new visual_aid_factory(tag, elements[tag][j], editor);
							if (visual_aid_mode) {
								obj.turnOff();
							} else {
								obj.turnOn();
							}
						}
					}
				}
				if (change_mode) {
					visual_aid_mode = !visual_aid_mode;
				}
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
			var target = new Array("all");
			this._visualAidToggle(target, true);
		break;
		<?php

	}//end print_plugin_button_click()


	/**
	* Prints any JS the plugin wants executed before getHTML runs
	* This code is a function - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_pre_get_html()
	{
		?>
		if (visual_aid_mode) {
			var target = new Array("all");
			this._visualAidToggle(target, true);
		}
		<?php

	}//end print_plugin_pre_get_html()


	/**
	* Shortcut keys for visual aid
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts()
	{
		?>
		case 'm':     // Toggle Visual Aid
			var target = new Array("all");
			this._visualAidToggle(target, true);
			HTMLArea._stopEvent(ev);
		break;
		<?php

	}//end print_plugin_shortcuts()


}//end class()

?>
