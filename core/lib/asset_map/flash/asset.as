
// Create the Class
function Asset() 
{
	this.assetid    = 0;
	this.type_code  = "";
	this.name       = "";
	this.accessible  = true;
	this.links      = new Array(); // array of linkids


	// We need to define our own instance of this here for the broadcasting,
	// because we are inialising the ASBroadcaster on the prototype
	this._listeners = new Array(); 

}

// Set ourselves up as a broadcaster
ASBroadcaster.initialize(Asset.prototype);


// all the ones we care about
Asset.prototype.UNDER_CONSTRUCTION_STATUS = 1;
Asset.prototype.LIVE_STATUS = 4;


Asset.prototype.toString = function()
{
	return  "AssetId: " + this.assetid + 
			", Type Code: " + this.type_code + 
			", Name: " + this.name + 
			", Accessible : " + this.accessible + 
			", Status : " + this.status + 
			", Links: " + this.links;
}

Asset.prototype.setInfo = function(assetid, type_code, name, accessible, status, links)
{
	if (assetid    != undefined && assetid    != null) this.assetid    = assetid;
	if (type_code  != undefined && type_code  != null) this.type_code  = type_code;
	if (name       != undefined && name       != null) this.name       = name;
	if (accessible != undefined && accessible != null) this.accessible = (accessible == "1") ? true : false;
	if (status     != undefined && status     != null) this.status     = status;
	if (links      != undefined && links      != null) this.links      = links;
	this.broadcastMessage("onAssetChange", this);
}


Asset.prototype.clone = function()
{
	var copy = new Asset();
	copy.assetid    = this.assetid;
	copy.type_code  = this.type_code;
	copy.name       = this.name;
	copy.accessible = this.accessible;
	copy.links      = this.links.clone();

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
