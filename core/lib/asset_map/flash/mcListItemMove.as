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
* $Id: mcListItemMove.as,v 1.7 2003/11/26 00:51:12 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


// Create the Class
function mcListItemMoveClass()
{
}

// Make it inherit from MovieClip
mcListItemMoveClass.prototype = new MovieClip();

mcListItemMoveClass.prototype.setIcon = function(iconID)
{
	if (this.icon != undefined)
		this.icon.removeMovieClip();

	this.attachMovie(iconID, 'icon', 1);
	if (typeof this.icon != "movieclip") {
		this.attachMovie("mc_asset_type_default_icon", 'icon', 1);
	}
}

mcListItemMoveClass.prototype.getState = function()
{

	switch(this._currentframe) {
		case 1 :
			return "off";
		break;
		case 2 :
			return "on";
		break;
		default :
			return "";
	}	
}

mcListItemMoveClass.prototype.setState = function(state)
{
	switch(state) {
		case "on" :
		case "off":
			this.gotoAndStop(state);
		break;
		default :
//			trace("ERROR: State '" + state + "' unknown for mcListItemMove");
	}	
}


mcListItemMoveClass.prototype.onPress = function()
{
	// return true so that the list item knows we have been pressed
	return true;
}

mcListItemMoveClass.prototype.onRelease = function()
{
	if(this.getState() == "off") {
		this._parent.startMove();
		break;
	}

}// end onRelease()


Object.registerClass("mcListItemMoveID", mcListItemMoveClass);
