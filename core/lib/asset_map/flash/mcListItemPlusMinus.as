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
* $Id: mcListItemPlusMinus.as,v 1.6 2003/11/26 00:51:12 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


// Create the Class
function mcListItemPlusMinusClass()
{
}

// Make it inherit from MovieClip
mcListItemPlusMinusClass.prototype = new MovieClip();

mcListItemPlusMinusClass.prototype.getState = function()
{

	switch(this._currentframe) {
		case 1 :
			return "plus";
		break;
		case 2 :
			return "minus";
		break;
		case 3 :
			return "none";
		break;
		default :
			return "";
	}	
}

mcListItemPlusMinusClass.prototype.setState = function(state)
{
	switch(state) {
		case "plus" :
		case "minus":
		case "none" :
			this.gotoAndStop(state);
		break;
		default :
//			trace("ERROR: State '" + state + "' unknown for mcListItemPlusMinus");
	}	
}


mcListItemPlusMinusClass.prototype.onPress = function()
{
	// return true so that the list item knows we have been pressed
	return true;
}

mcListItemPlusMinusClass.prototype.onRelease = function()
{
	switch(this.getState()) {
		case "plus" :
			this._parent.showKids();
			break;
		case "minus" :
			this._parent.hideKids();
			break;
	}

}// end onPress()

Object.registerClass("mcListItemPlusMinusID", mcListItemPlusMinusClass);
