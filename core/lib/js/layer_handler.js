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
* $Id: layer_handler.js,v 1.3 2003/11/18 15:37:37 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/*

#######################################################################
## Requires : detect.js
#######################################################################

  ##############################################################
 # Inspired by the Dan Steinman - http://www.dansteinman.com/ #
##############################################################

*/


function Layer_Handler(div_id, top, right, bottom, left) {

	if (is_major < 4) {
		alert('Unable to use layers on you page, sorry');
		return;
	}

	if (is_dom) {
		this.layer = document.getElementById(div_id);
		// if we can't find the layer die;
		if (this.layer == null) {
			this.layer_OK = false;
			return this;
		}
		this.layer_OK = true;
		this.style = this.layer.style;
		this.x = this.style.left;
		this.y = this.style.top;
		this.w = this.style.width;
		this.h = this.style.height;

	} else if (is_nav4up) {
		// if we can't find the layer die;
		if (typeof document.layers[div_id] == 'undefined') {
			this.layer_OK = false;
			return this;
		}

		this.layer_OK = true;
		this.layer = document.layers[div_id];
		this.style = document.layers[div_id];
		this.x = this.layer.left;
		this.y = this.layer.top;
		this.w = this.layer.clip.width;
		this.h = this.layer.clip.height;

	} else if (is_ie4up) {
		// if we can't find the layer die;
		if (typeof document.all[div_id] == 'undefined') {
			this.layer_OK = false;
			return this;
		}

		this.layer_OK = true;
		this.layer = document.all[div_id];
		this.style = document.all[div_id].style;
		this.x = this.layer.offsetLeft;
		this.y = this.layer.offsetTop;
		this.w = (is_ie4) ? this.style.pixelWidth  : this.layer.offsetWidth;
		this.h = (is_ie4) ? this.style.pixelHeight : this.layer.offsetHeight;
	}


	  ////////////////////////////
	 // FUNCTION DECLARATIONS  //
	////////////////////////////

	this.move  = move;
	this.show  = show;
	this.hide  = hide;
	this.clip  = clip;
	this.write = write;

	this.clip(top, right, bottom, left);

	return this;

	 /////////////////////////////////////////
	// Move the layer to some specified place
	function move(x,y) {
		if (x != null) {
			this.x = x;
			if      (is_dom) this.style.left      = this.x + "px";
			else if (is_nav) this.style.left      = this.x;
			else             this.style.pixelLeft = this.x;

		}// end if

		if (y != null) {
			this.y = y;
			if      (is_dom) this.style.top      = this.y + "px";
			else if (is_nav) this.style.top      = this.y;
			else        this.style.pixelTop = this.y;
		}// end if

	}// end move()

	 ////////////////////////////////
	// Make the layer visible 
	function show() {
		this.style.visibility = (is_nav4)? "show" : "visible";
	}
	 ////////////////////////////////
	// Make the layer invisible 
	function hide() {
		this.style.visibility = (is_nav4)? "hide" : "hidden";
	}

	 ////////////////////////////////////////
	// Clip the layer to a certain size
	function clip(top, right, bottom, left) {

		// get the current clip values
		var clip_values = new Object();
		if (is_dom || is_ie) {
			// grab the 4 pixel values from the string
			var re = /rect\(([0-9]*)px ([0-9]*)px ([0-9]*)px ([0-9]*)px\)/i;
			var result = re.exec(this.style.clip);

			clip_values["top"]    = (result) ? result[1] : 0;
			clip_values["right"]  = (result) ? result[2] : 0;
			clip_values["bottom"] = (result) ? result[3] : 0;
			clip_values["left"]   = (result) ? result[4] : 0;

		} else {

			clip_values["top"]    = this.style.clip.top;
			clip_values["right"]  = this.style.clip.right;
			clip_values["bottom"] = this.style.clip.bottom;
			clip_values["left"]   = this.style.clip.left;

		}// end if

		if (top    != null) clip_values["top"]    = top;
		if (right  != null) clip_values["right"]  = right;
		if (bottom != null) clip_values["bottom"] = bottom;
		if (left   != null) clip_values["left"]   = left;

		
		if (is_dom || is_ie) {
			this.style.clip = "rect("
							+ clip_values["top"]    + "px "
							+ clip_values["right"]  + "px "
							+ clip_values["bottom"] + "px "
							+ clip_values["left"]   + "px)";

		} else if (is_nav4up) {
			this.style.clip.top    = clip_values["top"];
			this.style.clip.right  = clip_values["right"];
			this.style.clip.bottom = clip_values["bottom"];
			this.style.clip.left   = clip_values["left"];

		}// end if

	}// end clip()

	 ////////////////////////////////////////////////////////////
	// Write some html to the layer, replacing current contents
	function write(html) {
		if (is_dom || is_ie) {
			this.layer.innerHTML = html;
		} else {
			this.layer.document.open();
			this.layer.document.write(html);
			this.layer.document.close();

		}// end if

	}// end write

}// end Layer_Handler()
