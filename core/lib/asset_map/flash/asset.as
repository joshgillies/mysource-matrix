
// Create the Class
function Asset() 
{
	this.assetid    = 0;
	this.type_code  = "";
	this.name       = "";
	this.links      = new Array(); // array of AssetLinks


	// We need to define our own instance of this here for the broadcasting,
	// because we are inialising the ASBroadcaster on the prototype
	this._listeners = new Array(); 

}

// Set ourselves up as a broadcaster
ASBroadcaster.initialize(Asset.prototype);

Asset.prototype.toString = function()
{
	return  "AssetId: " + this.assetid + 
			", Type Code: " + this.type_code + 
			", Name: " + this.name + 
			", Links: " + this.links;
}

Asset.prototype.setInfo = function(assetid, type_code, name, links)
{

	if (assetid   != undefined && assetid   != null) this.assetid   = assetid;
	if (type_code != undefined && type_code != null) this.type_code = type_code;
	if (name      != undefined && name      != null) this.name      = name;
	if (links     != undefined && links     != null) this.links     = links;
	this.broadcastMessage("onAssetChange", this);
}


Asset.prototype.clone = function()
{
	var copy = new Asset();
	copy.assetid   = this.assetid;
	copy.type_code = this.type_code;
	copy.name      = this.name;
	copy.links     = this.links.clone();

	return copy;
}


Asset.prototype.getLinkAssetids = function()
{
	var assetids = new Array();
	for(var i = 0; i < this.links.length; i++) {
		assetids[i] = _root.asset_manager.asset_links[this.links[i]].minorid;
	}
	return assetids;
}

/**
* Returns the position in the links array of the passed link
* Returns NULL if not found
*
* @param int	linkid
*
* @return int
*/
Asset.prototype.linkPos = function(linkid)
{
	return this.links.search(linkid);
}
