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
* $Id: stageResize.as,v 1.13 2003/11/26 00:51:12 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* This listener looks after the resizing of the objects
* on the stage when it is resized
*/

function StageResize()
{
	// Listen to the stage for resize calls
	Stage.addListener(this);
	// Listen to the msgs bar for resize calls
	//_root.msgs_bar.addListener(this);

	this.onResize();

}// end stageResize

StageResize.prototype.onResize = function()
{
	_root.tabs.setSize(Stage.width, Stage.height);
	_root.header.refresh();

//	var menu_height = 20;
//	var scroller_height = Stage.height - menu_height - _root.msgs_bar.height();
//
//	_root.scroller.setSize(Stage.width, scroller_height);
//	_root.msgs_bar._y = scroller_height + menu_height;
//	_root.msgs_bar.setWidth(Stage.width);
//
//	_root.list_container.refresh();

}// end onResize()

///**
//* Event fired when the Msg Bar is opened
//*/
//StageResize.prototype.onMsgsBarOpen = function()
//{
//	this.onResize();
//}
//
///**
//* Event fired when the Msg Bar is closed
//*/
//StageResize.prototype.onMsgsBarClose = function()
//{
//	this.onResize();
//}
