/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mcHeaderClass.as,v 1.10 2003/09/26 05:26:32 brobertson Exp $
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

