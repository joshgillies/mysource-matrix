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
* $Id: mcMenuContainerClass.as,v 1.24 2003/11/26 00:51:12 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

#include "mcMenuClass.as"
#include "mcMenuItemClass.as"

function mcMenuContainerClass() 
{
	this._children = new Array();
	this._isVertical = false;
	this._bgColour = 0xe0e0e0;

	this.createEmptyMovieClip ('_bg', 1);
	this._childDepthOffset = 2;

	if (this.isRoot()) {
//		trace ("I am the broadcaster : " + this);
		this._bgColour = 0x725B7D;

		// Set ourselves up as a listener on the asset types, so we know when they have been loaded
		_root.asset_manager.addListener(this);
		// Set ourselves up as a listener on the system events
		// so we know when a screen press occurs that isn't ours, allowing us to close the menu
		_root.system_events.addListener(this);

		// Set ourselves up as a broadcaster, so others can be notified of menu items being pressed
		ASBroadcaster.initialize(this);
	}
}

mcMenuContainerClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);
Object.registerClass('mcMenuContainerID', mcMenuContainerClass);


mcMenuContainerClass.prototype.onScreenPress = function(mc)
{
	// if the thing that was pressed wasn't on of the menu items
	// hide the menu
	//trace ("onscreen press: " + mc);
	//if (!(mc instanceof mcMenuItemClass)) this.hideChildMenus();
}

mcMenuContainerClass.prototype.onAssetManagerInitialised = function()
{
	this.clear();

	var newMenu = this.addMenu ('Add', 'mcAddMenuIconID');
	var newMenuContainer = newMenu.getContainer();
	var newMenuHead = newMenu.getHead();
	var format = newMenuHead.getTextFormat();

	format.color = 0xffffff;
	newMenuHead.setTextFormat(format);

	newMenuContainer.createFromArray (_root.asset_manager.getTypeMenu());
	this.hideChildMenus();
}

mcMenuContainerClass.prototype.onRelease = function()
{
//	trace (this + "::mcMenuContainerClass.onRelease()");
	return super.onRelease();
}


mcMenuContainerClass.prototype.__addAssetFn = function() 
{
//	trace (this + "::mcMenuItemContainerClass.__addAssetFn()");
//	trace ("broadcasting : add/" +  this._value);
//	trace ("broadcasting function : " + this.getRootContainer().broadcastMessage);
//	trace ("broadcaster : " + this.getRootContainer());

	this.getRootContainer().broadcastMessage ("onMenuItemPress", "add", this._value);
}

mcMenuContainerClass.prototype._getAddAssetMenuArray = function(types) 
{
	var elementArray = new Array();

	for (var i = 0; i < types.length; i++) {
		var type = _root.asset_manager.asset_types[types[i]];
		var subtypes = type.sub_types;
		var createable = type.createable();
		
		
		if (!createable && subtypes.length == 0) {
			continue;
		}

		var element = new Object();

		if (subtypes.length == 0) {
			// createable leaf
			element.type = 'item';
			element.label = type.name;
			element.iconID = "mc_asset_type_" + type.type_code + "_icon";
			element.value = type.type_code;
			element.action = this.__addAssetFn;

		} else {
			element.type = 'menu';
			element.label = type.name;
			element.iconID = "mc_asset_type_" + type.type_code + "_menu_icon";

			var childMenuArray = this._getAddAssetMenuArray(subtypes);
			
			if (createable) {
				// createable node - add parent to the top of its own child list
				element.label = type.name + " Types";

				var childElement = new Object();
				childElement.type = 'item';
				childElement.label = type.name;
				childElement.iconID = "mc_asset_type_" + type.type_code + "_icon";
				childElement.action = this.__addAssetFn;
				childElement.value = type.type_code;

				childMenuArray.unshift (childElement);
			} else {
				// if not createable AND there are no createable children
				if (childMenuArray.length == 0)
					continue;
			}
			element.children = childMenuArray;
		}
		elementArray.push (element);
	}
	return elementArray;
}

/*	
 *	elementArray is an array of menu elements, represented by an array of values for :
 *		'type' =>  (either menu or item)
 *		'label' => ...
 *		'iconID' => ...
 *		'value' => (for items)
 *		'action' => function ... (for items)
 *		'children' => (for menus)
 *
 */
mcMenuContainerClass.prototype.createFromArray = function (elementArray) 
{
//	trace (this + "::mcMenuContainerClass.createFromArray(" + elementArray + ")");
	for (var i = 0; i < elementArray.length; ++i) {
		var nextElement = elementArray[i];

		if (nextElement.type == 'item') {
//			trace ("item : " + nextElement.value);
			this.addItem (nextElement.label, nextElement.iconID, nextElement.value, nextElement.action);
		} else if (nextElement.type == 'menu') {
			var newMenu = this.addMenu(nextElement.label, nextElement.iconID);
			var newMenuContainer = newMenu.getContainer();
//			trace ("menu : " + children);
			newMenuContainer.createFromArray(nextElement.children);
		}
	}
}

