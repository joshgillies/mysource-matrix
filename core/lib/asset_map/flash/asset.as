
// Create the Class
function Asset() 
{
	this.assetid    = 0;
	this.type_code  = "";
	this.name       = "";
	this.kids       = new Array(); // array of assetids
	this.item_names = new Array(); // array of list item names
}

Asset.prototype.toString = function()
{
	return "AssetId: " + this.assetid + 
			", Type Code: " + this.type_code + 
			", Name: " + this.name + 
			", Kids: " + this.kids + 
			", Item Names: " + this.item_names;
}

