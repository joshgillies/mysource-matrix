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
* $Id: mcListItemClass.as,v 1.36 2003/11/18 15:37:35 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


#include "mcListItemPlusMinus.as"
#include "mcListItemMove.as"


// Create the Class
function mcListItemClass()
{
//	trace("mcListItemClass.(constructor)()");
	this.parent_item_name = "";
	this.asset_path = Array();
	this.link_path = Array();

	this.linkid    = "";
	this.assetid   = "";
	this.type_code = "";
	this.pos       = 0;
	this.indent    = 0;

	this._accessible = false;
	this._active = true;

	// the current state that the buttons is in 
	this.state = 'normal';
	this.selected = false;
	this.actions_bar_interval = 0;

	this.text_field.swapDepths(1);

	// Create the Plus Minus Button
	this.attachMovie("mcListItemPlusMinusID", "kids_button", 2);
	this.kids_button._x = 3;
	this.kids_button._y = 5;

	// Create the Move Button
	this.attachMovie("mcListItemMoveID", "move_button", 3);
	this.move_button.setState("off");
	this.move_button._x = 20;

	// set the text field up
	this.text_field.text = "";
	this.text_field.autoSize = "left";

	// background alpha
	this.bg_alpha = 0;

	// link type alpha 
	this.link_type_alpha = Array();
	this.link_type_alpha[1] = 100;
	this.link_type_alpha[2] = 40;
	this.link_type_alpha[3] = 0;
}

// Make it inherit from Nested Mouse Movements MovieClip, nested mc's NEVER OVERLAP
mcListItemClass.prototype = new NestedMouseMovieClip(true, NestedMouseMovieClip.NM_ON_PRESS | NestedMouseMovieClip.NM_ON_RELEASE);

mcListItemClass.prototype.setParent = function(parent_item_name) 
{
}//end setParent()

mcListItemClass.prototype.getParentAssetid = function() 
{
	return _root.asset_manager.asset_links[this.linkid].majorid;
}//end getParentAssetid()


/** 
* Sets the link for which the asset for this item is the minor asset
* 
* @param Link link The link
*
* @access public
*
*/
mcListItemClass.prototype.setLink = function(link) 
{
//	trace (this + "::mcListItemClass.setLink(" + link + ")");
//	trace("asset id: " + this.assetid);
	this.linkid = link.linkid;
	this.link_type = link.link_type;
	
	this.text_field.text = this.getText();
	var textformat = this.text_field.getTextFormat();
	textformat.color = (this.selected) ? (0xffffff) : (0x000000);
	this.text_field.setTextFormat(textformat);


}//end setLink()

mcListItemClass.prototype.setAsset = function(asset, parent_item_name) 
{
//	trace(this + "::mcListItemClass.setAsset(" + asset.assetid + ", " + parent_item_name + ")");
	var parentItem = this._parent[parent_item_name];
	
	if (parentItem != undefined) {
		this.asset_path = parentItem.asset_path.clone();
		this.asset_path.push(asset.assetid);
	} else {
		this.asset_path = Array(asset.assetid);
	}

	if (parentItem != undefined) {
		this.link_path = parentItem.link_path.clone();
		this.link_path.push(this.linkid);
	} else {
		this.link_path = Array(this.linkid);
	}

	this.parent_item_name = parent_item_name;
	
	if (this.asset.assetid != asset.assetid) {
		if (this.asset != undefined)
			this.asset.removeListener(this);
	
		asset.addListener(this);
	}

	this.asset				= asset;
	this.assetid			= asset.assetid;
	this.type_code			= asset.type_code;
	this._accessible		= asset.accessible;
	this.text_field.text	= this.getText();
	this.asset_status		= asset.status;


	var textformat = this.text_field.getTextFormat();
	textformat.color = (this.selected) ? (0xffffff) : (0x000000);
	this.text_field.setTextFormat(textformat);

	if (asset.paths.length == 0) {
		if (asset.url != '') {
			this.preview_url		= asset.url;
		} else {
			this.preview_url		= parentItem.preview_url;
		}
	} else {
		if (this.parent_item_name != 'li') {
			var parent_preview_url	= parentItem.preview_url;
			if (parent_preview_url != undefined)
				this.preview_url		= parent_preview_url + "/" + asset.paths[0]; // choose arbitrary web path
		} else {
			this.preview_url		= asset.url;
		}
	}
	
	this.move_button.setIcon("mc_asset_type_" + asset.type_code + "_icon");

	this.state = Asset.STATUSES[asset.status];

	if (asset.accessible && asset.links.length > 0) {
		this.setKidState((this.expanded()) ? "minus" : "plus");
	} else {
		this.setKidState("none");
	}

	this.refresh();
}//end setAsset()

