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
* $Id: mcTabsClass.as,v 1.12 2003/11/26 00:51:12 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Tabs
*
* This is the container for looking after all the tabs
*
*/

#include "mcTabButtonClass.as"
#include "mcTabContentAreaClass.as"

// Create the Class
function mcTabsClass()
{
	this.tabs = new Array();  // name of all the tabs that we have
	this.current_tab = null;  // name of the contents area of the current tab

	this.tab_height = null;   // set when the first tab is added
	this.tab_spacing = 2;

	this.dims = {w: 0, h: 0};

	this._x = 0;
	this._y = 0;

}

// Make it inherit from Nested Mouse Movements MovieClip
mcTabsClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);

/**
* Creates a tab using the passed export name and set's it to the passed name
*
* @param string	tab_type	the export name of the tab to add (eg mcTabAssetTreeID)
* @param string	name		the code accesable name under this object to assign the new tab too
* @param string	label		the label to show on the tab
*
* @public
*/
mcTabsClass.prototype.addTab = function(tab_type, name, label, iconID)
{
	// bugger off if we already have a tab by this name
	if (this.tabs.search(name) != null) return;

	var depth = (this.tabs.length * 2);
	this.attachMovie(tab_type, name, depth + 1);

	this.attachMovie("mcTabButtonID", "tab_button_" + name, depth + 2);

	this["tab_button_" + name].setLabel(label);
	this["tab_button_" + name].setIcon(iconID);
	this["tab_button_" + name]._visible = true;
	this["tab_button_" + name]._y = 0;

	if (this.tabs.length > 0) {
		var last_button_name = "tab_button_" + this.tabs[this.tabs.length - 1];
		// + 1 is for black line
		this["tab_button_" + name]._x = this[last_button_name]._x + this[last_button_name]._width + this.tab_spacing;
	} else {
		this["tab_button_" + name]._x = 0; 
	}

	this.tabs.push(name);

	this[name]._x = 0;
	this[name]._y = this._parent.header._y  + this._parent.header._height;

	if (this.current_tab == null) 
		this.setCurrentTab(name);
	if (this.tab_height  == null) 
		this.tab_height = this["tab_button_" + name]._height;

	this.refresh();

}// addTab();

/**
* Set's the current tab being displayed
*
* @param string	name	the name of the tab
* 
* @access private
*/
mcTabsClass.prototype.setCurrentTab = function(name)
{
	if (this.current_tab == name) return;
	
	// bugger off if we don't know about this tab
	if (this.tabs.search(name) == null) return;
	
	if (this.current_tab != null) {
		this["tab_button_" + this.current_tab].unselect();
		this[this.current_tab]._visible = false;
	}

	this["tab_button_" + name].select();
	this[name]._visible = true;
	this.current_tab = name;

	this.refreshTab();

}// _setCurrentTab()

/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
* @access public
*/
mcTabsClass.prototype.setSize = function(w, h)
{
//	trace (this + "::mcTabsClass.setSize (" + w + ", " + h + ")");

	if (this.tabs.length == 0) return;
	this.dims.w = w;
	this.dims.h = h;

	this.refreshTab();

}// setSize()


/**
* Refreshes the current tabs display
*
* @access public
*/
mcTabsClass.prototype.refreshTab = function()
{
	if (this.current_tab == null) return;

	// then just set the size of current tab contents area
	this[this.current_tab].setSize(this.dims.w, this.dims.h - (this._parent.header._y + this._parent.header._height));


}// refreshTab()

mcTabsClass.prototype.refresh = function()
{
	if (this.tabs.length == 0) {
		this._parent.header._y = 0;
	} else {
		this._parent.header._y = this["tab_button_" + this.tabs[0]]._height + this.tab_spacing - 1;
	}

	for (var i = 0; i < this.tabs.length; i++) {
		var nextTabArea = this[this.tabs[i]];

		nextTabArea._y = this._parent.header._y + this._parent.header._height;
	}
}

Object.registerClass("mcTabsID", mcTabsClass);
