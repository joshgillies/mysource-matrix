function mcHeaderClass() {
	this.resolve_fx_text._visible = false;

	this.setLoading(false);
	
	this.logout_clip.onRelease = logout;
	this.toolbar_clip.y = this.logout_clip._y  = 0;
	this.refresh();
}

mcHeaderClass.prototype = new MovieClip();

Object.registerClass ('mcHeaderID', mcHeaderClass);

mcHeaderClass.prototype.refresh = function()
{
	this.logout_clip._x = 0;
//	trace (this + "mcHeaderClass::refresh()");
	set_background_box(this, _root._width, this._height, 0x402F48, 100);
	this.logout_clip._x = Math.max (this.spinner._x + this.spinner._width, Stage.width - this.logout_clip._width - 10);

	this.logout_clip._y = this.loadingText._y;
	this.toolbar_clip._y = this.logout_clip._y + this.logout_clip._height;

	this.toolbar_clip._x = Math.max (this.spinner._x + this.spinner._width, Stage.width - this.toolbar_clip._width - 10);
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
	return;
	this.resolve_fx_text._x = this.mysource_matrix_text._x;
	this.resolve_fx_text._y = this.mysource_matrix_text._y;

	this.resolve_fx_text._visible = true;
	this.resolve_fx_text._alpha = 100;

	this.mysource_matrix_text._visible = false;
}

