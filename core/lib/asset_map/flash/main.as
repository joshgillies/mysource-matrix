
// Set this to make sure
Stage.scaleMode = "noScale";
Stage.align = "TL";


#include "test.as"
#include "stageResize.as"
#include "general.as"
#include "serverExec.as"
#include "asset.as"
#include "mcMenuContainerClass.as"
#include "mcMenuItemClass.as"
#include "mcListContainerClass.as"
#include "mcPlusMinus.as"
#include "mcListItemClass.as"

  ///////////////////////////////////////////////////
 // CONSTANTS                                     //
///////////////////////////////////////////////////

// this indent needs to set to the offset of the text box in a list item
// from the LHS of a list items background
_root.LIST_ITEM_INDENT_SPACE   = 20;
_root.LIST_ITEM_POS_INCREMENT  = 20;
// the vertical gap between the last item in a branch and the next item in it's parent's branch
_root.LIST_ITEM_END_BRANCH_GAP = 10;  

  ///////////////////////////////////////////////////
 // ALL INITIALISATION STUFF                      //
///////////////////////////////////////////////////
if (server_exec_path == undefined) {
	server_exec_path = "http://beta.squiz.net/blair_resolve/_edit/?SQ_BACKEND_PAGE=asset_map_request";
}
trace("Server Exec Path : " + server_exec_path);



// Initialise any pop-ups
_root.progress_bar._visible = false;
_root.progress_bar.swapDepths(20);
_root.progress_bar.stop();
_root.dialog_box._visible = false;
_root.dialog_box.swapDepths(21);
_root.dialog_box.stop();

server_exec = new serverExec(server_exec_path);

// Now attach the menu
_root.attachMovie("mcMenuContainerID", "menu_container", 2);
trace(_root.menu_container instanceof MovieClip);
_root.menu_container._x = 0;
_root.menu_container._y = 0;
_root.menu_container.create();


// Now we just attach the list container and get it to sort everything out
_root.attachMovie("mcListContainerID", "list_container", 1);

//Attach the container on to the "scroller"
_root.scroller.setScrollContent(_root.list_container);

// Load up the root entries
// create the root asset as we need to this to start somewhere
_root.list_container.assets['1'] = new Asset(1, 'root_folder', '/', true);
_root.list_container.showKids('1', 'li');

// Call resize to setup intial sizes of things
_root.stageResizeListener.onResize();

_root.external_call.registerCmd("reload_asset", _root.list_container, 'reloadAsset');

