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
* $Id: mcTabContentAreaLogMsgsClass.as,v 1.7 2003/11/18 15:37:35 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


#include "mcMsgsBarClass.as"

/**
* TabContentArea
*
* This is the code for a single instance of a tab content area that is used by mcTabs
*
*/

// Create the Class
function mcTabContentAreaLogMsgsClass()
{
	super();

	this.attachMovie('mcLogSubHeaderID', 'sub_header', 1);
	this.sub_header._y = 0;
	
	this.attachMovie("FScrollPaneSymbol", "scroll_pane", 2);
	this.scroll_pane.setHScroll(false);
	this.scroll_pane.setVScroll(true);
	this.scroll_pane._x = 0;
	this.scroll_pane._y = this._sub_header._y + this.sub_header._height;

	// Now the msgs container
	this.attachMovie("mcMsgsBarID", "msgs_container", 3);

	// Attach the container on to the scroll pane
	this.scroll_pane.setScrollContent(this.msgs_container);
	this.msgs_container._x = 0;
	this.msgs_container._y = this.sub_header._y + this.sub_header._height;

	// Because the scroll pane inherits from some other place 
	// we need to manually set it up for nesting
	makeNestedMouseMovieClip(this.scroll_pane, true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.hScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.vScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);

}// end constructor()

// Make it inherit from Tab Content Area
mcTabContentAreaLogMsgsClass.prototype = new mcTabContentAreaClass();

/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
*/
mcTabContentAreaLogMsgsClass.prototype.setSize = function(w, h)
{
	super.setSize(w, h);
	this.sub_header.back._width = w;
	this.scroll_pane.setSize(w, h - this.sub_header._height);
	this.msgs_container.refresh();

}// setSize()

/**
* Fired when we are pressed and then release
*
* @access public
*/
mcTabContentAreaLogMsgsClass.prototype.onRelease = function() 
{
	return super.onRelease(); // Fucking flash see SUPER_METHOD_EG.as
}// end


Object.registerClass("mcTabContentAreaLogMsgsID", mcTabContentAreaLogMsgsClass);
