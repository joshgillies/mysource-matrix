/**
* Tabs
*
* This is the container for looking after all the tabs
*
*/

#include "mcTabButtonClass.as"
#include "mcTabContentAreaClass.as"

// Create the Class
function mcTabsClass()
{
	this.tabs = new Array();  // name of all the tabs that we have
	this.current_tab = null;  // reference to the contents area of the current tab

	this.tab_height = null;   // set when the first tab is added

	this.dims = {w: 0, h: 0};

	this._x = 0;
	this._y = 0;

}

// Make it inherit from Nested Mouse Movements MovieClip
mcTabsClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS);
mcTabsClass.prototype.colours = {
									normal:   { fg: 0x000000, bg: 0xB1BDCD },
									selected: { fg: 0x000000, bg: 0xE0E0E0 }
								 };

/**
* Creates a tab using the passed export name and set's it to the passed name
*
* @param string	tab_type	the export name of the tab to add (eg mcTabAssetTreeID)
* @param string	name		the code accesable name under this object to assign the new tab too
* @param string	label		the label to show on the tab
*
* @public
*/
mcTabsClass.prototype.addTab = function(tab_type, name, label)
{
	// bugger off if we already have a tab by this name
	if (this.tabs.search(name) != null) return;

	var depth = (this.tabs.length * 2);
	this.attachMovie(tab_type, name, depth + 1);

	this.attachMovie("mcTabButtonID", "tab_button_" + name, depth + 2);

	this["tab_button_" + name].setLabel(label);
	this["tab_button_" + name]._visible = true;
	this["tab_button_" + name]._y = 0;

	if (this.tabs.length > 0) {
		var last_button_name = "tab_button_" + this.tabs[this.tabs.length - 1];
		// + 1 is for black line
		this["tab_button_" + name]._x = this[last_button_name]._x + 1 + this[last_button_name]._width;
	} else {
		this["tab_button_" + name]._x = 0; 
	}

	this.tabs.push(name);

	this[name]._x = 0;
	this[name]._y = this["tab_button_" + this.tabs[0]]._height;

	if (this.current_tab == null) this.setCurrentTab(name);
	if (this.tab_height  == null) this.tab_height = this["tab_button_" + name]._height;

}// addTab();

/**
* Set's the current tab being displayed
*
* @param string	name	the name of the tab
* 
* @access private
*/
mcTabsClass.prototype.setCurrentTab = function(name)
{
	// bugger off if we don't know about this tab
	if (this.tabs.search(name) == null) return;
	
	if (this.current_tab != null) {
		this["tab_button_" + this.current_tab._name].unselect();
		this.current_tab._visible = false;
	}

	this["tab_button_" + name].select();
	this.current_tab = this[name];
	this.current_tab._visible = true;

	this.refreshTab();

}// _setCurrentTab()


/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
* @access public
*/
mcTabsClass.prototype.setSize = function(w, h)
{
	if (this.tabs.length == 0) return;
	this.dims.w = w;
	this.dims.h = h;

	set_background_box(this, w, h, this.colours.normal.bg);


	this.lineStyle(1, 0x000000);
	for(var i = 0; i < this.tabs.length; i++) {
		var line_x = this["tab_button_" + this.tabs[i]]._x + this["tab_button_" + this.tabs[i]]._width;
		trace("LINE X : " + line_x + " H : " + line_h);
		this.moveTo(line_x, 0);
		this.lineTo(line_x, this.tab_height);

//		this.beginFill(0x000000, 100);
//		// This is commented out because when we try and explicitly set it, 
//		// an extra 2 pixels gets added to the width of the MC for no f!@#$ing reason
//	//	this.lineStyle();
//		var line_x = this["tab_button_" + name]._x + this["tab_button_" + name]._width;
//		var line_h = this["tab_button_" + this.tabs[0]]._height;
//		trace("LINE X : " + line_x + " H : " + line_h);
//		this.moveTo(line_x,     0);
//		this.lineTo(line_x + 1, 0);
//		this.lineTo(line_x + 1, line_h);
//		this.lineTo(line_x,     line_h);
//		this.lineTo(line_x,     0);
//		this.endFill();
	}

	this.refreshTab();

}// setSize()


/**
* Refreshes the current tabs display
*
* @access public
*/
mcTabsClass.prototype.refreshTab = function()
{
	if (this.current_tab == null) return;

	// then just set the size of current tab contents area
	this.current_tab.setSize(this.dims.w, this.dims.h - this.tab_height);
	this[name]._visible = true;

}// refreshTab()




Object.registerClass("mcTabsID", mcTabsClass);
