/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: assetLink.as,v 1.6 2003/11/26 00:51:12 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


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
	return "LinkId: " + this.linkid + 
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
