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
* $Id: spell_checker.inc,v 1.7 2006/03/21 22:28:56 rong Exp $
*
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
* @version $Revision: 1.7 $
* @package Fudge
* @subpackage wysiwyg
*/
class spell_checker extends wysiwyg_plugin
{

	/**
	* Constructor
	* Adds the spell check button to this plugin
	*
	* @access public
	*/
	function spell_checker(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('spellchecker','spellchecker','Check Spelling','false','54');

	}//end constructor


	/**
	* Adds a printable toolbar button to this plugin
	*
	* @param string $id			A unique ID for this button
	* @param string $cmd		The command to run when the button is clicked
	*							This command will be used to switch on action of
	*							a pressed button to determine the functionality
	* @param string $tooltip	The tooltip for this button (appears on hover)
	* @param string $textmode	Should this button be available in text mode?
	*							Value is either 'true' or 'false' (with quotes)
	* @param string $position	The position of the button in the toolbar image
	*							This is zero indexed (first button is id zero)
	* @param string $context	Enable this button if outside this element
	*
	* @return void
	* @access private
	*/
	function _add_button($id, $cmd, $tooltip, $textmode, $position, $context='')
	{
		// checks for the required PEAR package before adding the plugin to the editor
		$output = Array();
		exec('pear list | grep "\bXML_HTMLSax\b"', $output);
		if (!empty($output)) {
			parent::_add_button($id, $cmd, $tooltip, $textmode, $position, $context='');
		} else {
			trigger_localised_error('CORE0252', E_USER_WARNING, '"Spell Checker"', 'Required PEAR Package "XML_HTMLSax" not installed');
		}

	}//end _add_button()


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