/**
* This is an pop-up dialog box
*
*/


// Create the Class
function mcProgressBarClass()
{
	this.spinner = null;
	this._visible = false;

	this.counter = 0;
	this.descs   = new Object();
	this.order   = new Array();

	this.intervalid = null;
}



// Make it inherit from MovieClip
mcProgressBarClass.prototype = new MovieClip();

mcProgressBarClass.prototype.init = function(spinner, progress_text) 
{
	this.spinner = spinner;
	this.progress_text = progress_text;
}

/**
* Makes the spinner thing happen.
*
* @param string	text
*
* @access public
*/
mcProgressBarClass.prototype.show = function(desc) 
{
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
	
	this.spinner.play();

	return id;
}


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
		this.spinner.gotoAndStop(1);
		this.progress_text.text = '';
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
