<?php
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
* $Id: replace_text.inc,v 1.15 2012/08/30 00:56:52 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');

/**
* WYSIWYG Plugin - Format Text
*
* Purpose
*		A WYSIWYG plugin to separate text replace
*		from other formatting functions
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.15 $
* @package Fudge
* @subpackage wysiwyg
*/
class Replace_Text extends WYSIWYG_Plugin
{


	/**
	* Constructor
	* Adds the buttons for this plugins functions
	*
	* @returns object
	* @access public
	*/
	function replace_text()
	{
		$this->_add_button('replacetext','ReplaceText',translate('Replace Text'),'false','11');

	}//end replace_text()


	/**
	* Open the Replace Text plugin dialog
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_click()
	{
		?>
		case "replacetext":
				this._popupDialog("ReplaceText", "<?php echo $this->get_popup_href('replace_text.php')?>", 440, 470, true, function(types){
					if (types == null) return false;
					var reg = null;
					var rep = null;
					var selected_html = types[0];
					if (types[0] && editor.getSelectedHTML().length != 0) {
						selected_html = true;
						var curHTML = editor.getSelectedHTML();
						// Remove the line breaks in the html content (see bug #5704)
						if (HTMLArea.is_ie) {
							curHTML = curHTML.replace(/\r\n/g, "");
						}
					} else if (!types[0]) {
						selected_html = false;
						var curHTML = editor.getHTML();
					} else {
						alert("Selected text is not defined. Replace Text action aborted.");
						return false;
					}
					if (HTMLArea.is_ie) {
						// IE removes the qoutes from some speical attribute values. Hence we add quotes here to the 
						// unquoted attribute values so that the "replacement text" feature here works expectedly.
					    var tags = curHTML.match(/<(?!\/)[^>]+>/g);
						for(var i=0; i<tags.length; i++) {
							var tag = tags[i];
							curHTML = curHTML.replace(tag, tag.replace(/(\s[^\s<>="'`]+\s*=\s*)([^\s<>"\'`]+)/g, '$1"$2"'));
						}//end for
					}

					var HTMLtags = "!--|a|abbr|above|acronym|address|applet|array|area|b|base|basefont|bdo|bgsound|big|blink|blockquote|body|box|br|blink|button|caption|center|cite|code|col|colgroup|comment|dd|del|dfn|dir|div|dl|dt|em|embed|fieldset|fig|font|form|frame|frameset|h1|h2|h3|h4|h5|h6|head|hr|html|i|id|iframe|ilayer|img|input|ins|isindex|kbd|label|layer|legend|li|link|listing|map|marquee|menu|meta|multicol|nextid|nobr|noframes|nolayer|note|noscript|object|ol|option|keygen|optgroup|p|param|pre|q|quote|range|root|s|samp|script|select|small|sound|spacer|span|sqrt|strike|strong|style|sub|sup|table|tbody|td|text|textarea|tfoot|th|thead|title|tr|tt|u|ul|var|wbr|xmp";
					var bullet = unescape("%B7");
					var shy    = unescape("%AD");
					for (var i=1;i<types.length;i++)
						if (types[i])
							{
							// Bug #3204  - Remove Word Document HTML Clipboard Tags, that get pasted through on Firefox 3
							localreg = new RegExp("<link rel=\"[^\"]*\" href=\"file[^\"]*\">", "gi");
							localrep = "";
							curHTML = curHTML.replace(localreg, localrep);
							wordreg = new RegExp("<w\:[^>]*>(.*?)<\/w\:[^>]*>", "gi");
							
							wordrep = "";
							curHTML = curHTML.replace(wordreg, wordrep);
							switch (i) {
								case 1	: {
									reg = new RegExp("<\/?font ?[^>]*>", "gi");
									rep = "";
									break;
								}
								case 2	: {
									//#3827 -- using regexp literal 
									reg = /(\s|&nbsp;){2,}/gi;
									rep = "$1";
									break;
								}
								case 3	: {
									reg = new RegExp("<(?!(\/?(" + HTMLtags + ")[> ]))([^>]*)>", "gi");
									rep = "";
									break;
								}
								case 4	: {
									reg = new RegExp("(<p[^>]*>)?(<!--\[if !supportLists\]-->)?(<span[^>]*>)*("+bullet+"|\u00c2\u00b7|&middot\;)(<\/span>|<span[^>]*>(\s|&nbsp\;)*<\/span>)*(<!--\[endif\]-->)?(.*?)(<o\:p>.*?<\/o\:p>)?(<\/p>)?", "gi");
									rep = "<li>$8";
									break;
								}
								case 5	: {
									reg = new RegExp("(&shy;?|\u00c2\u00ad|"+shy+")", "gi");
									rep = "";
									break;
								}
								case 6	: {
									reg = new RegExp("( style='.*?'| style=\".*?\")", "gi");
									rep = "";
									break;
								}
								case 7	: {
									reg = new RegExp(" class=\"?[^\">]*[\"]?", "gi");
									rep = "";
									break;
								}
								case 8	: {
									reg = new RegExp("<(table|/table|tr|tbody|/tbody|td|th) ?[^>]*>", "gi");
									rep = "";
									curHTML = curHTML.replace(reg, rep);
									reg = new RegExp("<(/tr|/td|/th)>", "gi");
									rep = "<br />";
									break;
								}
								case 9	: {
									reg = new RegExp("<\/?span( [^>]*>|>)", "gi");
									rep = "";
									break;
								}
								case 10	: {
									reg = new RegExp("<([A-Z][A-Z0-9]*)( [^>]*)?>(&nbsp;| |\n|\t)*<\/\\1>", "gi");
									rep = "";
									break;
								}
								case 11	: {
									reg = new RegExp('<([^/ >]+)[^>]*?([^>]*?( (src|href)="?[^>"]*"?)[^>]*?)*[^>]*?>', "gi");
									rep = "<$1$3>";
									break;
								}
								default	: return false;
							}
							// BUG#928 - special condition to allow empty anchor tag
							if (i == 10) {
									reg2 = new RegExp("(<A NAME[^>]*?>)(&nbsp;| |\n|\t)*(</A>)", "gi");
									rep2 = "$1matrix_anchor_tmp$3"
									curHTML = curHTML.replace(reg2, rep2);
									reg2 = new RegExp("(<A ID[^>]*?>)(&nbsp;| |\n|\t)*(</A>)", "gi");
									rep2 = "$1matrix_anchor_tmp$3"
									curHTML = curHTML.replace(reg2, rep2);
							}
							curHTML = curHTML.replace(reg, rep); 		
							if (i == 10) {
									reg3 = new RegExp("(<A NAME[^>]*?>)matrix_anchor_tmp(</A>)", "gi");
									rep3 = "$1$2"
									curHTML = curHTML.replace(reg3, rep3);
									reg3 = new RegExp("(<A ID[^>]*?>)matrix_anchor_tmp(</A>)", "gi");
									rep3 = "$1$2"
									curHTML = curHTML.replace(reg3, rep3);
							}
						}
					if (selected_html) {
						if (types.length>0) {
							editor.insertHTML(curHTML);
						}
					} else {
						if (types.length>0) editor.setHTML(curHTML);
					}
					
				}, null);
		break;
		<?php

	}//end print_plugin_button_click()


}//end class

?>
