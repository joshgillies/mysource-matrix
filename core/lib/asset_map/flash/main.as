
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


#include "test.as"
#include "functions.as"
#include "general.as"
#include "nestedMouseMovieClip.as"
//#include "mcNestedMouseMovieClip.as"
#include "stageResize.as"
#include "systemStatus.as"
#include "externalCall.as"
#include "serverExec.as"
#include "assetType.as"
#include "assetLink.as"
#include "asset.as"
#include "assetManager.as"
#include "mcMenuContainerClass.as"
#include "mcMenuItemClass.as"
#include "mcListItemPlusMinus.as"
#include "mcListContainerClass.as"
#include "mcListItemContainerClass.as"
#include "mcListItemClass.as"
#include "mcMsgsBarClass.as"
#include "mcMsgsBarMessageClass.as"
#include "mcActionsBarClass.as"
#include "mcActionsBarButtonClass.as"
#include "mcOptionsBoxClass.as"
#include "mcDialogBoxClass.as"
#include "mcProgressBarClass.as"

  ///////////////////////////////////////////////////
 // CONSTANTS                                     //
///////////////////////////////////////////////////

// the height of the messages bar at the bottom of the screen
_root.MSG_BAR_HEIGHT = 130;
// the width of a scroll page
_root.SCROLL_PANE_SCROLL_WIDTH = 16;

// this indent needs to set to the offset of the text box in a list item
// from the LHS of a list items background
_root.LIST_ITEM_INDENT_SPACE   = 20;
_root.LIST_ITEM_POS_INCREMENT  = 20;
// the vertical gap between the last item in a branch and the next item in it's parent's branch
_root.LIST_ITEM_END_BRANCH_GAP = 10;  
// the colours for the background of a list item
_root.LIST_ITEM_BG_COLOURS = {
							normal:   {colour: 0xFFFFFF, alpha: 0},   // alpha = 0 -> transparent
							selected: {colour: 0x999999, alpha: 100}
							};  

  ///////////////////////////////////////////////////
 // ALL INITIALISATION STUFF                      //
///////////////////////////////////////////////////

/* for testing from the Flash IDE */
if (_root.server_exec_path == undefined) {
	_root.server_exec_path = "http://beta.squiz.net/blair_resolve/_edit/?SQ_BACKEND_PAGE=asset_map_request";
}
if (_root.action_bar_path == undefined) {
	_root.action_bar_path = "http://beta.squiz.net/blair_resolve/_edit/?SQ_BACKEND_PAGE=main&assetid=%assetid%&action=%action%";
}
if (_root.action_bar_frame == undefined) {
	_root.action_bar_frame = "main";
}




_root.system_events = new SystemEvents();


// Add the dialog box
_root.attachMovie("mcDialogBoxID", "dialog_box", 21);
_root.dialog_box.hide();

// Add the progress bar
_root.attachMovie("mcProgressBarID", "progress_bar", 20);
_root.progress_bar.hide();



_root.server_exec = new ServerExec(_root.server_exec_path);

_root.asset_manager = new AssetManager();


// Now attach the menu
_root.attachMovie("mcMenuContainerID", "menu_container", 2);
// Now the list container
_root.attachMovie("mcListContainerID", "list_container", 1);
// Now the messagess bar
_root.attachMovie("mcMsgsBarID", "msgs_bar", 3);

// Now the dialog options box
_root.attachMovie("mcOptionsBoxID", "options_box", 22);


// Initialise stage resize listener
_root.state_resize = new StageResize();
