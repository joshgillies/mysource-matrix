<?php

/**
* WYSIWYG Plugin
*
* Purpose
*     A class used to create a plugin for the WYSIWYG
*     editor. Must bee added using the add_plugin function
*     of wysiwyg.inc
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class wysiwyg_plugin {

	var $buttons = Array();

	/**
	* A reference to the WYSIWYG object that created this plugin
	* @var object WYSIWYG
	*/
	var $wysiwyg = null;

	/**
	* Constructor
	*
	* @param   object wysiwyg $wysiwyg	A reference to the WYSIWYG that 
	*									this plugin is being created for
	* @returns object WYSIWYG_Plugin
	* @access  public
	*/
	function wysiwyg_plugin(&$wysiwyg) {
		$this->wysiwyg = $wysiwyg;
	}

	
	/**
	* Adds a printable toolbar button to this plugin
	*
	* @param string $id			A unique ID for this button
	* @param string $cmd		The command to run when the button is clicked
	*							This command will be used to switch on action of
	*							a pressed button to determine the functionality
	* @param string $tooltip	The tooltip for this button (appears on hover)
	* @param string $textmode	Should this button be availabel in text mode?
	*							Value is either 'true' or 'false' (with quotes)
	* @param string $position	The position of the button in the toolbar image
	*							This is zero indexed (first button is id zero)
	*
	* @returns void
	* @access  private
	*/
	function _add_button($id, $cmd, $tooltip, $textmode, $position) {
		$this->buttons[$id]['command']  = $cmd;
		$this->buttons[$id]['tooltip']  = $tooltip;
		$this->buttons[$id]['textmode'] = $textmode;
		$this->buttons[$id]['position'] = $position;
	}

	
	/**
	* Prints any JS variable needed by the WYSIWYG editor
	* This code is output in the HTMLArea construtor - do not use script tags
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_vars() {
		return;
	}


	/**
	* Prints shortcut keys for the buttons of this plugin
	* This code is a case statement - do not use script tags
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts() {
		return;
	}


	/**
	* Prints any JS code needed to get the options array for any select boxes in this plugin
	* This code is a case statement - do not use script tags
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_create_select() {
		return;
	}


	/**
	* Paints any JS functions needed by this plugin on a per WYSIWYG basis
	* This code is printed outside of script tags - print script tags yourself
	*
	* @returns void
	* @access  public
	*/
	function paint() {
		return;
	}


	/**
	* Paints any JS generic functions needed by this plugin
	* These functions can be used by all WYSIWYGs on the page
	* This code is printed outside of script tags - print script tags yourself
	*
	* @returns void
	* @access  public
	*/
	function paint_generic() {
		return;
	}


	/**
	* Prints JS executed when a button of this plugin is clicked
	* This code is a case statement - do not use script tags
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click() {
		return;
	}


	/**
	* Prints the JS executed on select for any select boxes in this plugin
	* This code is a case statement - do not use script tags
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_combo_selected() {
		return;
	}


	/**
	* Prints any JS the plugin wants executed before the WYSIWYG has its way
	* This code is a case statement - do not use script tags
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_exec_command() {
		return;
	}


	/**
	* Prints the JS executed by buttons of this plugin when the toolbar is updated
	* This code is a case statement - do not use script tags
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar() {
		return;
	}


	/**
	* Prints JS to create special types of buttons for this plugin
	* This code is a case statement - do not use script tags
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_type() {
		return;
	}


	/**
	* Prints any JS the plugin wants executed on returned HTML from getHTML
	* This code is a function - do not use script tags
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_get_html() {
		return;
	}


	/**
	* Returns a path to a file in this plugin's directory
	* This value can be passed to the _popupDialog function of the
	* WYSIWYG object
	*
	* @returns string
	* @access  public
	*/
	function get_popup_href($filename) {
		return get_class($this).'/'.$filename;
	}
}

?>