/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mcToolBarClass.as,v 1.8 2003/10/16 01:20:49 dwong Exp $
* $Name: not supported by cvs2svn $
*/


function mcToolBarClass() {
	this.icons = new Array();
	
	ASBroadcaster.initialize(this);

	var nextDepth = 1;
	
	this.attachMovie('mcLogoutIconID', 'logout_icon', nextDepth++);
	this.icons.push(this.logout_icon);
	this.logout_icon.helpText = "Logout";
	this.logout_icon.onRelease = logout;

	this.attachMovie('mcKeyIconID', 'key_icon', nextDepth++);
	this.icons.push(this.key_icon);
	this.key_icon.helpText = "Key";
	this.key_icon.onRelease = function() { 
//		_root.dialog_box.show('Key', 'implement me!');
	}

	this.attachMovie('mcRefreshIconID', 'refresh_icon', nextDepth++);
	this.icons.push(this.refresh_icon);
	this.refresh_icon.helpText = "Full Refresh";
	this.refresh_icon.onRelease = function() { 
		_root.asset_manager.reloadAllAssets();
		_root.tabs.mail.msgs_container.refreshMail();
	}

	this.attachMovie('mcStatusIconID', 'status_icon', nextDepth++);
	this.icons.push(this.status_icon);
	this.status_icon.helpText = "Toggle Status View";
	this.status_icon.onRelease = function() { 
		this._parent.broadcastMessage("onStatusToggle");
	}

	this.attachMovie('mcHelpIconID', 'help_icon', nextDepth++);
	this.icons.push(this.help_icon);
	this.help_icon.helpText = "Help";
	this.help_icon.onRelease = function() { 
//		_root.dialog_box.show('Help', 'implement me!');
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
