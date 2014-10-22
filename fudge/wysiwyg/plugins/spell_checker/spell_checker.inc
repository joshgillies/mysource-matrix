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
* $Id: spell_checker.inc,v 1.13 2012/08/30 00:56:53 ewang Exp $
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
* @version $Revision: 1.13 $
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
		if (function_exists('pspell_suggest')) {
			$this->_add_button('spellchecker','spellchecker',translate('Check Spelling'),'false','54');
		}

	}//end constructor


	/**
	* Adds a printable toolbar button to this plugin
	*
	* @param string	$id			A unique ID for this button
	* @param string	$cmd		The command to run when the button is clicked
	*							This command will be used to switch on action of
	*							a pressed button to determine the functionality
	* @param string	$tooltip	The tooltip for this button (appears on hover)
	* @param string	$textmode	Should this button be available in text mode?
	*							Value is either 'true' or 'false' (with quotes)
	* @param string	$position	The position of the button in the toolbar image
	*							This is zero indexed (first button is id zero)
	* @param string	$context	Enable this button if outside this element
	*
	* @return void
	* @access private
	*/
	function _add_button($id, $cmd, $tooltip, $textmode, $position, $context='')
	{
		// checks for the required PEAR package before adding the plugin to the editor
		// use include_once so that it will still run if the package is not installed
		@include_once('XML/XML_HTMLSax.php');

		if (defined('XML_HTMLSAX')) {
			parent::_add_button($id, $cmd, $tooltip, $textmode, $position, $context='');
		} else {
			trigger_localised_error(
				'CORE0252',
				sprintf(
					translate('Unable to load "%s" WYSIWYG Plugin - Required PEAR package "%s" not installed.'),
					translate('Spell Checker'),
					'XML_HTMLSax'
				),
			E_USER_WARNING);		
		}

	}//end _add_button()


	/**
	* Prints a generic insert image function that can be used by any editor
	* A URL must be entered for the image you want to insert. Alt text must
	* also be entered before the image will be inserted.
	*
	* @return void
	* @access public
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

				var popup_height = 410;

				// A bit more breathing room for Safari
				if (HTMLArea.is_safari) popup_height += 15;

				this._popupDialog("spellChecker", "<?php echo $this->get_popup_href('spell_checker_popup.php')?>?editor_name=<?php echo $this->wysiwyg->name?>", 605, popup_height, false, function(param) {
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
	* @return void
	* @access public
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


	/**
	* Check whether this plugin is ready to be used.
	*
	* Return a TRUE if ok or a error message.
	*
	* @return mixed string|boolean
	* @access public
	* @static
	*/
	public static function check_usable()
	{
		if (function_exists('pspell_suggest')) return TRUE;
		return translate('Pspell is not installed');

	}//end check_usable()


}//end class

?>