mcListItemClass.prototype.getBgColour = function()
{
	if (!this.selected) {
		return adjust_brightness(_root.LIST_ITEM_BG_COLOURS[this.state].normal, 0.5);
	} else {
		return _root.LIST_ITEM_BG_COLOURS[this.state].selected;
	}
}

mcListItemClass.prototype.getText = function()
{
	var text = this.asset.name;
	if (this.link_type == 2)
		text += "*";
	text += " [" + this.assetid + "]";

	return text;
}//end getText()

mcListItemClass.prototype.setInfo = function(asset, link, parent_name) 
{
//	trace (this + "::mcListItemClass.setInfo (" + asset + ", " + link + ", " + parent_name + " )");
	this.setLink(link);
	this.setAsset(asset, parent_name);

}//end setInfo()

mcListItemClass.prototype.getKidState = function() 
{
	return this.kids_button.getState();
}

mcListItemClass.prototype.setKidState = function(state) 
{
	this.kids_button.setState(state);
}

mcListItemClass.prototype.setMoveState = function(state) 
{
	this.move_button.setState(state);
}


mcListItemClass.prototype.expanded = function() 
{
	return (this.kids_button.getState() == "minus") ? true : false;
}

mcListItemClass.prototype.setIndent = function(indent) 
{
	if (indent   < 0) indent   = 0;
	this._x = indent * _root.LIST_ITEM_INDENT_SPACE;
	this.indent   = indent;
}

mcListItemClass.prototype.setPos = function(pos) 
{
	if (pos < 0) pos = 0;
	this._y = (pos * _root.LIST_ITEM_POS_INCREMENT) + (this._parent.items_order[pos].branch_count * _root.LIST_ITEM_END_BRANCH_GAP);
	this.pos = pos;
}

mcListItemClass.prototype.resetXY = function() 
{
	this.setIndent(this.indent);
	this.setPos(this.pos);
}


/**
* Called by the kids button to show our kids
* @access public
*/
mcListItemClass.prototype.showKids = function() 
{
	this._parent.showKids(this.assetid, this._name);
}

/**
* Called by the kids button to hide our kids
* @access public
*/
mcListItemClass.prototype.hideKids = function() 
{
	this._parent.hideKids(this.assetid, this._name);
}


/**
* Called by the move button to start a move
* @access public
*/
mcListItemClass.prototype.startMove = function() 
{
	// if we aren't accessible or active we can't move
	if (!this._accessible || !this._active) return;
	this._parent.startMove();
}

/**
* Returns the button over which the mouse currently resides
*
* @returns string
* @access public
*/
mcListItemClass.prototype.getMouseButton = function() 
{
	// if we are over the kids button
	if (this.kids_button.hitTest(_root._xmouse, _root._ymouse, false)) {
		return "kids";

	// if we are over the move button
	} else if (this.move_button.hitTest(_root._xmouse, _root._ymouse, false)) {
		return "move";

	} else {
		return "";
	}

}

/**
* When the asset is changed update ourselves to reflect changes
*
* @param object Asset	asset	the asset that changed
*
*/
mcListItemClass.prototype.onAssetChange = function(asset) 
{
//	trace (this + ".onAssetChange(" + asset.assetid + "/" + this.linkid + ")");
	var link = _root.asset_manager.asset_links[this.linkid];
	this.setInfo(asset, link, this.parent_item_name);
}//end onAssetChange()


mcListItemClass.prototype.setShowColours = function(show_colours)
{
	if (show_colours) {
		this.bg_alpha = 100;
	} else {
		this.bg_alpha = 0;
	}

	this._drawBg();
}

