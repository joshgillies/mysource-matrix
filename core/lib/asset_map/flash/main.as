/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: main.as,v 1.52 2003/10/30 23:20:49 dwong Exp $
* $Name: not supported by cvs2svn $
*/

var minVersion = [6,0,40,0];
// Check versions


var versionStrings = this.$version.split(" ");
if (minVersion > versionStrings[1]) {
	_root.createTextField('error', 1, 0, 0, 50, 50);
	var textFormat = new TextFormat();
	textFormat.bold = true;
	textFormat.font = 'Arial';
	textFormat.color = 0xffffff;
	textFormat.align = 'center';

	_root.error.multiline = true;
	_root.error.wordWrap = true;
	_root.error.selectable = false;

	_root.error.setNewTextFormat(textFormat);

	_root.error.text = 'Flash Player Version ' + minVersion.join(',') + ' is required\n(your version: ' + versionNums.join(',') + ')';
	
	_root.error._width = Stage.width;
	_root.error._x = 0;
	_root.error._y = (Stage.height - _root.error._height) / 2;
	return;
}



// Set this to make sure
Stage.scaleMode = "noScale";
Stage.align = "TL";

/**************************************************
 * Additions to the ScrollPane class definition   *
 **************************************************/

/**
* Returns the width of the pane minus any vertical scroll bar
*/ 
FScrollPaneClass.prototype.getInnerPaneWidth = function()
{
	return this.hWidth;
}

/**
* Returns the height of the pane minus any horizontal scroll bar
*/ 
FScrollPaneClass.prototype.getInnerPaneHeight = function()
{
	return this.vHeight;
}

/**************************************************
 * Additions to the XML class definition          *
 **************************************************/

/**
* Make all the XML objects ignore white by default
*/ 
XML.prototype.ignoreWhite = true;

#include "test.as"
#include "functions.as"
#include "general.as"
#include "nestedMouseMovieClip.as"
#include "stageResize.as"
#include "systemEvents.as"
#include "externalCall.as"
#include "serverExec.as"
#include "assetManager.as"
#include "mcHeaderClass.as"
#include "mcToolBarClass.as"
#include "mcTabsClass.as"
#include "mcTabContentAreaTreeClass.as"
#include "mcTabContentAreaMailBoxClass.as"
#include "mcTabContentAreaLogMsgsClass.as"
#include "mcOptionsBoxClass.as"
#include "mcDialogBoxClass.as"
#include "mcProgressBarClass.as"


  ///////////////////////////////////////////////////
 // CONSTANTS                                     //
///////////////////////////////////////////////////

// minimum version of flash needed to run 

// the height of the messages bar at the bottom of the screen
_root.MSG_BAR_HEIGHT = 130;

// this indent needs to set to the offset of the text box in a list item
// from the LHS of a list items background
_root.LIST_ITEM_INDENT_SPACE   = 10;
_root.LIST_ITEM_POS_INCREMENT  = 20;
// the vertical gap between the last item in a branch and the next item in it's parent's branch
_root.LIST_ITEM_END_BRANCH_GAP = 5;  
// the colours for the background of a list item
_root.LIST_ITEM_BG_COLOURS = {
	archived:			{
		normal:			0xA59687,
		selected:		0x655240
	},
	under_construction: {
		normal:			0x78C7EB,
		selected:		0x00A0E2
	}, 
	pending_approval:	{
		normal:			0xAF9CC5,
		selected:		0x432C5F
	},
	approved:			{
		normal:			0xF4D425,
		selected:		0xEBB600
	},
	live:				{
		normal:			0xB1DC1B,
		selected:		0x92B41A
	},
	live_approval:		{
		normal:			0xAF9CC5,
		selected:		0x432C5F
	},
	editing:			{
		normal:			0xF25C86,
		selected:		0xB73E61
	},
	editing_approval:	{
		normal:			0xCCCCCC,
		selected:		0x666666
	},
	editing_approved:	{
		normal:			0xFF9A00,
		selected:		0xC96606
	}
}

// the colours for the background of a mail box message
_root.MAIL_MSG_BG_COLOURS = {
							normal:   {colour: 0xFFFFFF, alpha: 100},   // alpha = 0 -> transparent
							selected: {colour: 0x406080, alpha: 100}
							};  

  ///////////////////////////////////////////////////
 // ALL INITIALISATION STUFF                      //
///////////////////////////////////////////////////

// for testing from the Flash IDE 
if (_root.server_exec_path == undefined) {
	_root.server_exec_path = "http://beta.squiz.net/dom_resolvefx/_edit/?SQ_BACKEND_PAGE=asset_map_request";
}
if (_root.url_frame == undefined) {
	_root.url_frame = "main";
}
if (_root.action_bar_path == undefined) {
	_root.action_bar_path = "http://beta.squiz.net/dom_resolvefx/_edit/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid=%assetid%&sq_asset_path=%asset_path%&sq_link_path=%link_path%&asset_ei_screen=%action%";
}
if (_root.inbox_path == undefined) {
	_root.inbox_path = "http://beta.squiz.net/dom_resolvefx/_edit/?SQ_BACKEND_PAGE=main&backend_section=am";
}

_root.system_events = new SystemEvents();

// Add the dialog box
_root.attachMovie("mcDialogBoxID", "dialog_box", 21);
_root.dialog_box.hide();

// Now the dialog options box
_root.attachMovie("mcOptionsBoxID", "options_box", 22);

_root.server_exec = new ServerExec(_root.server_exec_path);

_root.asset_manager = new AssetManager();

// Attach the header
_root.attachMovie ("mcHeaderID", "header", 1);
Key.addListener(_root.header);

// Add the progress bar
_root.attachMovie("mcProgressBarID", "progress_bar", 20);
_root.progress_bar.init(_root.header.spinner, _root.header.loadingText);

// Now attach the Tabs
_root.attachMovie("mcTabsID", "tabs", 2);

_root.tabs.addTab("mcTabContentAreaTreeID",		"tree",		"Tree",			"mc_tree_tab_icon");
_root.tabs.addTab("mcTabContentAreaMailBoxID",	"mail",		"Messages",		"mc_messages_tab_icon");
_root.tabs.addTab("mcTabContentAreaLogMsgsID",	"log",		"Log",			"mc_log_tab_icon");
// Initialise stage resize listener	
_root.stage_resize = new StageResize();

_root.asset_manager.init();
_root.tabs.mail.msgs_container.refreshMail();

_root.tabs.setSize(Stage.width, Stage.height);
