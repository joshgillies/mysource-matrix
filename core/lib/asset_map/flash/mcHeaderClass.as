function mcHeaderClass() {
	this.resolve_fx_text._visible = false;

	this.setLoading(false);

	this.refresh();
}

mcHeaderClass.prototype = new MovieClip();

Object.registerClass ('mcHeaderID', mcHeaderClass);

/*
mcHeaderClass.prototype. = function()
{

}
*/

mcHeaderClass.prototype.refresh = function()
{
//	trace (this + "mcHeaderClass::refresh()");
	set_background_box(this, _root._width, this._height, 0x402F48, 100);
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

