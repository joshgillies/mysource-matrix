/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: asset.as,v 1.18 2003/11/18 15:37:34 brobertson Exp $
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

Asset.prototype.STATUSES = Array();
Asset.prototype.STATUSES[1]		= 'archived';
Asset.prototype.STATUSES[2]		= 'under_construction';
Asset.prototype.STATUSES[4]		= 'pending_approval';
Asset.prototype.STATUSES[8]		= 'approved';
Asset.prototype.STATUSES[16]	= 'live';
Asset.prototype.STATUSES[32]	= 'live_approval';
Asset.prototype.STATUSES[64]	= 'editing';
Asset.prototype.STATUSES[128]	= 'editing_approval';
Asset.prototype.STATUSES[256]	= 'editing_approved';


Asset.prototype.toString = function()
{
	return  "AssetId: " + this.assetid + 
			", Type Code: " + this.type_code + 
			", Name: " + this.name + 
			", Accessible: " + this.accessible + 
			", Status: " + this.status + 
			", URL: " + this.url + 
			", Paths: [" + this.paths + "]" +
			", Links: [" + this.links + "]";
}


Asset.prototype.setInfo = function(assetid, type_code, name, accessible, status, url, paths, links)
{
//	trace("setInfo called on " + assetid);
	if (assetid    != undefined && assetid    != null) this.assetid		= assetid;
	if (type_code  != undefined && type_code  != null) this.type_code	= type_code;
	if (name       != undefined && name       != null) this.name		= name;
	if (accessible != undefined && accessible != null) this.accessible	= (accessible == "1") ? true : false;
	if (status     != undefined && status     != null) this.status		= status;
	if (url        != undefined && url        != null) this.url			= url;
	if (paths      != undefined && paths      != null) this.paths		= paths;
	if (links      != undefined && links      != null) this.links		= links;
	this.broadcastMessage("onAssetChange", this);
}


Asset.prototype.clone = function()
{
	var copy = new Asset();
	copy.assetid    = this.assetid;
	copy.type_code  = this.type_code;
	copy.name       = this.name;
	copy.accessible = this.accessible;
	copy.paths		= this.paths.clone();
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
