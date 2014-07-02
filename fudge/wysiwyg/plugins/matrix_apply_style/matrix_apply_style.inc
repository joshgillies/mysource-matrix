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
* $Id: matrix_apply_style.inc,v 1.28 2013/02/08 05:52:52 ewang Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');


/**
* WYSIWYG Plugin - Matrix Apply Style
*
* Purpose
*     A WYSIWYG plugin to apply a style from the effective design
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.28 $
* @package MySource_Matrix
* @subpackage wysiwyg
*/
class Matrix_Apply_Style extends WYSIWYG_Plugin
{

	/**
	* A list of classess to show in the drop down
	*
	* @var array
	*/
	var $_css_classes = Array();


	/**
	* Constructor
	*
	* @access public
	*/
	function Matrix_Apply_Style(&$wysiwyg)
	{
		  ///////////////////////////////
		 //  WORK OUT THE STLYE LIST  //
		///////////////////////////////
		require_once dirname(__FILE__).'/../../../../core/include/init.inc';

		// check for any classes declared for current design
		$classes = $this->getDesignClasses();
		if (!empty($classes)) {
			foreach ($classes as $class_name => $short_name) {
				$this->_css_classes[$class_name] = $short_name;
			}
		}

		// if there weren't any classes defined for design, we are getting some out from ccs file 
		if (empty($this->_css_classes) && isset($GLOBALS['sq_bodycopy_cssids'])) {
			foreach ($GLOBALS['sq_bodycopy_cssids'] as $css_assetid) {
				$css = $GLOBALS['SQ_SYSTEM']->am->getAsset($css_assetid);
				$css_file = $css->getExistingFile();
				$stylesheet_src = '';
				if (!empty($css_file)) {
					$stylesheet_src = $css_file['path'];
				}

				// set the stylesheet to use in the editor
				$apply_style = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_WYSIWYG_APPLY_DESIGN_STYLE');
				if($apply_style)
				    $wysiwyg->set_stylesheet($css->getURL());
				
				// if it exists, parse the stylesheet to get a list of styles we can use
				if (file_exists($stylesheet_src)) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					$stylesheet = preg_replace('%[\r\n]+%',' ',file_to_string($stylesheet_src));
					$stylesheet = preg_replace('/\/\*.*\*\//sU', '',$stylesheet);
					$matches = Array();
					preg_match_all('%\s*([^\{]+)\{[^\}]+\}%', $stylesheet, $matches);
					$class_strings = $matches[1];

					// $class_strings is now a list of all the class and their variations
					// in the stylesheet - so we need to parse this again to get unique
					// class names without variations
					foreach ($class_strings as $class_string) {
						$variations = explode(',', $class_string);
						foreach ($variations as $class) {
							// non-standard class begin with a '.'
							list($class) = explode(':',trim($class));
							if (substr($class,0,1) == '.') $this->_css_classes[trim($class,'.')] = trim($class,'.');
						}
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($css);
				unset($css);
			}
		}

		$this->_add_button('formatblock','','','','');
		if (!empty($this->_css_classes)) $this->_add_button('applystyle','','','','');

	}//end constructor


	/**
	* Set the options for the select boxes as vars of the object
	*
	* @return void
	* @access public
	*/
	function print_plugin_vars()
	{
		?>
		this.config.applystyle = {
		<?php

		if (!empty($this->_css_classes)) {
			?>"-- select css style --": "",<?php
			$string = '';
			foreach ($this->_css_classes as $class_name => $cn) echo '"     '.str_replace('"', '\"', $cn).'": "'.str_replace('"', '\"', $class_name).'",';
			?>"-- remove css style --": "no_style"<?php
		}

		?>
		};

		this.config.formatblock = {
			"Heading 1": "h1",
			"Heading 2": "h2",
			"Heading 3": "h3",
			"Heading 4": "h4",
			"Heading 5": "h5",
			"Heading 6": "h6",
			"Normal"   : "p",
			"Address"  : "address",
			"Formatted": "pre"
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
		case "applystyle":
		case "formatblock":
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
		case "applystyle":
			if (!text) {
				var options = this.config["applystyle"];
				var current_span = this.getClosest("span");
				var found = false;
				if (current_span) {
					var value = current_span.className;
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
				if (!found) { btn.element.selectedIndex = 0; }
			}
		break;

		case "formatblock":
			if (!text) {
				if (!doc) break;
				var value = '';
				try {
					var value = ("" + doc.queryCommandValue(cmd)).toLowerCase();
				} catch (e) {}
				if (!value) {
					break;
				}
				btn.element.selectedIndex = 0;
				var options = this.config["formatblock"]; // HACK!!
				var k = 0;
				for (var j in options) {
					// FIXME: the following line is scary.
					if ((j.toLowerCase() == value) ||
						(options[j].substr(0, value.length).toLowerCase() == value)) {
						btn.element.selectedIndex = k;
						break;
					}
					++k;
				}
			}
		break;
		<?php

	}//end print_plugin_update_toolbar()


	/**
	* When the select box option is changed, apply changes to selected HTML
	*
	* @return void
	* @access public
	*/
	function print_plugin_combo_selected()
	{
		?>
		case "applystyle":
			if (value == '') {
				break;
			}
			range = this._createRange(this._getSelection());

			if (HTMLArea.is_ie) {
				
				///////// INTERNET EXPLORER /////////
				
				// if the selection is empty
				if (range.boundingWidth == 0) {
					break;
				}				

				// get the parent element of the range, working around an IE bug
				// with parentELement()
				var parent = range.parentElement();
				if ((parent.nodeType == 1) && (parent.tagName.toLowerCase() == 'p') && (parent.innerHTML == range.htmlText)) {
					parent.className = value;
				} else {
					while (true) {
						var testRange = range.duplicate();
						testRange.moveToElementText(parent);
						if (testRange.inRange(range)) {
							break;
						}
						if ((parent.nodeType != 1) || (parent.tagName.toLowerCase() == 'body')) {
							break;
						}
						parent = parent.parentElement;
					}
					var selectedHTML = range.htmlText.replace(/^\s+|\s+$/, '');

					if (selectedHTML == range.text.replace(/^\s+|\s+$/, '')) {
						// we are within a single element
						range.text = 'MYOPENSPAN'+range.text+'MYCLOSESPAN';
					} else if ((parent.innerHTML == selectedHTML) || (parent.outerHTML.replace(/\s/g, '') == selectedHTML.replace(/\s/g, ''))) {
						// Apply class to parent element directly
						parent.className = value;
					} else {
						// Traverse children of parent
						for (var elt = parent.firstChild; elt != null; elt = elt.nextSibling) {
							var tempRange = document.body.createTextRange();
							tempRange.moveToElementText(elt);
							var overlaps =
								(range.compareEndPoints('StartToEnd', tempRange) <= 0)
								&& (range.compareEndPoints('EndToStart', tempRange) >= 0);
							if (overlaps) {
								var endsAfter = (range.compareEndPoints('EndToEnd', tempRange) == -1);
								var startsBefore = (range.compareEndPoints('StartToStart', tempRange) == 1);
								if (startsBefore) {
									var offset = 0;
									while (range.compareEndPoints('StartToStart', tempRange) == 1) {
										offset++;
										tempRange.moveStart('character', 1);
									}
									tempRange.moveToElementText(elt);
									var newText = tempRange.text.substring(0, offset)+"MYOPENSPAN"+tempRange.text.substring(offset, tempRange.text.length)+"MYCLOSESPAN";
									if (elt.nodeType == 3) {
										elt.data = newText;
									} else if ((elt.childNodes.length == 1) && (elt.firstChild.nodeType == 3)) {
										elt.innerHTML = newText;
									} else {
										tempRange.text = newText; // IE stuffs this up often so we avoid it
									}
								} else if (endsAfter) {
									// This element extends beyond the selection
									var offset = 0;
									while (range.compareEndPoints('EndToEnd', tempRange) == -1) {
										offset++;
										tempRange.moveEnd('character', -1);
									}
									tempRange.moveToElementText(elt);
									tempRange.text = "MYOPENSPAN"+tempRange.text.substring(0, tempRange.text.length-offset+1)+"MYCLOSESPAN"+tempRange.text.substring(tempRange.text.length-offset+1, tempRange.text.length);
								} else {
									// This element is fully within the selection
									// Apply class to it, or wrap a classed span around it
									if (elt.nodeType == 3) {
										// surround it with a span
										var newSpan = document.createElement('SPAN');
										newSpan.innerHTML = elt.data;
										newSpan.className = value;
										elt.parentNode.insertBefore(newSpan, elt);
										elt.parentNode.removeChild(elt);
									} else {
										elt.className = value;
									}
								}
							}
						}
					}
					parent.innerHTML = (parent.innerHTML.replace(new RegExp("MYOPENSPAN", "g"), '<span class="'+value+'">').replace(new RegExp("MYCLOSESPAN", "g"), '</span>'));
				}

			} else {

				///////// NON IE /////////
				if (range.collapsed) {
					break;
				}
				if ((range.startContainer.nodeType == 3) && (range.startContainer == range.endContainer)) {
					// Selection covers only a single text node
					if ((range.startOffset == 0) && (range.endOffset == range.endContainer.length)) {
						// all of one text node selected
						range.startContainer.parentNode.className = value;
					} else {
						// part of one text node selected
						this.surroundHTML('<span class="'+value+'">', '</span>');
					}
					break;

				}//end if inside a single text node

				var effectiveStartElt = range.startContainer;
				if ((effectiveStartElt.nodeType == 3)) {
					effectiveStartElt = effectiveStartElt.parentNode;
				}
				var effectiveEndElt = range.endContainer;
				if ((effectiveEndElt.nodeType == 3)) {
					effectiveEndElt = effectiveEndElt.parentNode;
				}

				if (range.startContainer == range.endContainer) {
					if ((range.startOffset != 0) || (range.endOffset != range.childNodes.length)) {
						effectiveStartElt = range.startContainer.childNodes.item(range.startOffset);
						effectiveEndElt = range.startContainer.childNodes.item(range.endOffset);
					}
				}

				var ancestor = range.commonAncestorContainer;
				if ((ancestor.tagName == 'UL') || (ancestor.tagName == 'OL') || (ancestor.tagName == 'TABLE') || (ancestor.tagName == 'DL') || (ancestor.tagName == 'TR')) {
					// Do some clearing up of empty text nodes to avoid confusion
					var elt = ancestor.firstChild;
					while (elt != null) {
						nextElt = elt.nextSibling;
						if ((elt.nodeType == 3) && (elt.data.replace(/^\s+|\s+$/, '') == '')) {
							elt.parentNode.removeChild(elt);
						}
						elt = nextElt;
					}
				}

				if ((effectiveStartElt == effectiveStartElt.parentNode.firstChild) && (effectiveEndElt == effectiveStartElt.parentNode.lastChild) && (range.startOffset == 0) && (range.endOffset == 0)) {
					// the selection equals all of the parent container, so just apply there
					effectiveStartElt.parentNode.className = value;
					break;

				}//end if all of parent container selected

				if (effectiveStartElt.parentNode == effectiveEndElt.parentNode) {
					// we can traverse

					// Do the start container
					if (range.startContainer.nodeType == 3) {
						if (range.startOffset == 0) {
							effectiveStartElt.className = value;
						} else {
							// split it
							var newSpan = document.createElement('SPAN');
							newSpan.innerHTML = range.startContainer.data.substring(range.startOffset);
							newSpan.className = value;
							if (range.startContainer.nextSibling == null) {
								range.startContainer.parentNode.appendChild(newSpan);
							} else {
								range.startContainer.parentNode.insertBefore(newSpan, range.startContainer.nextSibling);
							}
							range.startContainer.data = range.startContainer.data.substring(0, range.startOffset);
						}
					} else {
						effectiveStartElt.className = value;
					}

					// Do the in-between containers
					var tempElt = effectiveStartElt.nextSibling;
					while ((tempElt != null) && (tempElt != effectiveEndElt)) {
						if (tempElt.nodeType == 3) {
							// surround it with a span
							var newSpan = document.createElement('SPAN');
							newSpan.innerHTML = tempElt.data;
							newSpan.className = value;
							tempElt.parentNode.insertBefore(newSpan, tempElt);
							tempElt.parentNode.removeChild(tempElt);
						} else {
							tempElt.className = value;
						}
						tempElt = tempElt.nextSibling;
					}

					// Do the end container
					if (range.endContainer.nodeType == 3) {
						if (range.endOffset >= range.endContainer.data.length-1) {
							effectiveEndElt.className = value;
						} else {
							// split it
							var newSpan = document.createElement('SPAN');
							newSpan.className = value;
							newSpan.innerHTML = range.endContainer.data.substring(0, range.endOffset);
							range.endContainer.parentNode.insertBefore(newSpan, range.endContainer);
							range.endContainer.data = range.endContainer.data.substring(range.endOffset);
						}
					} else {
						effectiveEndElt.className == value;
					}
					break;

				}//end if common parent

				alert('Styles cannot be applied to the current selection.  Try applying styles to smaller subsections individually');
			}
		break;

		case "formatblock":
			if (HTMLArea.is_ie || HTMLArea.is_post_ie11) { // sad but true
				value = "<" + value + ">";
			}
			this._execCommand(txt, false, value);
		break;
		<?php

	}//end print_plugin_combo_selected()


	/**
	* Removes all applied styles that are blank
	*
	* @param string	&html	the HTML that was submitted by the editor
	*
	* @return string
	* @access public
	*/
	function process(&$html)
	{
		$html = preg_replace('|<span[^>]+class\="?no_style"?([^>]+)?>([^<]+)?</span>|i', '\\2', $html);		
		$html = preg_replace('|(<[^>]*?)class\="?no_style"?([^>]*?>)|i', '$1$2', $html);

	}//end process();
	
	
	/**
	* Gets current classes from applied design
	*
	* @return Array()
	* @access public
	*/
	function getDesignClasses()
	{
		$classes = Array();
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if (isset($GLOBALS['sq_preview_url']) && !empty($GLOBALS['sq_preview_url'])) {
			$url = $GLOBALS['sq_preview_url'];
			$url = preg_replace('|^http[s]?://|', '', $url);
			
			$designid = $am->getValueFromURL($url, 'design::%frontend%', TRUE);

			if ($designid) {
				$design = $am->getAsset($designid);
				$classes_list = $design->attr('wysiwyg_classes');
				if (!empty($classes_list)) {
					foreach ($classes_list as $key => $value) {
						$classes[$key] = $value;
					}
				}
			}
		}

		return $classes;

	}//end getDesignClasses();	


}//end class()

?>
