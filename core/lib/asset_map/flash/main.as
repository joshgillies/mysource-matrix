
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

// the height of the messages bar at the bottom of the screen
_root.MSG_BAR_HEIGHT = 130;

// this indent needs to set to the offset of the text box in a list item
// from the LHS of a list items background
_root.LIST_ITEM_INDENT_SPACE   = 20;
_root.LIST_ITEM_POS_INCREMENT  = 20;
// the vertical gap between the last item in a branch and the next item in it's parent's branch
_root.LIST_ITEM_END_BRANCH_GAP = 10;  
// the colours for the background of a list item
_root.LIST_ITEM_BG_COLOURS = {
							normal:   {colour: 0xFFFFFF, alpha: 0},   // alpha = 0 -> transparent
							selected: {colour: 0xc0c0c0, alpha: 100}
							};  
// the colours for the background of a mail box message
_root.MAIL_MSG_BG_COLOURS = {
							normal:   {colour: 0xFFFFFF, alpha: 0},   // alpha = 0 -> transparent
							selected: {colour: 0xc0c0c0, alpha: 100}
							};  

  ///////////////////////////////////////////////////
 // ALL INITIALISATION STUFF                      //
///////////////////////////////////////////////////

// for testing from the Flash IDE 
if (_root.server_exec_path == undefined) {
	_root.server_exec_path = "http://beta.squiz.net/blair/_edit/?SQ_BACKEND_PAGE=asset_map_request";
}
if (_root.url_frame == undefined) {
	_root.url_frame = "main";
}
if (_root.action_bar_path == undefined) {
	_root.action_bar_path = "http://beta.squiz.net/blair/_edit/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid=%assetid%&asset_ei_screen=%action%";
}
if (_root.duplicate_path == undefined) {
	_root.duplicate_path = "http://beta.squiz.net/blair/_edit/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=duplicate&assetid=%assetid%&to_parent_assetid=%to_parent_assetid%&to_parent_pos=%to_parent_pos%";
}
if (_root.inbox_path == undefined) {
	_root.inbox_path = "http://beta.squiz.net/blair/_edit/?SQ_BACKEND_PAGE=main&backend_section=am";
}


_root.system_events = new SystemEvents();

// Add the dialog box
_root.attachMovie("mcDialogBoxID", "dialog_box", 21);
_root.dialog_box.hide();

// Add the progress bar
_root.attachMovie("mcProgressBarID", "progress_bar", 20);
_root.progress_bar.hide();

// Now the dialog options box
_root.attachMovie("mcOptionsBoxID", "options_box", 22);

_root.server_exec = new ServerExec(_root.server_exec_path);

_root.asset_manager = new AssetManager();


// Now attach the Tabs
_root.attachMovie("mcTabsID", "tabs", 1);
_root.tabs.addTab("mcTabContentAreaTreeID", "tree",  "Tree");
_root.tabs.addTab("mcTabContentAreaMailBoxID", "mail",  "Mail Messages");
_root.tabs.addTab("mcTabContentAreaLogMsgsID", "log",  "Log Messages");

// Initialise stage resize listener
_root.stage_resize = new StageResize();

_root.asset_manager.init();
_root.tabs.mail.msgs_container.refreshMail();

