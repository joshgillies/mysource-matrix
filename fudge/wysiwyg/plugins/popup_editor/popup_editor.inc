<?php

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
* @package Fudge::wysiwyg
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
	function popup_editor(&$wysiwyg) {
		wysiwyg_plugin::wysiwyg_plugin($wysiwyg);
		$this->_add_button('popupeditor','popupeditor','Enlarge Editor','true','5');
	}


	/**
	* Open the editor in a popup when the button is clicked
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click() {
		?>
		case "popupeditor":
			eval("this._popupEditor" + this._uniqueID + "();");
		break;
		<?php
	}


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
	function paint() {

		$query_string  = '?editor_name='.$this->wysiwyg->name;
		$query_string .= '&editor_web_path='.$this->wysiwyg->web_path;
		$query_string .= '&editor_plugins=';
		foreach ($this->wysiwyg->plugins as $name => $obj) {
			if ($name == get_class($this)) continue;
			$query_string .= $name.'|';
		}
		$query_string = trim($query_string,' |');
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
	}
}

?>