
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
