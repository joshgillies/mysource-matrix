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
* $Id: visual-aid.js,v 1.10.6.1 2013/07/16 02:06:14 lwright Exp $
*
*/

/**
* WYSIWYG Plugin - Visual Aid
*
* Purpose
*     A WYSIWYG plugin to allow the user to have visual
*     representation of html tags
*
* @author  Scott Kim <skim@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/

///////////////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// To add a new visual aid, you must add the tag name to visual_aid_types array
// and new case statement in visual_aid_types factory class to return a proper object
// in order to handle that particular tag
///////////////////////////////////////////////////////////////////////////////////////
var visual_aid_types = ["SPAN", "A", "TABLE", "TH", "TD", "P", "BR"];
//var visual_aid_types_single_tag = ["BR"];

visual_aid_factory = function(tag_type, element, editor)
{
	switch (tag_type) {
		case "SPAN":
			return new visual_aid_span(element);
		break;
		case "A":
			return new visual_aid_a(element);
		break;
		case "TABLE":
			return new visual_aid_table(element);
		break;
		case "TH":
			return new visual_aid_th(element);
		break;
		case "TD":
			return new visual_aid_td(element);
		break;
		case "P":
			return new visual_aid_p(element, editor);
		break;
		case "BR":
			return new visual_aid_br(element, editor);
		break;
		default:
			return null;
		break;
	}
}

// SPAN tag
visual_aid_span = function(element)
{
	this.element	= element;
	this.style		= 'padding-left: 1px; padding-right: 1px; padding-top: 1px; padding-bottom: 1px; color : #000000; background-color : #ffff80; font: 10px Verdana,Tahoma,sans-serif; font-weight: bold; border-left: 1px solid; border-right: 1px solid; border-top: 1px solid; border-bottom: 1px solid;';

	this.turnOn = function()
	{
		// Does this span include lang attribute?
		var lang_attr = element.getAttribute("lang");
		if (lang_attr != null && lang_attr != "") {
			var test_html = '<span id="wysiwyg-visual-aid-plugin-lang" style="' + this.style + '">' + lang_attr + '</span>';
			element.innerHTML = test_html + element.innerHTML;
			element.style.backgroundColor = "#DDDF0D";
		}
	}

	this.turnOff = function()
	{
		var lang_attr = element.getAttribute("lang");
		if (lang_attr != null && lang_attr != "") {
			if (HTMLArea.is_gecko) {
				element.innerHTML = element.innerHTML.replace(/<span id="wysiwyg-visual-aid-plugin-lang".*<\/span>/g, '');
				element.style.backgroundColor = "";
			} else if (HTMLArea.is_ie) {
				var e = '<span ([^>]*)id="?wysiwyg-visual-aid-plugin-lang"?.*<\/span>';
				var re = new RegExp(e, "ig");
				var new_html = element.innerHTML.replace(re, '');
				element.innerHTML = new_html;
				element.removeAttribute("style");
			}

		}
	}
}


// A(Anchor) tan.
visual_aid_a = function(element)
{
	this.element	= element;
	this.text_style	= 'vertical-align:text-center; padding-right: 1px; background-color : #FFFF80; font: 10px Verdana,Tahoma,sans-serif; font-weight: bold; border-left: 1px solid; border-right: 1px solid; border-top: 1px solid; border-bottom: 1px solid;';
	this.img_style	= 'vertical-align:text-bottom;';

	this.turnOn = function()
	{
		var name_attr = element.getAttribute("name");
		var href_attr = element.getAttribute("href");
		if (HTMLArea.is_gecko) {
			if (href_attr == null && name_attr != null && name_attr != "") {
				var test_html = '<span id="wysiwyg-visual-aid-plugin-anchor" style="' + this.text_style + '"><img style="' + this.img_style + '" src="'+ visual_aid_image_path +'/anchor.png"/>&nbsp;' + name_attr + '</span>';
				element.innerHTML = test_html + element.innerHTML;
			}
		} else if (HTMLArea.is_ie) {
			if (href_attr == "" && name_attr != "") {
				var test_html = '<span id="wysiwyg-visual-aid-plugin-anchor" style="' + this.text_style + '"><img style="' + this.img_style + '" src="'+ visual_aid_image_path +'/anchor.png"/>&nbsp;' + name_attr + '</span>';
				element.innerHTML = test_html + element.innerHTML;
			}
		}
	}

	this.turnOff = function()
	{
		var name_attr = element.getAttribute("name");
		var href_attr = element.getAttribute("href");
		if (HTMLArea.is_gecko) {
			if (href_attr == null && name_attr != null && name_attr != "") {
				element.innerHTML = element.innerHTML.replace(/<span id="wysiwyg-visual-aid-plugin-anchor".*<\/span>/g, '');
			}
		} else if (HTMLArea.is_ie) {
			if (href_attr == "" && name_attr != "") {
				var e = '<span ([^>]*)id="?wysiwyg-visual-aid-plugin-anchor"?.*<\/span>';
				var re = new RegExp(e, "ig");
				var new_html = element.innerHTML.replace(re, '');
				element.innerHTML = new_html;
			}
		}
	}
}


// TABLE tag
visual_aid_table = function(element)
{
	this.element	= element;
	this.style		= 'padding-left: 1px; padding-right: 1px; padding-top: 1px; padding-bottom: 1px; background-color : #8FC2FF; font: 10px Verdana,Tahoma,sans-serif; font-weight: bold; border-left: 1px solid; border-right: 1px solid; border-top: 1px solid; border-bottom: 1px solid;';
	this.turnOn = function()
	{
		HTMLArea._addClass(element, 'wysiwyg-noborders');
	}
	this.turnOff = function()
	{
		HTMLArea._removeClass(element, 'wysiwyg-noborders');
	}
}


// TH tag
visual_aid_th = function(element)
{
	this.element	= element;
	this.style		= 'vertical-align:text-bottom;';
	this.turnOn = function()
	{
		if (HTMLArea.is_gecko) {
			var test_html = '<img id="wysiwyg-visual-aid-plugin-th" style="' + this.style + '" src="'+ visual_aid_image_path +'/th.png" />&nbsp;';
			element.innerHTML = test_html + element.innerHTML;
		} else if (HTMLArea.is_ie) {
			var test_html = '<img id="wysiwyg-visual-aid-plugin-th" style="' + this.style + '" src="'+ visual_aid_image_path +'/th.png" />&nbsp;';
			element.innerHTML = test_html + element.innerHTML;
		}
		HTMLArea._addClass(element, 'wysiwyg-noborders');
	}
	this.turnOff = function()
	{
		if (HTMLArea.is_gecko) {
			element.innerHTML = element.innerHTML.replace(/<img id="wysiwyg-visual-aid-plugin-th"([^>]*)>(&nbsp;| )/g, '');
		} else if (HTMLArea.is_ie) {
			var e = '<img ([^>]*)id="?wysiwyg-visual-aid-plugin-th"?.*>&nbsp;';
			var re = new RegExp(e, "ig");
			var new_html = element.innerHTML.replace(re, '');
			element.innerHTML = new_html;
		}
		HTMLArea._removeClass(element, 'wysiwyg-noborders');
	}
}


// TD tag
visual_aid_td = function(element)
{
	this.element	= element;
	this.style		= 'vertical-align:text-bottom;';
	this.turnOn = function()
	{
		if (HTMLArea.is_gecko) {
			var test_html = '<img id="wysiwyg-visual-aid-plugin-td" style="' + this.style + '" src="'+ visual_aid_image_path +'/td.png" />&nbsp;';
			element.innerHTML = test_html + element.innerHTML;
		} else if (HTMLArea.is_ie) {
			var test_html = '<img id="wysiwyg-visual-aid-plugin-td" style="' + this.style + '" src="'+ visual_aid_image_path +'/td.png" />&nbsp;';
			element.innerHTML = test_html + element.innerHTML;
		}
		HTMLArea._addClass(element, 'wysiwyg-noborders');
	}
	this.turnOff = function()
	{
		if (HTMLArea.is_gecko) {
			element.innerHTML = element.innerHTML.replace(/<img id="wysiwyg-visual-aid-plugin-td"([^>]*)>(&nbsp;| )/g, '');
		} else if (HTMLArea.is_ie) {
			var e = '<img ([^>]*)id="?wysiwyg-visual-aid-plugin-td"?.*>&nbsp;';
			var re = new RegExp(e, "ig");
			var new_html = element.innerHTML.replace(re, '');
			element.innerHTML = new_html;
		}
		HTMLArea._removeClass(element, 'wysiwyg-noborders');
	}
}


// P tag
visual_aid_p = function(element, editor)
{
	this.editor 	= editor;
	this.element	= element;
	this.style		= 'vertical-align:text-bottom;';
	this.turnOn = function()
	{
		if (HTMLArea.is_gecko) {
			var test_html = '<img id="wysiwyg-visual-aid-plugin-p" style="' + this.style + '" src="'+ visual_aid_image_path +'/paragraph.png" />&nbsp;';
			element.innerHTML = element.innerHTML + test_html;
		} else if (HTMLArea.is_ie) {
			var test_html = '<img id="wysiwyg-visual-aid-plugin-p" style="' + this.style + '" src="'+ visual_aid_image_path +'/paragraph.png" />&nbsp;';
			element.innerHTML = element.innerHTML + test_html;
		}
	}
	this.turnOff = function()
	{
		if (HTMLArea.is_gecko) {
			element.innerHTML = element.innerHTML.replace(/<img id="wysiwyg-visual-aid-plugin-p"([^>]*)>(&nbsp;| )/g, '');
		} else if (HTMLArea.is_ie) {
			var e = '<img ([^>]*)id="?wysiwyg-visual-aid-plugin-p"?.*>&nbsp;';
			var re = new RegExp(e, "ig");
			var new_html = element.innerHTML.replace(re, '');
			element.innerHTML = new_html;
		}
	}
}


// BR tag
visual_aid_br = function(element, editor)
{
	this.editor 	= editor;
	this.element	= element;
	this.style		= 'vertical-align:text-bottom;';
	this.turnOn = function()
	{
		var br = this.editor._doc.createElement("br");
		var img = this.editor._doc.createElement("img");
		img.style.position = "absolute";
		img.id = "wysiwyg-visual-aid-plugin-br";
		img.src = visual_aid_image_path + '/break.png';

		if (HTMLArea.is_gecko) {
			// Replace the current BR element with the visual aid SPAN element
			var parent = element.parentNode;
			parent.replaceChild(img, element);

			// Append a new BR tag at the end
			if (img.nextSibling != null) {
				parent.insertBefore(br, img.nextSibling);
			} else {
				parent.appendChild(br);
			}
		} else if (HTMLArea.is_ie) {
			// Replace the innerHTML
			element.replaceNode(img);

			// Append a new BR tag at the end
			img.insertAdjacentElement("afterEnd", br);
		}
	}
	this.turnOff = function()
	{
		var parent = element.parentNode;
		if (element.previousSibling != null) {

			if (element.previousSibling.id == "wysiwyg-visual-aid-plugin-br") {
				parent.removeChild(element.previousSibling);
			}
		}
	}
}
