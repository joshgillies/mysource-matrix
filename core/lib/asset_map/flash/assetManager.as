#include "assetType.as"
#include "assetLink.as"
#include "asset.as"

/**
* This class controls the sending and retrieval of data from the server in XML
*
*/
function AssetManager()
{
	// holds all the asset types that are available in the system
	this.asset_types  = new Object();

	// holds all the assets that have been referenced
	this.assets = new Object();

	// holds all the assets links that have been referenced
	this.asset_links = new Object();

	// holds the current users assetid
	this.current_userid = 0;

	// a temp object that can hold any run-time data
	this.tmp = new Object();

	// Set ourselves up as a listener for any external calls
	// Used to refresh assets
	_root.external_call.addListener(this);

	// Set ourselves up as a broadcaster
    ASBroadcaster.initialize(this);

}

/**
* Called to intialised the asset manager
*
*/
AssetManager.prototype.init = function() 
{
	var xml = new XML();
	var cmd_elem = xml.createElement("command");
	cmd_elem.attributes.action = "initalise";
	xml.appendChild(cmd_elem);

	// start the loading process
	var exec_indentifier = _root.server_exec.init_exec(xml, this, "initFromXML", "initialisation");
	_root.server_exec.exec(exec_indentifier, "Initialising Asset Manager");

}


/**
* Called after the XML has been loaded 
*
* @param object XML xml   the xml object that contain the information that we need
*
*/
AssetManager.prototype.initFromXML = function(xml, exec_indentifier) 
{
	if (xml.firstChild.childNodes[0].nodeName.toLowerCase() != "asset_types")	return;
	if (xml.firstChild.childNodes[1].nodeName.toLowerCase() != "current_user")	return;
	if (xml.firstChild.childNodes[2].nodeName.toLowerCase() != "assets")		return;

	if (xml.firstChild.childNodes[1].attributes.assetid == undefined
	    || xml.firstChild.childNodes[1].attributes.assetid == "") return;
	this.current_userid = xml.firstChild.childNodes[1].attributes.assetid;
	
	var asset_types = xml.firstChild.childNodes[0];
	var assets = xml.firstChild.childNodes[2];

	for (var i = 0; i < asset_types.childNodes.length; i++) {
		// get a reference to the child node
		var type_node = asset_types.childNodes[i];
		if (type_node.nodeName.toLowerCase() == "type") {

			var edit_screens = new Array();
			for (var j = 0; j < type_node.childNodes.length; j++) {
				edit_screens.push({code_name: type_node.childNodes[j].attributes.code_name, name: type_node.childNodes[j].firstChild.nodeValue});
			}


			this.asset_types[type_node.attributes.type_code] = new AssetType(type_node.attributes.type_code,
																		type_node.attributes.name,
																		type_node.attributes.version,
																		type_node.attributes.instantiable,
																		type_node.attributes.allowed_access,
																		type_node.attributes.parent_type,
																		edit_screens);

		}//end if
	}//end for

	// make sure that all the asset types know about their sub types
	for (var type_code in this.asset_types) {
		if (this.asset_types[type_code].parent_type != "asset") {
			this.asset_types[this.asset_types[type_code].parent_type].sub_types.push(type_code);
		}
	}


	this._createAssetsFromXML(assets);
	this.broadcastMessage("onAssetManagerInitialised");

}// end loadTypesFromXML()


