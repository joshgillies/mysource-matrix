
// Create the Class
function mcPlusMinusClass()
{
	this.assetid   = "";
	this.type_code = "";
	this.item_text = "";
	this.num_down  = 0;
	this.indent    = 0;
}

// Make it inherit from MovieClip
mcPlusMinusClass.prototype = new MovieClip();

mcPlusMinusClass.prototype.getState = function()
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

mcPlusMinusClass.prototype.setState = function(state)
{
	switch(state) {
		case "plus" :
		case "minus":
		case "none" :
			this.gotoAndStop(state);
		break;
		default :
			trace("ERROR: State '" + state + "' unknown for mcPlusMinus");
	}	
}

Object.registerClass("mcPlusMinusID", mcPlusMinusClass);
