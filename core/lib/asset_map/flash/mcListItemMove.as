
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
