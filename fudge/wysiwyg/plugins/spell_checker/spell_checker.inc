<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: spell_checker.inc,v 1.1 2003/10/08 22:36:32 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Spell Checker
*
* Purpose
*     A WYSIWYG plugin to allow the user to Check the spelling of the document
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package wysiwyg
*/
class spell_checker extends wysiwyg_plugin {

	/**
	* Constructor
	* Adds the spell check button to this plugin
	*
	* @access public
	*/
	function spell_checker(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('spellchecker','spellchecker','Check Spelling','false','1');

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
		<script type="text/javascript" language="Javascript">
			// Called when the user clicks on "InsertImage" button
			HTMLArea.prototype._spellChecker = function() {
				var editor = this; // for nested functions

				// set focus in case we havnt selected anything
				if (HTMLArea.is_ie) {
					this._docContent.focus();
				} else if (HTMLArea.ie_gecko) {
					this.focusEditor();
				}
				
				// disable the toolbar
				this._tmp['disable_toolbar'] = true;
				this.updateToolbar(false);
				
				editor_<?php echo $this->wysiwyg->name?>._object = this;

				this._popupDialog("spellChecker", "<?php echo $this->get_popup_href('spell_checker_popup.php')?>?editor_name=<?php echo $this->wysiwyg->name?>", 600, 400, false, function(param) {
					if (!param) { 
						// user must have pressed Cancel
						return false;
					}

					editor.setHTML(param["html"]);

				}, null);
			};
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
		case "spellchecker":
			this._spellChecker();
			return;
		break;
		<?php

	}//end print_plugin_button_click()
}

?>