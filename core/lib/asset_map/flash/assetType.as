
// Create the Class
function AssetType(type_code, name, version, instantiable, system_only, parent_type, edit_screens) 
{

	this.type_code		= type_code;
	this.name			= name;
	this.version		= version;
	this.instantiable	= instantiable;
	this.system_only	= system_only;
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
			", Sys Only: " + this.system_only + 
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
	return (this.instantiable && !this.system_only);
}