mcMenuContainerClass.prototype.clear = function()
{
	for (var i = 0; i < this._children.length; ++i) {
		this._children[i].removeMovieClip();
	}
	this._children = new Array();
}

mcMenuContainerClass.prototype.addItem = function(label, iconID, value, onReleaseFn) 
{
	//trace (this + "::mcMenuContainerClass.addItem(" + label + ", " + iconID + ", " + value + ", " + onReleaseFn + ")");
	var newIndex = this._children.length;
	var newDepth = newIndex + this._childDepthOffset;
	var showArrow = false;
	var newItem;
	
	newItem = this.attachMovie ('mcMenuItemID', 'item_' + newIndex, newDepth);
	newItem.create (this, label, iconID, value, onReleaseFn, showArrow);

	this._children[newIndex] = newItem;

	this._refresh();

	return newItem;
}

mcMenuContainerClass.prototype.addMenu = function(label, iconID) 
{
	//trace (this + "::mcMenuContainerClass.addMenu (" + label + ", " + iconID + ")");
	var newIndex = this._children.length;
	var newDepth = newIndex + this._childDepthOffset;
	var showArrow = true;
	var newMenu;
	
	newMenu = this.attachMovie ('mcMenuID', 'menu_' + newIndex, newDepth);
	newMenu.create(this, label, iconID);

	this._children[newIndex] = newMenu;

	this._refresh();

	return newMenu;

}

mcMenuContainerClass.prototype.isRoot = function() 
{
	return (!(this._parent instanceof mcMenuClass));
}

mcMenuContainerClass.prototype.getWidth = function() 
{
	var width = 0;

	for (var i = 0; i < this._children.length; ++i) {
		var thisChild = this._children[i];

		if (!this._isVertical) {
			width += thisChild.getWidth();
		} else  {
			if (thisChild.getWidth() > width) {
				width = thisChild.getWidth();
			}
		}
	}
	return width;
}

mcMenuContainerClass.prototype.getHeight = function() 
{
	var height = 0;

	for (var i = 0; i < this._children.length; ++i) {
		var thisChild = this._children[i];

		if (this._isVertical) {
			height += thisChild.getHeight();
		} else {
			if (thisChild.getHeight() > height) {
				height = thisChild.getHeight();
			}
		}
	}

	return height;
}


mcMenuContainerClass.prototype.hideChildMenus = function()
{
//	trace (this + "::mcMenuContainerClass.hideChildMenus()");
	for (var i = 0; i < this._children.length; ++i) {
		var thisChild = this._children[i];

		if (thisChild.isMenu())
			thisChild.hideChildren();
	}
}

mcMenuContainerClass.prototype._refresh = function() 
{
//	trace (this + "::mcMenuContainerClass.refresh()");
	this._refreshChildren();
	this._refreshBackground();
}

mcMenuContainerClass.prototype._refreshChildren = function()
{
	if (this._children.length > 0) {
		this._refreshChildrenWidths();
		this._refreshChildrenPositions();
	}
}
mcMenuContainerClass.prototype._refreshChildrenWidths = function()
{
	var maxWidth = this.getWidth();
	if (this._isVertical) {
		for (var i = 0; i < this._children.length; ++i) {
			var nextChild = this._children[i];
			nextChild.setItemWidth(maxWidth);
		}
	}
}

mcMenuContainerClass.prototype._refreshChildrenPositions = function()
{
	var nextX = 0, nextY = 0;

	for (var i = 0; i < this._children.length; ++i) {
		var child = this._children[i];

		child._x = nextX;
		child._y = nextY;

		if (this._isVertical) {
			nextY += child.getHeight();
		} else {
			nextX += child.getWidth();
		}
	}
	
}

mcMenuContainerClass.prototype._refreshBackground = function()
{
	this._bg.clear();
	var width = this.getWidth();
	var height = this.getHeight();

	this._drawShadow (width, height);
	this._drawBackground (width, height);
	this._drawBorder (width, height);
}

mcMenuContainerClass.prototype._drawBackground = function(width, height)
{
	with (this._bg) {
		beginFill (this._bgColour);
		moveTo (0, 0);
		lineTo (width, 0);
		lineTo (width, height);
		lineTo (0, height);
		lineTo (0, 0);
		endFill();
	}
}

mcMenuContainerClass.prototype._drawShadow = function(width, height)
{
	if (this.isRoot())
		return;
	
	var dist = 2;

	with (this._bg) {
		beginFill (0x000000, 50);
		moveTo (dist, dist);
		lineTo (width + dist, dist);
		lineTo (width + dist, height + dist);
		lineTo (dist, height + dist);
		lineTo (dist, dist);
		endFill();
	}
}

mcMenuContainerClass.prototype._drawBorder = function(width, height)
{
	if (this.isRoot())
		return;
	
	with (this._bg) {
		lineStyle (0);
		moveTo (0, 0);
		lineTo (width, 0);
		lineTo (width, height);
		lineTo (0, height);
		lineTo (0, 0);
	}		
}
