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
	// check if something else is modal
	if (_root.system_events.inModal(this)) return false;
	// attempt to get the modal status
	if (!_root.system_events.startModal(this)) return false;


	var id = this.counter++;
	this.descs[id] = desc;
	this.order.push(id);

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
	delete this.descs[id];
	this.order.removeElement(id);

	if (this.order.length > 0) {
		if (this.descs.length < 2) {
			clearInterval(this.intervalid);
			this.intervalid = null;
			this.setText();
		}
	} else {
		_root.system_events.stopModal(this);
		this._visible = false;
		this.stop();
	}
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
	this.progress_text.text = this.descs[this.order[this.interval_pos]];

}

Object.registerClass("mcProgressBarID", mcProgressBarClass);
