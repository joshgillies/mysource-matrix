#include "mcAssetFinderHeading.as"

/**
* TabContentArea
*
* This is the code for a single instance of a tab content area that is used by mcTabs
*
*/

// Create the Class
function mcTabContentAreaTreeClass()
{
	super();

	// NOTE: the depth order for these MC's is important because the menu must be higner
	//       than the scroll pane and list container so that it's items will display 
	//       over the top of them

	// Now attach the menu
	this.attachMovie("mcMenuContainerID", "menu_container", 3);
	this.menu_container._x = 0;
	this.menu_container._y = 0;

	this.attachMovie("FScrollPaneSymbol", "scroll_pane", 1);
	this.scroll_pane.setHScroll(true);
	this.scroll_pane.setVScroll(true);
	this.scroll_pane._x = 0;
	this.scroll_pane._y = this.menu_container._height;

	// Now the list container
	this.attachMovie("mcListContainerID", "list_container", 2);

	// Attach the container on to the scroll pane
	this.scroll_pane.setScrollContent(this.list_container);

	// Because the scroll pane inherits from some other place 
	// we need to manually set it up for nesting
	makeNestedMouseMovieClip(this.scroll_pane, true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.hScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.vScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);

	// Set ourselves up as a listener for any external calls
	// Used for Asset Finder
	_root.external_call.addListener(this);

}// end constructor()

// Make it inherit from Tab Content Area
mcTabContentAreaTreeClass.prototype = new mcTabContentAreaClass();

/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
*/
mcTabContentAreaTreeClass.prototype.setSize = function(w, h)
{
	super.setSize(w, h);

	this.menu_container.setWidth(w);
	this.scroll_pane.setSize(w, h - this.menu_container._height);
	this.list_container.refresh();

}// setSize()

/**
* Fired when we are pressed and then release
*
* @access public
*/
mcTabContentAreaTreeClass.prototype.onRelease = function() 
{
	return super.onRelease(); // Fucking flash see SUPER_METHOD_EG.as
}// end




/**
* Event fired whenever a command is made from outside the flash movie
*
* @param string	cmd		the command to perform
* @param object	params	the parameters for the command
*
* @access public
*/
mcTabContentAreaTreeClass.prototype.onExternalCall = function(cmd, params) 
{
	switch(cmd) {
		case "asset_finder" :
			if (params.type_codes_xml == null || params.type_codes_xml.length <= 0) return;
			var xml  = new XML(params.type_codes_xml);

			// something buggered up with the connection
			if (xml.status != 0) {
				_root.dialog_box.show("XML Error, unable to use asset finder", "XML Status '" + xml.status + "'\nPlease Try Again");
				return;

			// we got an unexpected root node
			} else if (xml.firstChild.nodeName != "type_codes") {
				_root.dialog_box.show("XML Error, unable to print messages", "Unexpected Root XML Node '" + xml.firstChild.nodeName + '"');
				return;
			}// end if

			// everything went well, load 'em up
			var type_codes = new Array();
			for (var i = 0; i < xml.firstChild.childNodes.length; i++) {
				// get a reference to the child node
				var node = xml.firstChild.childNodes[i];
				if (node.nodeName.toLowerCase() == "message") {
					type_codes.push({type: node.attributes.type, text: node.firstChild.nodeValue}); 
				}//end if
			}//end for
			this.startAssetFinder(type_codes);

		break;
	}// end switch

}// end onExternalCall()

/**
* Starts the asset finder
*
* @param Array(string)	type_codes		the allowed type codes in the asset finder
*
* @access public
*/
mcTabContentAreaTreeClass.prototype.startAssetFinder = function(type_codes) 
{
	trace("START ASSET FINDER");
	trace("UNDEFINED : " (this.asset_finder_heading == undefined));
	// if we haven't created the heading before do so now
	if (this.asset_finder_heading == undefined) {
		trace('Attach');
		this.attachMovie("mcAssetFinderHeadingID", "asset_finder_heading", 4);
		this.asset_finder_heading._x = 0;
		this.asset_finder_heading._y = 0;
	}

	this.asset_finder_heading.setWidth(this.menu_container._width);

	this.asset_finder_heading._visible = true;
	this.menu_container._visible = false;

}// end startAssetFinder()


/**
* Cancels the asset finder
*
* @access public
*/
mcTabContentAreaTreeClass.prototype.cancelAssetFinder = function() 
{
	this.asset_finder_heading._visible = false;
	this.menu_container._visible = true;

}// end cancelAssetFinder()


Object.registerClass("mcTabContentAreaTreeID", mcTabContentAreaTreeClass);
