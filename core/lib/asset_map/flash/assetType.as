
// Create the Class
function AssetType(type_code, name, version, instantiable, system_only, parent_type) 
{

	this.type_code    = type_code;
	this.name         = name;
	this.version      = version;
	this.instantiable = instantiable;
	this.system_only  = system_only;
	this.parent_type  = parent_type;

	this.sub_types    = new Array();


}

AssetType.prototype.toString = function()
{
	return "Type Code: " + this.type_code + 
			", Name: " + this.name + 
			", Version: " + this.version + 
			", Sys Only: " + this.system_only + 
			", Parent Type: " + this.parent_type +
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

