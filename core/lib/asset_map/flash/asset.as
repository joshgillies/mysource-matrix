
// Create the Class
function Asset() 
{
	this.assetid    = 0;
	this.type_code  = "";
	this.name       = "";
	this.kids       = new Array(); // array of assetids


	// We need to define our own instance of this here for the broadcasting,
	// because we are inialising the ASBroadcaster on the prototype
	this._listeners = new Array(); 

}

// Set ourselves up as a broadcaster
ASBroadcaster.initialize(Asset.prototype);

Asset.prototype.toString = function()
{
	return "AssetId: " + this.assetid + 
			", Type Code: " + this.type_code + 
			", Name: " + this.name + 
			", Kids: " + this.kids;
}

Asset.prototype.setInfo = function(assetid, type_code, name, kids)
{

	if (assetid   != undefined && assetid   != null) this.assetid   = assetid;
	if (type_code != undefined && type_code != null) this.type_code = type_code;
	if (name      != undefined && name      != null) this.name      = name;
	if (kids      != undefined && kids      != null) this.kids      = kids;
	this.broadcastMessage("onAssetChange", this);
}


Asset.prototype.clone = function()
{
	var clone = new Asset();
	clone.assetid   = this.assetid;
	clone.type_code = this.type_code;
	clone.name      = this.name;
	clone.kids      = array_copy(this.kids);

	return clone;
}


