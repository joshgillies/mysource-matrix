
// Create the Class
function mcMenuContainerClass()
{

	this.dynamic_items = ["Add"];
	this.static_items  =	[
								{text: "Options",
								 value: "",
								 kids:	[
											{text: "Blah",
											 value: "options/blah1",
											 kids: []
											},
										
											{text: "Blah 2",
											 value: "options/blah2",
											 kids: []
											}
										]
								}
							];


	this.top_level  = new Array();
	this.num_items  = 0;

	this.open_items = new Array();

}

// Make is inherit from MovieClip
mcMenuContainerClass.prototype = new MovieClip();

mcMenuContainerClass.prototype.create = function(arr)
{

	var xml = new XML();
	var cmd_elem = xml.createElement("command");
	cmd_elem.attributes.action = "menu items";
	xml.appendChild(cmd_elem);

	for(var i = 0; i < this.dynamic_items.length; i++) {
		var opt = xml.createElement("item");
		opt.attributes.name = this.dynamic_items[i].toLowerCase();
		cmd_elem.appendChild(opt);
	}

	// start the loading process
	_root.server_exec.exec(xml, this, "loadItems", "options", "Loading Menu");

}

mcMenuContainerClass.prototype.loadItems = function(xml)
{

	var xml_tops   = this._recurseCreateFromXML(xml.firstChild.childNodes, 0);
	var fixed_tops = this._recurseCreateFromArray(this.static_items, 0);

	this.top_level = xml_tops.concat(fixed_tops);

	this.show();

}// loadItems();

mcMenuContainerClass.prototype._recurseCreateFromXML = function(xml_nodes, depth)
{

	var item_names = new Array();

	for (var i = 0; i < xml_nodes.length; i++) {
		// get a reference to the child node
		var item_node = xml_nodes[i];
		if (item_node.nodeName.toLowerCase() == "item") {
			var item_name = this._createItem(item_node.attributes.text, item_node.attributes.value, depth);
			if (item_node.childNodes.length) {
				this[item_name].kids = this._recurseCreateFromXML(item_node.childNodes, depth + 1);
			}
			item_names.push(item_name);
		}//end if
	}//end for

	return item_names;

}// end _recurseCreate()

mcMenuContainerClass.prototype._recurseCreateFromArray = function(arr, depth)
{
	var item_names = new Array();

	for(var i = 0; i < arr.length; i++) {
		var item_name = this._createItem(arr[i].text, arr[i].value, depth);
		if (arr[i].kids.length) {
			this[item_name].kids = this._recurseCreateFromArray(arr[i].kids, depth + 1);
		}
		item_names.push(item_name);
	}// end for

	return item_names;

}// end _recurseCreateFromArray()

mcMenuContainerClass.prototype._createItem = function(text, value, depth)
{
	this.num_items++;
	var item_name = "mi_" + this.num_items;
	this.attachMovie("mcMenuItemID", item_name, this.num_items);
	this[item_name].hide();
	this[item_name].setInfo(text, value, depth);

	return item_name;

}// end _createItem()


mcMenuContainerClass.prototype.show = function()
{

	var x = 0;
	for (var i = 0; i < this.top_level.length; i++) {
		var name = this.top_level[i];
		this[name]._x = x;
		this[name]._y = 0;
		this[name].show();
		x += this[name]._width;
	}// end for

}// show();


mcMenuContainerClass.prototype.hide = function()
{
	for (var i = 0; i < this.top_level.length; i++) {
		this[this.top_level[i]].show();
	}// end for
}// hide();



mcMenuContainerClass.prototype.itemOpen = function(item)
{

	if (this.open_items[item.depth] != undefined) {
		this[this.open_items[item.depth]].hideKids();
	}

	this.open_items[item.depth] = item._name;

}// end itemOpen()

mcMenuContainerClass.prototype.itemPress = function(item)
{
	// Hide the Open Menu
	this[this.open_items[0]].hideKids();
	trace('Item Press : ' + item.value);
	var cmds = item.value.split("/", 3);

	switch(cmds[0]) {
		case "list" :
			_root.list_container.execAction(cmds[1], cmds[2]);
		break;

		default:
			_root.showDialog("Menu Action Error", "Unknown action destination '" + cmds[0] + "'");

	}// end switch

}// end itemPress()

Object.registerClass("mcMenuContainerID", mcMenuContainerClass);
