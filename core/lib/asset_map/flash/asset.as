/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: asset.as,v 1.14 2003/09/26 05:26:32 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


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
/* 
define('SQ_STATUS_ARCHIVED',           1); // asset is archived
define('SQ_STATUS_UNDER_CONSTRUCTION', 2); // asset is under construction
define('SQ_STATUS_PENDING_APPROVAL',   4); // asset is currently in workflow
define('SQ_STATUS_APPROVED',           8); // asset is approved waiting to go live from under construction
define('SQ_STATUS_LIVE',              16); // asset is live
define('SQ_STATUS_LIVE_APPROVAL',     32); // asset is up for review
define('SQ_STATUS_EDITING',           64); // asset is currently safe editing
define('SQ_STATUS_EDITING_APPROVAL', 128); // asset is currently in workflow from safe edit
define('SQ_STATUS_EDITING_APPROVED', 256); // asset is approved waiting to go live from safe edit

define('SQ_SC_STATUS_NOT_LIVE',      15); // short hand for SQ_STATUS_ARCHIVED | SQ_STATUS_UNDER_CONSTRUCTION | SQ_STATUS_PENDING_APPROVAL | SQ_STATUS_APPROVED
define('SQ_SC_STATUS_CAN_APPROVE',   66); // short hand for SQ_STATUS_UNDER_CONSTRUCTION | SQ_STATUS_EDITING
define('SQ_SC_STATUS_PENDING',      164); // short hand for SQ_STATUS_PENDING_APPROVAL | SQ_STATUS_EDITING_APPROVAL | SQ_STATUS_LIVE_APPROVAL
define('SQ_SC_STATUS_ALL_APPROVED', 136); // short hand for SQ_STATUS_APPROVED | SQ_STATUS_EDITING_APPROVED
define('SQ_SC_STATUS_SAFE_EDITING', 448); // short hand for SQ_STATUS_EDITING | SQ_STATUS_EDITING_APPROVAL | SQ_STATUS_EDITING_APPROVED
*/
Asset.prototype.UNDER_CONSTRUCTION_STATUS = 2 | 4 | 8 | 64 | 128 | 256;
Asset.prototype.LIVE_STATUS = 16 | 32;



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
