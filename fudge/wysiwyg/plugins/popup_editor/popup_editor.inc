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
* $Id: popup_editor.inc,v 1.10 2003/11/26 00:51:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Popup Editor
*
* Purpose
*     A WYSIWYG plugin to replicate the WYSIWYG
*     in a popup window for full screen editing
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class popup_editor extends wysiwyg_plugin {


	/**
	* Constructor
	* Add popup editor button to the plugin
	* Also needs reference to the WYSIWYG object, so calls
	* wysiwyg_plugin constructor
	*
	* @returns object Popup_Editor
	* @access  public
	*/
	function popup_editor(&$wysiwyg)
	{
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('popupeditor','popupeditor','Enlarge Editor','true','5');

	}//end constructor


	/**
	* Open the editor in a popup when the button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "popupeditor":
			eval("this._popupEditor" + this._uniqueID + "();");
		break;
		<?php

	}//end print_plugin_button_click()


	/**
	* Print a function to open the popup editor.
	* This function is created and printed for each WYSIWYG on
	* the page and is suffixed with the unique edotr ID.
	* Currenty istalled plugins are passed to the new window, as
	* is a reference to the WYSIWYG that opended the window.
	* This plugin will sync the two editors every second or so.
	*
	* @returns void
	* @access  public
	*/
	function paint()
	{
		$query_string  = '?editor_name='.$this->wysiwyg->name;
		$query_string .= '&editor_web_path='.rawurlencode('../../');
		$query_string .= '&editor_php_self='.rawurlencode($_SERVER['PHP_SELF']);
		$query_string .= '&editor_plugins=';
		foreach ($this->wysiwyg->_loaded_plugins as $name => $obj) {
			if ($name == get_class($this)) continue;
			$query_string .= $name.'|';
		}
		$query_string = trim($query_string,' |');
		$query_string .= '&rhc='.rawurlencode(serialize($this->wysiwyg->relative_href_checks));
		$query_string .= '&auc='.rawurlencode(serialize($this->wysiwyg->absolute_url_checks));
		?>
		<script type="text/javascript" language="Javascript">
			// Called when the user clicks the Insert Table button
			HTMLArea.prototype._popupEditor<?php echo $this->wysiwyg->name?> = function() {
				// pass this object to the newly opened window
				editor_<?php echo $this->wysiwyg->name?>._object = this;
				window.open(this.pluginURL("<?php echo $this->get_popup_href('fullscreen.php')?><?php echo $query_string?>"), "ha_fullscreen",
						"toolbar=no,location=no,directories=no,status=yes,menubar=no," +
						"scrollbars=no,resizable=yes,width=640,height=480");
			}
		</script>
		<?php
	}//end paint()

}//end class

?>