/**
* Called when this list item is selected
*/
mcListItemClass.prototype.select = function() 
{
	if (!this._active)
		return;
	this.selected = true;
	
	// change text
	var textformat = this.text_field.getTextFormat();
	textformat.color = 0xffffff;
	this.text_field.setTextFormat(textformat);

	this._drawBg();
}

/**
* Called when this list item is unselected
*/
mcListItemClass.prototype.unselect = function() 
{
	if (!this._active)
		return;
	this.selected = false;

	// change text
	var textformat = this.text_field.getTextFormat();
	textformat.color = 0x000000;
	this.text_field.setTextFormat(textformat);

	this._drawBg();
}

/**
* Called to enable/disable this item
*
* @param boolean	active		are we active or not
* 
*/
mcListItemClass.prototype.setActive = function(active)
{
	if (!active)
		this.unselect();

	this._active = active;

	var text_format = this.text_field.getTextFormat();
	if (!this._active) {
		text_format.color = 0x999999;
	} else {
		text_format.color = (this.selected) ?  0xffffff : 0x000000;
	}
	this.text_field.setTextFormat(text_format); 

}//end setActive()


/**
* Called when this item has been pressed
*/
mcListItemClass.prototype.onPress = function()
{
	if (!this._accessible) return false;

	this._parent.selectItem(this);
	// try the kids, but it they don't want it then set the interval for the actions bar
	if (!super.onPress() && this._active) {
		this.actions_bar_interval = setInterval(this, "showActionsBar", 100);
	}
	return true;
}//end onPress()

/**
* Called when this item has been pressed
*/
mcListItemClass.prototype.showActionsBar = function()
{
	if (!this._accessible) return;
	clearInterval(this.actions_bar_interval);
	this.actions_bar_interval = 0;
	this._parent._parent.showActionsBar();
}// end showActionsBar()

/**
* Called when this item has been pressed and then released
*/
mcListItemClass.prototype.onRelease = function()
{
	if (!this._accessible) return false;
	if (this.actions_bar_interval) {
		clearInterval(this.actions_bar_interval);
		this.actions_bar_interval = 0;
	}
	super.onRelease();
	return true;
}//end onRelease()

/**
* Called when this item has been pressed and then the mouse was released somewhere else
*/
mcListItemClass.prototype.onReleaseOutside = function()
{
	if (!this._accessible) return false;
	if (this.actions_bar_interval) {
		clearInterval(this.actions_bar_interval);
		this.actions_bar_interval = 0;
	}
	super.onReleaseOutside();
	return true;
}// end onReleaseOutside()

mcListItemClass.prototype.refresh = function()
{
//	trace (this + "::mcListItemClass.refresh()");

	var iconPadding = 5;
	var nextX = iconPadding;
	
	this.kids_button._x = nextX;
	nextX += this.kids_button._width;
	nextX += iconPadding;

	this.move_button._x = nextX;
	nextX += this.move_button._width;
	nextX += iconPadding;
	
	nextX = Math.max (nextX, 20);
	this.text_field._x = nextX;
	this.text_field._y = 0;
	this.text_field._y = ( this._height - this.text_field._height ) / 2;

	this.kids_button._y = (this._height - this.kids_button._height) / 2;
	this.move_button._y = (this._height - this.move_button._height) / 2;
	this.text_field._y = (this._height - this.text_field._height) / 2;

	this._drawBg();

}

/**
* Draw the Background for this list item
*/
mcListItemClass.prototype._drawBg = function(alpha) 
{
	if (alpha == undefined)
		alpha = 100;

	var list_container = this._parent._parent;
	var left = -this._x;
	var top = 2;
	var right = Math.max (_root.tabs.tree.scroll_pane._width - this._x - 25, list_container.getRightEdge() - this._x + 5);
	var bottom = _root.LIST_ITEM_POS_INCREMENT - 2;
	this.clear();

	this.beginFill(this.getBgColour(), this.bg_alpha);

	this.lineStyle();
	this.moveTo(left, top);
	this.lineTo(right, top);
	this.lineTo(right, bottom);
	this.lineTo(left, bottom);
	this.lineTo(left, top);
	this.endFill();

}//end _drawBg()

Object.registerClass("mcListItemID", mcListItemClass);

