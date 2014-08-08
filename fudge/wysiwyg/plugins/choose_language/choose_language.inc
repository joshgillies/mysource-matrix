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
* $Id: choose_language.inc,v 1.8 2012/08/30 00:56:51 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');


/**
* WYSIWYG Plugin - Keyword Replace
*
* Purpose
*     A WYSIWYG plugin to add keywords to the editor using a nice interface
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.8 $
* @package Fudge
* @subpackage wysiwyg
*/
class Choose_Language extends WYSIWYG_Plugin
{


	/**
	* Constructor
	*
	* @access public
	*/
	function Choose_Language(&$wysiwyg)
	{
		  ////////////////////////////////
		 //  WORK OUT THE LOCALE LIST  //
		////////////////////////////////
		//require_once dirname(__FILE__).'/../../../../core/include/init.inc';


		/*$lm = &$GLOBALS['SQ_SYSTEM']->lm;
		$curr_locale = $lm->getCurrentLocale();
		if ($curr_locale !== FALSE) {

		}*/
		$this->_add_button('chooselang','','','','');

	}//end constructor


	/**
	* Set the options for the select box as vars of the object
	*
	* @return void
	* @access public
	*/
	function print_plugin_vars()
	{
		$common_languages = Array(
								''	=> 'None',
								'en'	=> 'English',
								'es'	=> 'Spanish',
								'fr'	=> 'French',
								'it'	=> 'Italian',
								'pt'	=> 'Portuguese',
								'de'	=> 'German',
								'ru'	=> 'Russian',
								'ar'	=> 'Arabic',
								'zh'	=> 'Chinese',
								'he'	=> 'Hebrew',
								'ja'	=> 'Japanese',
								'ko'	=> 'Korean',
								'sw'	=> 'Swahili',
								'el'	=> 'Greek',
								'la'	=> 'Latin',
								'he'	=> 'Hebrew',
							);
		?>
		this.config.chooselang = {
			"-- choose language --": "",<?php
			foreach ($common_languages as $code => $name) {
				echo '"'.$name.'": "'.$code.'",'."\n";
			}
			?>
			"---------------------": "",<?php
			require_once SQ_FUDGE_PATH.'/standards_lists/languages.inc';
			$count = 1;
			foreach ($standards_lists_languages as $code => $name) {
				if (array_key_exists($code, $common_languages)) {
					$count++;
					continue;
				}
				if ($count == count($standards_lists_languages)) {
					echo '"'.$name.'": "'.$code.'"'."\n";
				} else {
					echo '"'.$name.'": "'.$code.'",'."\n";
				}
				$count++;
			}
			?>
		};
		<?php

	}//end print_plugin_vars()


	/**
	* Populate the select boxes with the options stored in the WYSIWYGs vars (set ealier)
	*
	* @return void
	* @access public
	*/
	function print_plugin_create_select()
	{
		?>
		case "chooselang" :
			options = editor.config[txt];
			cmd = txt;
		break;
		<?php

	}//end print_plugin_create_select()


	/**
	* When the toolbar is updated, change the selected option to reflect the selected text
	*
	* @return void
	* @access public
	*/
	function print_plugin_update_toolbar()
	{
		?>
		case "chooselang" :
		if (!text) {
			var options = this.config["chooselang"];
			var current_span = this.getClosest("span");
			var found = false;
			if (current_span) {
				var value = current_span.lang;
				var k = 0;
				for (var j in options) {
					if (options[j].toLowerCase() == value) {
						btn.element.selectedIndex = k;
						found = true;
						break;
					}
					++k;
				}
			}
			if (!found) {btn.element.selectedIndex = 0; }
		}
		break;
		<?php

	}//end print_plugin_update_toolbar()


	/**
	* When the select box option is changed
	*
	* @return void
	* @access public
	*/
	function print_plugin_combo_selected()
	{
		?>
		case "chooselang":
			if (value != null) {
				var current_span = this.getClosest("span");
				if (current_span) {
					if (value == "") {
						  /////////////////////
						 // Remove Lang Tag //
						/////////////////////
						var sel = this._getSelection();
						var range = this._createRange(sel);
						var span_parent = current_span.parentNode;
						for (var i = 0; i < span_parent.childNodes.length; i++) {
							var child_node = span_parent.childNodes[i];
							if (child_node.tagName == "SPAN") {
								if (child_node.innerHTML == current_span.innerHTML) {
									if (HTMLArea.is_gecko) {
										// Gecko provides a cleaner way
										if (child_node.attributes.length == 1) {
											// span tag may have only one attribute, lang. then
											// we remove the surrounding span tag itself
											if (child_node.attributes[0].name == "lang") {
												range.selectNode(child_node);
												this.insertHTML(child_node.innerHTML, range);
											}
										} else if (child_node.attributes.length > 1) {
											// span tag may have multiple attributes, then
											// we only remove lang attribute from the tag
											var attrs = child_node.attributes;
											var lang_found = false;
											var xml_lang_found = false;
											var tmp_span = "<span ";
											for (var j = 0; j < attrs.length; j++) {
												if (attrs[j].name == "lang") {
													lang_found = true;
												} else if (attrs[j].name == "xml:lang") {
													xml_lang_found = true;
												}else {
													tmp_span = tmp_span + attrs[j].name + "=\"" + attrs[j].value +"\" ";
												}
											}

											if (lang_found && xml_lang_found && attrs.length == 2) {
												range.selectNode(child_node);
												this.insertHTML(child_node.innerHTML, range);
											} else if (lang_found || xml_lang_found) {
												tmp_span += ">";
												range.selectNode(child_node);
												this.insertHTML(tmp_span + child_node.innerHTML + "</span>", range)
											}
										}
									} else if (HTMLArea.is_ie) {
										var inner_html = child_node.innerHTML
										var new_child = this._doc.createTextNode('');
										if (typeof(visual_aid_mode) != 'undefined' && visual_aid_mode) {
											var e = '<span id=wysiwyg-visual-aid-plugin-lang[^>]*>[^>]*<\/span>';
											var re = new RegExp(e, "ig");
											inner_html = inner_html.replace(re, '');
										}
										span_parent.replaceChild(new_child, child_node);
										range.pasteHTML(inner_html);
									}
								}
							}
						}
					} else {
						  ///////////////////////////////
						 // Choose The Lang Attribute //
						///////////////////////////////
						this.selectNodeContents(current_span);
						current_span.lang = value;
					}
				} else if (value != "") {
					  /////////////////////////////////////
					 // Add The Span Tag With Lang Attr //
					/////////////////////////////////////
					if (this.getSelectedHTML().length != 0) {
						this.surroundHTML("<span lang=\"" + value + "\">", "</span>");
					}
				}
			}
		break;
		<?php

	}//end print_plugin_combo_selected()


}//end class()

?>
