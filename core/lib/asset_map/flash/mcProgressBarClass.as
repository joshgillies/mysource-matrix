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
* $Id: mcProgressBarClass.as,v 1.11 2003/11/18 15:37:35 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

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
	_root.header.refresh();
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
//	trace ("settext called");
//	trace(this.order);

	var first = true;
	var order = this.order.clone();

	this.progress_text.text = '';

	var baseText = '';

	for (var i = 0; i < order.length; ++i) {
		if (!first) {
			baseText += " + ";
		}
		baseText += this.descs[order[(this.progress_pos + i) % order.length]];
		first = false;
	}

	this.progress_text.text = baseText;
	while (this.progress_text.textWidth > this.progress_text._width) {
		baseText = baseText.substr(0, baseText.length - 1);
		if (baseText.length == 0)  {
			this._progress_text.text = "";
			break;
		}
		this.progress_text.text = baseText + "...";
	}

	this.progress_pos = (this.progress_pos + 1) % order.length;

}

Object.registerClass("mcProgressBarID", mcProgressBarClass);
