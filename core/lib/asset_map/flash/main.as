

#include "test.as"
#include "general.as"
#include "asset.as"
#include "mcListContainerClass.as"
#include "mcPlusMinus.as"
#include "mcListItemClass.as"

  ///////////////////////////////////////////////////
 // CONSTANTS                                     //
///////////////////////////////////////////////////

// this indent needs to set to the offset of the text box in a list item
// from the LHS of a list items background
_root.LIST_ITEM_INDENT_SPACE   = 20;
_root.LIST_ITEM_POS_INCREMENTS = 20;
// the vertical gap between the last item in a branch and the next item in it's parent's branch
_root.LIST_ITEM_END_BRANCH_GAP = 10;  



  ///////////////////////////////////////////////////
 // ALL INITIALISATION STUFF                      //
///////////////////////////////////////////////////
asset_xml_load_path = "http://beta.squiz.net/blair_resolve/_edit/?SQ_BACKEND_PAGE=site_map_request&parent_assetid=";

// Initialise any pop-ups
_root.progress_bar._visible = false;
_root.progress_bar.swapDepths(20);
_root.progress_bar.stop();
_root.dialog_box._visible = false;
_root.dialog_box.swapDepths(21);
_root.dialog_box.stop();

assetXML = new XML();
assetXML.ignoreWhite = true;
assetXML.onLoad = assetXMLonLoad;

// Now we just attach the list container and get it to sort everything out
_root.attachMovie("mcListContainerID", "list_container", 1);

//Attach the container on to the "scroller"
_root.scroller.setScrollContent(_root.list_container);


// Load up the root entries
// create the root asset as we need to this to start somewhere
_root.list_container.assets['1'] = new Asset(1, 'root_folder', '/', true);
_root.list_container.showKids('1');
