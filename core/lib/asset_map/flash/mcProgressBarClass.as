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

	this.progress_pos = 0;
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
			this.intervalid = setInterval(this, 'setText', 500);
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
		if (this.order.length < 2) {
			clearInterval(this.intervalid);
			this.intervalid = null;
			this.setText();
		}
	} else {
		_root.system_events.stopModal(this);
		this.spinner.stop();
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
	trace ("settext called");
	trace(this.order);

	var first = true;

	this.progress_text.text = '';

	for (var i = 0; i < this.order.length; ++i) {
		if (!first) {
			this.progress_text.text += " + ";
		}
		this.progress_text.text += this.descs[this.order[(this.progress_pos + i) % this.order.length]];
		first = false;
	}

	this.progress_pos = (this.progress_pos + 1) % this.order.length;

}

Object.registerClass("mcProgressBarID", mcProgressBarClass);
