
function mcToolBarClass() {
	this.attachMovie('mcLogoutIconID', 'logout_icon', 1);
	this.logout_icon.onRelease = logout;

	this.attachMovie('mcRefreshIconID', 'refresh_icon', 2);
	this.refresh_icon.onRelease = function() { 
		_root.asset_manager.reloadAllAssets();
		_root.tabs.mail.msgs_container.refreshMail();
	}

	this.refresh_icon.onRollOver = function() {
		this._parent._parent.toolbar_help._visible = true;
		this._parent._parent.toolbar_help.text = "Full Refresh";
		this._parent._parent.refresh();
	}

	this.logout_icon.onRollOver = function() {
		this._parent._parent.toolbar_help._visible = true;
		this._parent._parent.toolbar_help.text = "Logout";
		this._parent._parent.refresh();
	}

	this.logout_icon.onRollOut = this.refresh_icon.onRollOut = function() {
		this._parent._parent.toolbar_help._visible = false;
	}

	this.icons = new Array();

	this.icons.push(this.logout_icon, this.refresh_icon);
	this.icons.push();

	this.refresh();
}

mcToolBarClass.prototype = new MovieClip();

Object.registerClass ('mcToolBarID', mcToolBarClass);

/*
mcToolBarClass.prototype. = function()
{

}
*/

mcToolBarClass.prototype.refresh = function() {
	this.clear();

	var xpos = 0;
	var hpadding = 5;
	var vpadding = 2;
	for (var i = 0; i < this.icons.length; ++i) {
		xpos += hpadding;
		var icon = this.icons[i];
		trace(icon + ": " + icon._width);
		trace ("xpos: " + xpos);
		icon._x = xpos;
		icon._y = (this._height + 2 * vpadding - icon._height) / 2;
		xpos += icon._width;
	}

	set_background_box (this, xpos + hpadding, this._height + 2 * vpadding, 0x725B7D, 100);

}
