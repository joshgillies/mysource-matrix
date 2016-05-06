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
* $Id: loader.js,v 1.1 2013/06/25 02:51:33 cupreti Exp $
*
*/

/**
* Loader class. Constructor
*
* @param	id			unique ID for loader objects
* @param	color		color of the background [optional][default: #FFFFFF]
* @param	text		text in the loader [optional][default: 'Loading...']
* @param	imagepath	path of the image for loader [optional][default: 'i/loader.gif']
*
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @return null
* @access public
*/
function Loader(id, color, text, imagepath, imageheight, imagewidth)
{
	this.id = id;
	if (typeof color		== "undefined") this.color			= "#FFFFFF";
	else this.color = color;
	if (typeof text			== "undefined") this.text 			= "Loading...";
	else this.text = text;
	if (typeof imagepath	== "undefined") this.imagepath		= "i/loader.gif";
	else this.imagepath = imagepath;
	if (typeof imageheight	== "undefined") this.imageheight	= 50;
	else this.imageheight = imageheight;
	if (typeof imagewidth	== "undefined") this.imagewidth		= 50;
	else this.imagewidth = imagewidth;


/**
* output into HTML necessary objects
*
* @return void
* @access public
*/
	this.print = function() {
		document.write('<iframe id="'+this.id+'_frame" border="0" frameborder="0" class="loader_overlay"></iframe>');
		document.write('<table id="'+this.id+'_table" bgcolor="'+this.color+'" border="1" class="loader_overlay"><tr><td align="center"><img alt="loader" src="'+this.imagepath+'" height="'+this.imageheight+'px" width="'+this.imagewidth+'px"><br /><span>'+this.text+'</span></td></tr></table>');
	}//end print()


/**
* shows loader on the screen
*
* @return void
* @access public
*/
	this.show = function() {
		document.getElementById(this.id+"_frame").style.display = "inline-table";
		document.getElementById(this.id+"_table").style.display = "inline-table";
	}//end show()


/**
* hides loader
*
* @return void
* @access public
*/
	this.hide = function() {
		document.getElementById(this.id+"_frame").style.display = "none";
		document.getElementById(this.id+"_table").style.display = "none";
	}//end hide()

}//end class Loader()
