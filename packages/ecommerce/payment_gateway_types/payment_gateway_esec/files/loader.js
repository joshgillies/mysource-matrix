/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: loader.js,v 1.1.1.1 2004/10/13 01:31:48 arailean Exp $
* $Name: not supported by cvs2svn $
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
		document.write('<iframe id="'+this.id+'_frame" style="visibility:hidden;z-index:500;position:absolute;bottom:0px;height:100%;width:100%;left:0px;top:0px;right:0px;filter:alpha(opacity=0);moz-opacity:0;opacity:0" border="0" frameborder="0"></iframe>');
		document.write('<table id="'+this.id+'_table" bgcolor="'+this.color+'" border="1" cellpadding="0" cellspacing="0" style="visibility:hidden;z-index:501;position:absolute;bottom:0px;height:100%;width:100%;left:0px;top:0px;right:0px;filter:alpha(opacity=90);moz-opacity:0.9;opacity:0.9"><tr><td align="center"><img alt="loader" src="'+this.imagepath+'" height="'+this.imageheight+'" width="'+this.imagewidth+'"><br /><b style="font: 9pt Tahoma">'+this.text+'</b></td></tr></table>');
	}//end print()


/**
* shows loader on the screen
*
* @return void
* @access public
*/
	this.show = function() {
		document.getElementById(this.id+"_frame").style.visibility = "visible";
		document.getElementById(this.id+"_table").style.visibility = "visible";
	}//end show()


/**
* hides loader
*
* @return void
* @access public
*/
	this.hide = function() {
		document.getElementById(this.id+"_frame").style.visibility = "hidden";
		document.getElementById(this.id+"_table").style.visibility = "hidden";
	}//end hide()

}//end class Loader()
