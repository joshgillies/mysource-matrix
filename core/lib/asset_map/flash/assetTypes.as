
/**
* This class controls the sending and retrieval of data from the server in XML
*
*/
function assetTypes()
{
	this.types = new Object();

	var xml = new XML();
	var cmd_elem = xml.createElement("command");
	cmd_elem.attributes.action = "get asset types";
	xml.appendChild(cmd_elem);

	trace(xml);

	// start the loading process
	var exec_indentifier = _root.server_exec.init_exec(xml, this, "loadTypesFromXML", "asset_types");
	_root.server_exec.exec(exec_indentifier, "Loading Assets Types");

	// Set ourselves up as a broadcaster
    ASBroadcaster.initialize(this);
}

/**
* Called after the XML has been loaded 
*
* @param object XML xml   the xml object that contain the information that we need
*
*/
assetTypes.prototype.loadTypesFromXML = function(xml, exec_indentifier) 
{

	trace(xml);

	for (var i = 0; i < xml.firstChild.childNodes.length; i++) {
		// get a reference to the child node
		var type_node = xml.firstChild.childNodes[i];
		if (type_node.nodeName.toLowerCase() == "type") {

			this.types[type_node.attributes.type_code] = new AssetType(type_node.attributes.type_code,
																		type_node.attributes.name,
																		type_node.attributes.version,
																		type_node.attributes.instantiable,
																		type_node.attributes.system_only,
																		type_node.attributes.parent_type);

		}//end if
	}//end for

	// make sure that all the asset types know about their sub types
	for (var type_code in this.types) {
		if (this.types[type_code].parent_type != "asset") {
			this.types[this.types[type_code].parent_type].sub_types.push(type_code);
		}
	}

	for (var type_code in this.types) trace(this.types[type_code]);

	this.broadcastMessage("onAssetTypesLoaded");

}// end loadTypesFromXML()


/**
* Returns an array of the asset types code that have a parent "asset", ie are at the top of the tree
*
* @return array
*/
assetTypes.prototype.getTopTypes = function() 
{

	var top_level = new Array();

	for (var type_code in this.types) {
		if (this.types[type_code].parent_type == "asset") {
			top_level.push(type_code);
		}
	}

	return top_level;

}// end getTopTypes()
