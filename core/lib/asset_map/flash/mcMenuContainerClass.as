
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
	this._x = 0;
	this._y = 0;

	// Set ourselves up as a listener on the asset types, so we know when they have been loaded
	_root.asset_manager.addListener(this);

	// Set ourselves up as a broadcaster, so others can be notified of menu items being pressed
    ASBroadcaster.initialize(this);


}

// Make it inherit from MovieClip
mcMenuContainerClass.prototype = new MovieClip();

/**
* Event fired when the Asset Types object has finished recieving all the asset types from the server
* We can then run create()
*
*/
mcMenuContainerClass.prototype.onAssetTypesLoaded = function()
{
	trace(' -- -- -- -- Create Menu -- -- -- -- -- ');
	this.create();
}


/**
* Create the menu, with all it's items
*
*/
mcMenuContainerClass.prototype.create = function()
{
	
	this.top_level = new Array();

	var add_menu  = this._createItem("Add", "", 0);
	this[add_menu].kids = this._recurseCreateAddMenu(_root.asset_manager.getTopTypes(), 1);
	this.top_level.push(add_menu);

	var fixed_tops = this._recurseCreateFromArray(this.static_items, 0);

	this.top_level = this.top_level.concat(fixed_tops);

	this.show();

}// loadItems();

mcMenuContainerClass.prototype._recurseCreateAddMenu = function(kids, depth)
{

	var item_names = new Array();

	for (var i = 0; i < kids.length; i++) {
		trace('AddMenu : ' + kids[i]);
		var type = _root.asset_manager.types[kids[i]];

		// Create any kids, also a check to see if we have any valid kids
		var item_kids = (type.sub_types.length) ? this._recurseCreateAddMenu(type.sub_types, depth + 1) : new Array();

		// no point, if you can't create an instance and there are no kids
		if (!type.createable() && !item_kids.length) continue;

		// OK, what this is all about is that if there are sub types, 
		// then we need append " Types" to the name and remove any value
		var name  = type.name;
		var value = "add/" + type.type_code;

		var item_name = "";
		// if we have kids and we can create an instance of ourselves, then we need to add
		// ourselves to the top of our kids list, so that we can be selected normally
		if (item_kids.length && type.createable()) {
			item_name = this._createItem(name + " Types", "", depth);
			item_kids.unshift(this._createItem(name, value, depth + 1));
		} else {
			item_name = this._createItem(name, value, depth);
		}

		this[item_name].kids = item_kids;

		item_names.push(item_name);
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

mcMenuContainerClass.prototype.hideKids = function()
{
	if (this.open_items[0] != undefined) {
		this[this.open_items[0]].hideKids();
	}
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
	this.hideKids();
	trace('Item Press : ' + item.value);
	var cmds = item.value.split("/", 2);

	this.broadcastMessage("onMenuItemPress", cmds[0], cmds[1]);

}// end itemPress()

Object.registerClass("mcMenuContainerID", mcMenuContainerClass);
