
// Create the Class
function mcMenuContainerClass()
{

	this.top_level = new Array();
	this.num_items = 0;

}

// Make is inherit from MovieClip
mcMenuContainerClass.prototype = new MovieClip();

mcMenuContainerClass.prototype.create = function(arr)
{

	this.top_level = this._recurseCreate(arr, 0);
	var x = 0;
	for (var i = 0; i < this.top_level.length; i++) {
		var name = this.top_level[i];

		this[name]._x = x;
		this[name]._y = 0;
		this[name].show();
		x += this[name]._width;

	}// end for

}// end create()

mcMenuContainerClass.prototype._recurseCreate = function(arr, depth)
{
	var item_names = new Array();

	for(var i = 0; i < arr.length; i++) {

		this.num_items++;
		var item_name = "mi_" + this.num_items;
		trace('Name : ' + item_name);
		this.attachMovie("mcMenuItemID", item_name, this.num_items);
		this[item_name].hide();
		this[item_name].setInfo(arr[i].text, arr[i].value, (depth == 0));
		if (arr[i].kids.length) {
			this[item_name].kids = this._recurseCreate(arr[i].kids, depth + 1);
		}

		item_names.push(item_name);
		
	}// end for

	return item_names;

}// end create()

Object.registerClass("mcMenuContainerID", mcMenuContainerClass);
