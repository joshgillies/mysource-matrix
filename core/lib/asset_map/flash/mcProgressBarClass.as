/**
* This is an pop-up dialog box
*
*/


// Create the Class
function mcProgressBarClass()
{
	this.stop();
	this._visible = false;

	this.counter = 0;
	this.descs   = new Object();
	this.order   = new Array();

	this.intervalid = null;
	this.text_pos   = 0;

	this.bg_colour   = 0xC0C0C0;
	this.fg_colour   = 0xFFFFFF;
	this.full_width  = 230;
	this.full_height = 100;



	this.progress_text.text_color = this.fg_colour;

	this.clear();
	_root.dialog_border(this, 0, 0, this.full_width, this.full_height, false, false);
	this.beginFill(this.bg_colour, 100);
	this.lineStyle();
	this.moveTo(0, 0);
	this.lineTo(this.full_width, 0);
	this.lineTo(this.full_width, this.full_height);
	this.lineTo(0, this.full_height);
	this.lineTo(0, 0);
	this.endFill();

}

// Make it inherit from MovieClip
mcProgressBarClass.prototype = new MovieClip();

/**
* Initialises a new Options box
*
* @param string	text
*
* @access public
*/
mcProgressBarClass.prototype.show = function(desc) 
{
	var id = this.counter++;
	this.descs[id] = desc;
	this.order.push(id);

	trace("SHOW PB : " + id + " : " + desc);


	if (!this.intervalid) {
		if (this.order.length > 1) {
			this.intervalid = setInterval(this, "setText", 1000);
		} else {
			this.setText();
		}
	} 

	// centre this box in the stage
	this._x = (Stage.width  - this.full_width)  / 2;
	this._y = (Stage.height - this.full_height) / 2;
	this.gotoAndPlay(1);
	this._visible = true;

	return id;

}// end show()


/**
* Hides the dialog box
*
* @access public
*/
mcProgressBarClass.prototype.hide = function(id) 
{
	trace("HIDE PB : " + id);
	trace("HIDE PB : order before -> " + this.order);

	delete this.descs[id];
	this.order.remove_element(id);
	trace("HIDE PB : order after  -> " + this.order);

	if (this.order.length > 0) {
		if (this.descs.length < 2) {
			clearInterval(this.intervalid);
			this.intervalid = null;
			this.setText();
		}
	} else {
		this._visible = false;
		this.stop();
	}
}

/**
* Fired when the close button is pressed
*
* @access public
*/
mcProgressBarClass.prototype.closePressed = function() 
{
	if (this.call_back_obj && this.call_back_fn) {
		this.call_back_obj[this.call_back_fn](null, this.call_back_params);
	}
	this.hide();
}


/**
* Set's the text
*
* @access private
*/
mcProgressBarClass.prototype.setText = function() 
{
	this.interval_pos++;
	if (this.interval_pos >= this.order.length) this.interval_pos = 0;

	trace("SET TEXT pos  : " + this.interval_pos);
	trace("SET TEXT id   : " + this.order[this.interval_pos]);
	trace("SET TEXT desc : " + this.descs[this.order[this.interval_pos]]);

	this.progress_text.text = this.descs[this.order[this.interval_pos]];
}

Object.registerClass("mcProgressBarID", mcProgressBarClass);
