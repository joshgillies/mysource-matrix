/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: mcTabContentAreaClass.as,v 1.7 2003/11/18 15:37:35 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* TabContentArea
*
* This is the code for a single instance of a tab content area that is used by mcTabs
*
*/

// Create the Class
function mcTabContentAreaClass()
{
	this.stop();
	this._visible = false;
}

// Make it inherit from Nested Mouse Movements MovieClip
mcTabContentAreaClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);

/**
* Opens this tab in the tab container
*/
mcTabContentAreaClass.prototype.openTab = function()
{
	this._parent.setCurrentTab(this._name);
}// openTab()

/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
*/
mcTabContentAreaClass.prototype.setSize = function(w, h)
{
	set_background_box(this, w, h, this._parent.colours.selected.bg, 100);
}// setSize()

/**
* Fired when we are pressed and then release
*
* @access public
*/
mcTabContentAreaClass.prototype.onRelease = function() 
{
	if (super.onRelease()) return true;
	_root.system_events.screenPress(this);
	return true;
}// end

Object.registerClass("mcTabContentAreaID", mcTabContentAreaClass);
