
function mcToolBarClass() {
	this.icons = new Array();
	
	ASBroadcaster.initialize(this);

	this.attachMovie('mcLogoutIconID', 'logout_icon', 1);
	this.icons.push(this.logout_icon);
	this.logout_icon.helpText = "Logout";
	this.logout_icon.onRelease = logout;

	this.attachMovie('mcStatusIconID', 'status_icon', 2);
	this.icons.push(this.status_icon);
	this.status_icon.helpText = "Toggle Status View";
	this.status_icon.onRelease = function() { 
		this._parent.broadcastMessage("onStatusToggle");
	}

	this.attachMovie('mcRefreshIconID', 'refresh_icon', 3);
	this.icons.push(this.refresh_icon);
	this.refresh_icon.helpText = "Full Refresh";
	this.refresh_icon.onRelease = function() { 
		_root.asset_manager.reloadAllAssets();
		_root.tabs.mail.msgs_container.refreshMail();
	}

	for (var i = 0; i < this.icons.length; ++i) {
		var mc = this.icons[i];
		mc.onRollOver = function() {
			this._parent._parent.toolbar_help._visible = true;
			this._parent._parent.toolbar_help.text = this.helpText;
			this._parent._parent.refresh();
		};

		mc.onRollOut = function() {
			this._parent._parent.toolbar_help._visible = false;
		};
	}

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
//		trace(icon + ": " + icon._width);
//		trace ("xpos: " + xpos);
		icon._x = xpos;
		icon._y = (this._height + 2 * vpadding - icon._height) / 2;
		xpos += icon._width;
	}

	set_background_box (this, xpos + hpadding, this._height + 2 * vpadding, 0x725B7D, 100);

}
