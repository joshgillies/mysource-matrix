/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: mcToolBarClass.as,v 1.11 2003/11/18 15:37:36 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


function mcToolBarClass() {
	this.icons = new Array();
	
	ASBroadcaster.initialize(this);

	var nextDepth = 1;
	
	this.attachMovie('mcKeyIconID', 'key_icon', nextDepth++);
	this.icons.push(this.key_icon);
	this.key_icon.helpText = "Key";
	this.key_icon.onRelease = function() { 
		_root.external_call.makeExternalCall("open_legend", null);
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
		_root.external_call.makeExternalCall("open_help", null);
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
