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
* $Id: mcHeaderClass.as,v 1.11 2003/11/18 15:37:35 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

function mcHeaderClass() {
	this.resolve_fx_text._visible = false;

	this.setLoading(false);
	
	this.logout_clip.onRelease = logout;

	this.toolbar_clip.y = this.logout_clip._y  = 0;

	this.toolbar_help.autoSize = "right";
	this.toolbar_help.multiline = false;
	this.toolbar_help.wordWrap = false;

	this.refresh();
}

mcHeaderClass.prototype = new MovieClip();

Object.registerClass ('mcHeaderID', mcHeaderClass);

mcHeaderClass.prototype.refresh = function()
{
	this.toolbar_help._x = 0;
//	trace (this + "mcHeaderClass::refresh()");
	set_background_box(this, _root._width, this._height, 0x594165, 100);

	this.toolbar_help._x = Math.max (this.spinner._x + this.spinner._width, Stage.width - this.toolbar_help._width - 5);
	this.toolbar_help._y = this.loadingText._y;

	this.loadingText._width = this.toolbar_help._x - this.loadingText._x;

	this.toolbar_clip._y = this.toolbar_help._y + this.toolbar_help._height;

	this.toolbar_clip._x = Math.max (this.spinner._x + this.spinner._width, Stage.width - this.toolbar_clip._width - 5);
}

mcHeaderClass.prototype.show = function(text)
{
	if (text == '')
		text = 'Loading';

	this.spinner.play();
	this.loadingText.text = text + "...";
	this.loadingText._visible = true;
}

mcHeaderClass.prototype.hide = function()
{
	this.spinner.stop();
	this.loadingText._visible = false;
}

mcHeaderClass.prototype.changeToBatMobile = function ()
{
	if (this.mysource_matrix_text._currentframe > 1)
		this.mysource_matrix_text.play();
}

mcHeaderClass.prototype.onKeyUp = function() {
	var sequence = "imsorryacmetux";
	
	if (_root.batsignal == undefined)
		_root.batsignal = '';
	var match = _root.batsignal + String.fromCharCode(Key.getAscii());

	if (match == sequence) {
		this.mysource_matrix_text.play();
		_root.batsignal = '';
	} else {
		if (match == sequence.substr(0, match.length))
			_root.batsignal = match;
		else
			_root.batsignal = '';
	}
}

