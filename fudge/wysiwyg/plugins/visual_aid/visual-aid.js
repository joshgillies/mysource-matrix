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
* $Id: visual-aid.js,v 1.3 2006/03/03 03:44:24 rong Exp $
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
var visual_aid_types = ["SPAN", "A", "TABLE", "TH", "TD"];
visual_aid_factory = function(tag_type, element)
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
		default:
			return null;
		break;
	}
}

// SPAN tag
visual_aid_span = function(element)
{
	this.element	= element;
	this.style		= 'padding-left: 1px; padding-right: 1px; padding-top: 1px; padding-bottom: 1px; background-color : #DDDF0D; font: 10px Verdana,Tahoma,sans-serif; font-weight: bold; border-left: 1px solid; border-right: 1px solid; border-top: 1px solid; border-bottom: 1px solid;';

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
				var e = '<span id=wysiwyg-visual-aid-plugin-lang.*<\/span>';
				var re = new RegExp(e, "ig");
				var new_html = element.innerHTML.replace(re, '');
				element.innerHTML = new_html;
				element.removeAttribute("style");
			}

		}
	}
}


// A(Anchor) tag.
visual_aid_a = function(element)
{
	this.element	= element;
	this.style		= 'padding-left: 1px; padding-right: 1px; padding-top: 1px; padding-bottom: 1px; background-color : #37DF5E; font: 10px Verdana,Tahoma,sans-serif; font-weight: bold; border-left: 1px solid; border-right: 1px solid; border-top: 1px solid; border-bottom: 1px solid;';

	this.turnOn = function()
	{
		var name_attr = element.getAttribute("name");
		var href_attr = element.getAttribute("href");
		if (HTMLArea.is_gecko) {
			if (href_attr == null && name_attr != null && name_attr != "") {
				var test_html = '<span id="wysiwyg-visual-aid-plugin-anchor" style="' + this.style + '">A "' + name_attr + '"</span>';
				element.innerHTML = test_html + element.innerHTML;
			}
		} else if (HTMLArea.is_ie) {
			if (href_attr == "" && name_attr != "") {
				var test_html = '<span id="wysiwyg-visual-aid-plugin-anchor" style="' + this.style + '">A "' + name_attr + '"</span>';
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
				var e = '<span id=wysiwyg-visual-aid-plugin-anchor.*<\/span>';
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
	this.style		= 'padding-left: 1px; padding-right: 1px; padding-top: 1px; padding-bottom: 1px; background-color : #8FC2FF; font: 10px Verdana,Tahoma,sans-serif; font-weight: bold; border-left: 1px solid; border-right: 1px solid; border-top: 1px solid; border-bottom: 1px solid;';
	this.turnOn = function()
	{
		if (HTMLArea.is_gecko) {
			var test_html = '<span id="wysiwyg-visual-aid-plugin-th" style="' + this.style + '">TH</span>&nbsp;';
			element.innerHTML = test_html + element.innerHTML;
		} else if (HTMLArea.is_ie) {
			var test_html = '<span id="wysiwyg-visual-aid-plugin-th" style="' + this.style + '">TH</span>&nbsp;';
			element.innerHTML = test_html + element.innerHTML;
		}
		HTMLArea._addClass(element, 'wysiwyg-noborders');
	}
	this.turnOff = function()
	{
		if (HTMLArea.is_gecko) {
			element.innerHTML = element.innerHTML.replace(/<span id="wysiwyg-visual-aid-plugin-th".*<\/span>(&nbsp;| )/g, '');
		} else if (HTMLArea.is_ie) {
			var e = '<span id=wysiwyg-visual-aid-plugin-th.*<\/span>&nbsp;';
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
	this.style		= 'padding-left: 1px; padding-right: 1px; padding-top: 1px; padding-bottom: 1px; background-color : #8FC2FF; font: 10px Verdana,Tahoma,sans-serif; font-weight: bold; border-left: 1px solid; border-right: 1px solid; border-top: 1px solid; border-bottom: 1px solid;';
	this.turnOn = function()
	{
		if (HTMLArea.is_gecko) {
			var test_html = '<span id="wysiwyg-visual-aid-plugin-td" style="' + this.style + '">TD</span>&nbsp;';
			element.innerHTML = test_html + element.innerHTML;
		} else if (HTMLArea.is_ie) {
			var test_html = '<span id="wysiwyg-visual-aid-plugin-td" style="' + this.style + '">TD</span>&nbsp;';
			element.innerHTML = test_html + element.innerHTML;
		}
		HTMLArea._addClass(element, 'wysiwyg-noborders');
	}
	this.turnOff = function()
	{
		if (HTMLArea.is_gecko) {
			element.innerHTML = element.innerHTML.replace(/<span id="wysiwyg-visual-aid-plugin-td".*<\/span>(&nbsp;| )/g, '');
		} else if (HTMLArea.is_ie) {
			var e = '<span id=wysiwyg-visual-aid-plugin-td.*<\/span>&nbsp;';
			var re = new RegExp(e, "ig");
			var new_html = element.innerHTML.replace(re, '');
			element.innerHTML = new_html;
		}
		HTMLArea._removeClass(element, 'wysiwyg-noborders');
	}
}
