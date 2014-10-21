/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: layer_handler.js,v 1.10 2012/08/30 01:09:21 ewang Exp $
*
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
		alert(js_translate('Unable to use layers on your page, sorry'));

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
		
		//Commented this out as it seems outdated, plus it stuffs up in IE when you do move first and then show as the first move gets overwritten by the move in this if statement.
		/*if (is_ie4up) {
			var top_offset = 20;
			// if the commit button is anchored to bottom of frame a custom css is used for IE (edit_ie6.css)
			// so we need to pick the y-offset for the sq-content div that is scrollable 
			// otherwise (commit button bottom of page) we pick y-offset for the body that is scrollable.
			// NOTE: sq-content is only available in the backend interface
			// so we calculate the y-offset for limbo differently
			if (document.getElementById('sq-content')) {
				scroll_top_content = document.getElementById('sq-content').scrollTop;
				scroll_top_body = document.body.scrollTop;
				if (scroll_top_content <= scroll_top_body) {
					scroll_top = scroll_top_body;
				} else {
					scroll_top = scroll_top_content;
				}
				this.move(null,top_offset + scroll_top);
			} else {
				// we are in the limbo interface AND in IE so we use this:
				scroll_top = document.documentElement.scrollTop;
				this.move(null,top_offset + scroll_top);
			}
		}*/

		this.style.visibility = (is_nav4)? "show" : "visible";
		this.style.zIndex = 1001;
	}
	 ////////////////////////////////
	// Make the layer invisible
	function hide() {
		this.style.visibility = (is_nav4)? "hide" : "hidden";
		this.style.zIndex = -1;
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
