
// Create the Class
function AssetType(type_code, name, version, instantiable, allowed_access, parent_type, edit_screens) 
{
	this.type_code		= type_code;
	this.name			= name;
	this.version		= version;
	this.instantiable	= instantiable;
	this.allowed_access	= allowed_access;
	this.parent_type	= parent_type;
	this.edit_screens	= edit_screens;

	this.sub_types		= new Array();

}

AssetType.prototype.toString = function()
{

	var edit_opts = "{";
	for(var i = 0; i < this.edit_screens.length; i++) {
		var c = this.edit_screens[i].code_name;
		var n = this.edit_screens[i].name;
		edit_opts += "(" + c + ": " + n + ")";
	}
	edit_opts += "}";


	return "Type Code: " + this.type_code + 
			", Name: " + this.name + 
			", Version: " + this.version + 
			", Sys Only: " + this.allowed_access + 
			", Parent Type: " + this.parent_type +
			", Edit Opts: " + edit_opts +
			", Sub Types: " + this.sub_types;
}

/**
* Whether or not this asset type can be created by the user
*
* @return boolean
*/
AssetType.prototype.createable = function()
{
	var user_type = _root.asset_manager.asset_types[_root.asset_manager.assets[_root.asset_manager.current_userid].type_code];
	return (this.instantiable && user_type.isType(this.allowed_access));
}

/**
* Returns true if this asset type or one of it's ancestors is the passed type
*
* @return boolean
*/
AssetType.prototype.isType = function(type)
{
	if (this.type_code == type) return true;
	var parent_type = this.type_code;

	while(_root.asset_manager.asset_types[parent_type].parent_type != 'asset') {
		var parent_type = _root.asset_manager.asset_types[parent_type].parent_type;
		if (parent_type == type) return true;
		// this should NEVER happen, if it does DIE
		if (_root.asset_manager.asset_types[parent_type] == undefined) return false;
	}
	return false;
}

