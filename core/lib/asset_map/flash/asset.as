
// Create the Class
function Asset(assetid, type_code, name, has_kids) 
{
	this.assetid    = assetid;
	this.type_code  = type_code;
	this.name       = name;
	this.has_kids   = has_kids;
	this.kids       = new Array(); // array of assetids
	this.list_items = new Array(); // array of list item names
}

Asset.prototype.toString = function()
{
	return "AssetId: " + this.assetid + 
			", Type Code: " + this.type_code + 
			", Name: " + this.name + 
			", Has Kids: " + this.has_kids;
}

