
/**
* This class controls the sending and retrieval of data from the server in XML
*
*/
function AssetManager()
{
	// holds all the asset types that are available in the system
	this.types  = new Object();
	// holds all the assets that have been referenced in this container
	this.assets = new Object();

	// a temp object that can hold any run-time data
	this.tmp = new Object();

	// Set ourselves up as a listener for any external calls
	_root.external_call.addListener(this);

	// Set ourselves up as a broadcaster
    ASBroadcaster.initialize(this);


	var xml = new XML();
	var cmd_elem = xml.createElement("command");
	cmd_elem.attributes.action = "get asset types";
	xml.appendChild(cmd_elem);

	trace(xml);

	// start the loading process
	var exec_indentifier = _root.server_exec.init_exec(xml, this, "loadTypesFromXML", "asset_types");
	_root.server_exec.exec(exec_indentifier, "Loading Assets Types");

}

/**
* Called after the XML has been loaded 
*
* @param object XML xml   the xml object that contain the information that we need
*
*/
AssetManager.prototype.loadTypesFromXML = function(xml, exec_indentifier) 
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
AssetManager.prototype.getTopTypes = function() 
{

	var top_level = new Array();

	for (var type_code in this.types) {
		if (this.types[type_code].parent_type == "asset") {
			top_level.push(type_code);
		}
	}

	return top_level;

}// end getTopTypes()


/**
* Loads the passed assetids asset values from the server, unless they exist already
* The call_back_fn will be passed in the call_back_params, and if force_reload is true
* will get 2 extra args - old_assets and new_assets, allowing you to compare the changes 
* that occured in the reload if need be
*
* @param array(int)	assetids			array of assetids to load
* @param string		call_back_obj		the object to run the call back fn on
* @param string		call_back_fn		the fn in this class to execute after the loading has finished
* @param Object		call_back_params	an object of params to send to the call back fn
* @param boolean	force_reload		reload from server, even if we already have references to the passed assets
*
*/
AssetManager.prototype.loadAssets = function(assetids, call_back_obj, call_back_fn, call_back_params, force_reload) 
{
	
	if (force_reload !== true) force_reload = false;

	trace('Load Assets : ' + assetids);

	var load_assetids = array_copy(assetids);

	if (!force_reload) {
	    for (var i = 0; i < load_assetids.length; i++) {
			if (this.assets[load_assetids[i]] != undefined) {
				load_assetids.splice(i, 1);
				i--;
			}
		}
	}

	trace('Load Assets : ' + load_assetids);

	// if there are assets to load, get them from server
	if (load_assetids.length) {

		var xml = new XML();
		var cmd_elem = xml.createElement("command");
		cmd_elem.attributes.action = "get assets";
		xml.appendChild(cmd_elem);

	    for (var i = 0; i < load_assetids.length; i++) {
			var asset_elem = xml.createElement("asset");
			asset_elem.attributes.assetid = load_assetids[i];
			cmd_elem.appendChild(asset_elem);
		}

		trace(xml);

		// start the loading process
		var exec_indentifier = _root.server_exec.init_exec(xml, this, "loadAssetsFromXML", "assets");
		if (this.tmp.load_assets == undefined) this.tmp.load_assets = new Object();
		this.tmp.load_assets[exec_indentifier] = {force_reload: force_reload, obj: call_back_obj, fn: call_back_fn, params: call_back_params};
		_root.server_exec.exec(exec_indentifier, "Loading Assets");

	// else nothing to do, so just execute the call back fn
	} else {
		call_back_obj[call_back_fn](call_back_params);

	}// end if
	
}// end loadAssets()


/**
* Called after the XML has been loaded 
*
* @param object XML xml   the xml object that contain the information that we need
*
*/
AssetManager.prototype.loadAssetsFromXML = function(xml, exec_indentifier) 
{

	var old_assets = new Object();
	var new_assets = new Object();

	for (var i = 0; i < xml.firstChild.childNodes.length; i++) {
		// get a reference to the child node
		var asset_node = xml.firstChild.childNodes[i];
		if (asset_node.nodeName.toLowerCase() == "asset") {

			var kids = new Array();

			for (var j = 0; j < asset_node.childNodes.length; j++) {
				kids.push(asset_node.childNodes[j].attributes.assetid);
			}

			// only create if it doesn't already exist
			if (this.assets[asset_node.attributes.assetid] == undefined) {
				this.assets[asset_node.attributes.assetid] = new Asset();

			// take backup before we update
			} else {
				old_assets[asset_node.attributes.assetid] = this.assets[asset_node.attributes.assetid].clone();

			}

			this.assets[asset_node.attributes.assetid].setInfo(asset_node.attributes.assetid, asset_node.attributes.type_code, asset_node.attributes.name, kids);
			trace(this.assets[asset_node.attributes.assetid]);

			new_assets[asset_node.attributes.assetid] = this.assets[asset_node.attributes.assetid];

		}//end if
	}//end for

	if (this.tmp.load_assets[exec_indentifier].force_reload) {
		trace(this.tmp.load_assets[exec_indentifier].obj + "[ " + this.tmp.load_assets[exec_indentifier].fn + "](this.tmp.load_assets[exec_indentifier].params, new_assets, old_assets);");
		this.tmp.load_assets[exec_indentifier].obj[this.tmp.load_assets[exec_indentifier].fn](this.tmp.load_assets[exec_indentifier].params, new_assets, old_assets);
	} else {
		this.tmp.load_assets[exec_indentifier].obj[this.tmp.load_assets[exec_indentifier].fn](this.tmp.load_assets[exec_indentifier].params);
	}

	delete this.tmp.load_assets[exec_indentifier];

}// end loadAssets()



/**
* Forces the re-retrieval of the passed assets information, including who it's kids are
*
* @param int assetid
*
* @access public
*/
AssetManager.prototype.reloadAsset = function(assetid) 
{
	if (this.assets[assetid] == undefined) return;
	this.loadAssets([assetid], this, "reloadAssetLoaded", {assetid: assetid}, true);

}// end reloadAsset()

/**
* Continuation of reloadAsset() after asset is loaded from server
*
* @param object params
*
* @access public
*/
AssetManager.prototype.reloadAssetLoaded = function(params, new_assets, old_assets) 
{
	if (this.assets[params.assetid] == undefined) return;
	this.broadcastMessage("onAssetReload", params.assetid, new_assets[params.assetid], old_assets[params.assetid]);

}// end reloadAssetLoaded()


/**
* Event fired whenever a command is made from outside the flash movie
*
* @param string	cmd		the command to perform
* @param object	params	the parameters for the command
*
* @access public
*/
AssetManager.prototype.onExternalCall = function(cmd, params) 
{

	switch(cmd) {
		case "reload_asset" :
			if (params.assetid) {
				this.reloadAsset(params.assetid);
			}
		break;
	}

}// end onExternalCall()