/**
* Returns an array of the asset types code that have a parent "asset", ie are at the top of the tree
*
* @return array
*/
AssetManager.prototype.getTopTypes = function() 
{

	var top_level = new Array();

	for (var type_code in this.asset_types) {
		if (this.asset_types[type_code].parent_type == "asset") {
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
* @param boolean	load_new_links		Upon a reload from the server asks the server to supply information about any new links that the assets have
*
*/
AssetManager.prototype.loadAssets = function(assetids, call_back_obj, call_back_fn, call_back_params, force_reload, load_new_links) 
{

	if (force_reload !== true)  force_reload  = false;
	if (load_new_links !== true) load_new_links = false;

	var load_assetids = assetids.unique();

	if (!force_reload) {
	    for (var i = 0; i < load_assetids.length; i++) {
			if (this.assets[load_assetids[i]] != undefined) {
				load_assetids.splice(i, 1);
				i--;
			}
		}
	}

//	trace('Load Assets : ' + load_assetids);

	// if there are assets to load, get them from server
	if (load_assetids.length) {

		var xml = new XML();
		var cmd_elem = xml.createElement("command");
		cmd_elem.attributes.action         = "get assets";
		cmd_elem.attributes.load_new_links = (load_new_links) ? 1 : 0;
		xml.appendChild(cmd_elem);

	    for (var i = 0; i < load_assetids.length; i++) {
			var asset_elem = xml.createElement("asset");
			asset_elem.attributes.assetid = load_assetids[i];

			if (load_new_links && this.assets[load_assetids[i]] != undefined) {
				for(var j = 0; j < this.assets[load_assetids[i]].links.length; j++) {
					var link_elem = xml.createElement("child");
					link_elem.attributes.linkid  = this.assets[load_assetids[i]].links[j];
					link_elem.attributes.assetid = this.asset_links[this.assets[load_assetids[i]].links[j]].minorid;
					asset_elem.appendChild(link_elem);
				}
			}// end if

			cmd_elem.appendChild(asset_elem);
		}// end for

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
	var changes = this._createAssetsFromXML(xml.firstChild);

	if (this.tmp.load_assets[exec_indentifier].force_reload) {
		this.tmp.load_assets[exec_indentifier].obj[this.tmp.load_assets[exec_indentifier].fn](this.tmp.load_assets[exec_indentifier].params, changes.new_assets, changes.old_assets);
	} else {
		this.tmp.load_assets[exec_indentifier].obj[this.tmp.load_assets[exec_indentifier].fn](this.tmp.load_assets[exec_indentifier].params);
	}

	delete this.tmp.load_assets[exec_indentifier];

}// end loadAssetsFromXML()


/**
* Called to create the asset objects from the passed XML node tree
* Returns a object containing the old versions of assets and their new versions
*
* @param object XML_Node	assets_node		XML node whose children are asset nodes
*
* @return object (old_assets, new_assets)
*
*/
AssetManager.prototype._createAssetsFromXML = function(assets_node) 
{

	var changes = {old_assets: {}, new_assets: {}};

	for (var i = 0; i < assets_node.childNodes.length; i++) {
		// get a reference to the child node
		var asset_node = assets_node.childNodes[i];
		if (asset_node.nodeName.toLowerCase() == "asset") {
			var assetid = asset_node.attributes.assetid;

			var links = new Array();

			for (var j = 0; j < asset_node.childNodes.length; j++) {
				var linkid = asset_node.childNodes[j].attributes.linkid;
				this.asset_links[linkid] = new AssetLink(linkid, 
														 assetid, 
														 asset_node.childNodes[j].attributes.minorid, 
														 asset_node.childNodes[j].attributes.link_type);
				links.push(linkid);
			}

			// only create if it doesn't already exist
			if (this.assets[assetid] == undefined) {
				this.assets[assetid] = new Asset();

			// take backup before we update
			} else {
				changes.old_assets[assetid] = this.assets[assetid].clone();
			}

			this.assets[assetid].setInfo(	assetid, 
											asset_node.attributes.type_code, 
											asset_node.attributes.name, 
											asset_node.attributes.accessible, 
											links);

			changes.new_assets[assetid] = this.assets[assetid];

		}//end if
	}//end for

	return changes;

}// end _createAssetsFromXML()


/**
* Forces the re-retrieval of the assets information, including who it's kids are
*
* @param Array(int) assetids
*
* @access public
*/
AssetManager.prototype.reloadAssets = function(assetids) 
{
	assetids = assetids.unique();
	this.loadAssets(assetids, this, "reloadAssetsLoaded", {assetids: assetids}, true, true);
}// end reloadAssets()

/**
* Continuation of reloadAssets() after asset is loaded from server
*
* @param object params
*
* @access public
*/
AssetManager.prototype.reloadAssetsLoaded = function(params, new_assets, old_assets) 
{
	this.broadcastMessage("onAssetsReload", params.assetids, new_assets, old_assets);
}// end reloadAssetsLoaded()


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
				this.reloadAssets([params.assetid]);
			}
		break;
	}

}// end onExternalCall()

