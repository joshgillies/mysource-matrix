
// Create the Class
function AssetLink(linkid, majorid, minorid, link_type) 
{
	this.linkid     = linkid;
	this.majorid    = majorid;
	this.minorid    = minorid;
	this.link_type  = link_type;
}

AssetLink.prototype.toString = function()
{
	return "LinkId: " + linkid + 
			", MajorId: " + this.majorid + 
			", MinorId: " + this.minorid + 
			", Link Type: " + this.link_type;
}


AssetLink.prototype.clone = function()
{
	return new AssetLink(this.linkid, this.majorid, this.minorid, this.link_type)
}


/**
* Returns true if an object equals the other obj
*
* @param Object other_obj	the object to check
*
* @return boolean
*/
AssetLink.prototype.equals = function(other_obj) 
{
	if (other_obj instanceof AssetLink) {
		return (this.linkid    == other_obj.linkid && 
				this.majorid   == other_obj.majorid &&
				this.minorid   == other_obj.minorid &&
				this.link_type == other_obj.link_type);
	}
	return false;
}
