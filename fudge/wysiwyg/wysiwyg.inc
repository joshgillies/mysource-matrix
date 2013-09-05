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
* $Id: wysiwyg.inc,v 1.101 2013/09/05 06:48:54 akarelia Exp $
*
*/


/**
* WYSIWYG
*
* Purpose
*     A class used to create a cross browser WYSIWYG editor.
*     The WYSIWYG object requires you to add plugins to it to
*     provide the functionality. Use the add_plugin function
*     to do this.
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.101 $
* @package Fudge
* @subpackage wysiwyg
*/
class wysiwyg
{

	/**
	* A unique name to identify this WYSIWYG editor
	*
	* @var string
	* @see paint()
	*/
	var $name  = 'wysiwyg';

	/**
	* If the WYSIWYG should use an iframe or div to print/scale
	* A div will scale better than an iframe
	*
	* @var string
	* @see paint(), set_body_type()
	*/
	var $body_type  = 'iframe';

	/**
	* Should the WYSIWYG initilise itself onLoad for the window?
	* Set to 'no' if you want to init the WYSIWYG yourself
	*
	* @var string
	* @see paint(), set_init_onload()
	*/
	var $init_onload  = TRUE;


	/**
	* Should we show the status bar at the top of the editor?
	*
	* @var boolean
	* @see print_config(), set_show_status_bar()
	*/
	var $show_status_bar = TRUE;

	/**
	* The relative path for web accessable files
	*
	* @var string
	* @see paint()
	*/
	var $web_path  = '';

	/**
	* The stylesheet to use in the editor
	*
	* @var string
	* @see paint(), set_stylesheet()
	*/
	var $stylesheets  = Array();

	/**
	* The width of the WYSIWYG editor
	*
	* @var string
	* @see print_config(), set_width()
	*/
	var $width  = 'auto';

	/**
	* The height of the WYSIWYG editor
	*
	* @var string
	* @see print_config(), set_height();
	*/
	var $height  = 'auto';

	/**
	* Extra HTML code to add in the TEXTAREA tag
	*
	* @var string
	* @see print_config(), set_textarea_extras();
	*/
	var $textarea_extras = '';

	/**
	* The HTML to print in the editor
	*
	* @var string
	* @see paint(), set_contents();
	*/
	var $contents = '';

	/**
	* An array of plugins objects grouped for toolbar creation
	*
	* @var array
	* @see open_group(), close_group()
	*/
	var $_plugins = Array();

	/**
	* An array of plugins objects to provide functionality to the editor
	*
	* @var array
	* @see add_plugin()
	*/
	var $_loaded_plugins = Array();

	/**
	* Is a plugin group currently open
	*
	* @var boolean
	* @see open_group(), close_group(), add_plugin()
	*/
	var $_group_open = FALSE;

	/**
	* Has this WYSIWYG printed its header info yet?
	* We need this in case we have multiple WYSIWYG's on
	* the same page. We dont want to print the JS more than once
	*
	* @var boolean
	* @see paint()
	*/
	var $init = FALSE;

	/**
	* Dir attribute to be added while in wysiwyg mode
	*
	* @var string
	* @see paint()
	*/
	var $dir_attribute = '';


	/**
	* Constructor
	*
	* @param string $name A unique name for this WYSIWYG
	*
	* @access public
	*/
	function wysiwyg($name='wysiwyg', $web_path='')
	{
		$this->name = $name;
		$this->web_path = $web_path;
		global $ROOT_PATH;
		$ROOT_PATH = realpath(dirname(__FILE__)).'/';

	}//end constructor


	/**
	* Paints the JS needed for the WYSIWYG and the editable textarea
	*
	* @return void
	* @access public
	*/
	function paint()
	{

		global $WYSIWYG_INIT;
		if (!$WYSIWYG_INIT) {
			?>
			<script type="text/javascript">
				//
				// htmlArea v3.0 - Copyright (c) 2002 interactivetools.com, inc.
				// This copyright notice MUST stay intact for use (see license.txt).
				//
				// A free WYSIWYG editor replacement for <textarea> fields.
				// For full source code and docs, visit http://www.interactivetools.com/
				//
				// Version 3.0 developed by Mihai Bazon for InteractiveTools.
				//           http://students.infoiasi.ro/~mishoo
				//
				// Modifications for PHP Plugin Based System
				//           developed by Greg Sherwood for Squiz.Net.
				//           http://www.squiz.net/
				//           greg@squiz.net
				//

				// Creates a new HTMLArea object.  Tries to replace the textarea with the given
				// ID with it.
				function HTMLArea(textarea, config) {
					if (HTMLArea.checkSupportedBrowser()) {
						if (typeof config == "undefined") {
							eval("this.config = new HTMLArea.Config_" + textarea + "();");
						} else {
							this.config = config;
						}

						this._htmlArea = null;
						this._textArea = textarea;
						this._uniqueID = textarea;
						this._editMode = "wysiwyg";
						this._initialised = false;
						this._timerToolbar = null;
						this._toolbarObjects = Array();
						this._toolbarMenus = Array();
						this._tmp = Array();
<?php $this->print_plugin_vars() ?>
					}
				};
			</script>

			<script type="text/javascript">
				// Creates the toolbar and appends it to the _htmlarea
				HTMLArea.prototype._createToolbar = function () {
					var editor = this; // to access this in nested functions

					var toolbar = document.createElement("div");
					<?php
					if ($this->width != 'auto') {
						echo 'toolbar.style.width = "'.$this->width.'";';
					}
					?>
					var innerToolbar = document.createElement("div");

					var table = document.createElement("table");
					table.border = "0px";
					table.cellSpacing = "1px";
					table.cellPadding = "1px";

					toolbar.appendChild(table);
					// TBODY is required for IE, otherwise you don't see anything in the TABLE.
					var tb_body = document.createElement("tbody");
					table.appendChild(tb_body);
					tb_row = document.createElement("tr");
					tb_body.appendChild(tb_row);
					tb_cell = document.createElement("td");
					tb_row.appendChild(tb_cell);
					tb_cell.appendChild(innerToolbar);

					this._toolbar = toolbar;
					toolbar.className = "htmlarea-toolbar";
					toolbar.unselectable = "1";
					var tb_objects = new Object();
					this._toolbarObjects = tb_objects;

					// updates the state of a toolbar element
					function setButtonStatus(id, newval) {
						var oldval = this[id];
						var el = this.element;
						var img = el.style.backgroundImage;
						if (img != "" && oldval != newval) {
							switch (id) {
								case "enabled":
									if (newval) {
										HTMLArea._removeClass(el, "htmlarea-buttonDisabled");
										el.style.backgroundImage = "url(" + editor.imgURL("toolbar.png") + ")";
										el.disabled = false;
									} else {
										HTMLArea._addClass(el, "htmlarea-buttonDisabled");
										el.style.backgroundImage = "url(" + editor.imgURL("toolbar_disabled.png") + ")";
										el.disabled = true;
									}
								break;
								case "active":
									if (newval) {
										HTMLArea._addClass(el, "htmlarea-buttonPressed");
									} else {
										HTMLArea._removeClass(el, "htmlarea-buttonPressed");
									}
								break;
							}
							this[id] = newval;
						}
					};

					// this function will handle creation of combo boxes
					function createSelect(txt) {
						var options = null;
						var el = null;
						var sel = null;
						var cmd = null;
						switch (txt) {
<?php $this->print_plugin_create_select()?>
						}
						if (options) {
							sel = document.createElement("select");
							sel.className = "htmlarea-select";
							var obj = {
								name: txt,     // field name
								element: sel,  // the UI element (SELECT)
								enabled: true, // is it enabled?
								text: false,   // enabled in text mode?
								cmd: cmd,      // command ID
								state: setButtonStatus // for changing state
							};
							tb_objects[txt] = obj;
							for (var i in options) {
								var op = document.createElement("option");
								op.appendChild(document.createTextNode(i));
								op.value = options[i];
								sel.appendChild(op);
							}
							el = document.createElement("div");
							el.className = "htmlarea-select-container";
							el.appendChild(sel);

							HTMLArea._addEvent(sel, "change", function () {
								editor._comboSelected(sel, txt);
							});
						}
						return el;
					};

					// appends a new button to toolbar
					function createButton(container, txt) {
						// the element that will be created
						var el = null;
						var btn = null;
						switch (txt) {
							case "separator":
								el = document.createElement("div");
								el2 = document.createElement("div");
								el.appendChild(el2);
								el.className = "htmlarea-separator";
							break;
							case "space":
								el = document.createElement("div");
								el.className = "htmlarea-space";
							break;
							case "linebreak":
								el = document.createElement("div");
								el.className = "htmlarea-space";
							break;
<?php $this->print_plugin_button_type()?>
							default:
								btn = editor.config.btnList[txt];
							break;
						}
						if (!el && btn) {
							el = document.createElement("div");
							el.title = btn[1];
							el.className = "htmlarea-button";

							// let's just pretend we have a button object, and
							// assign all the needed information to it.
							var obj = {
								name    : txt,             // the button name (i.e. 'bold')
								element : el,              // the UI element (SPAN)
								enabled : true,            // is it enabled?
								active  : false,           // is it pressed?
								text    : btn[2],          // enabled in text mode?
								cmd     : btn[0],          // the command ID
								state   : setButtonStatus, // for changing state
								context : btn[4] || null   // will be disabled if outside this element
							};
							tb_objects[txt] = obj;

							// handlers to emulate nice flat toolbar buttons
							HTMLArea._addEvent(el, "mouseover", function () {
								if (obj.enabled) {
									HTMLArea._addClass(el, "htmlarea-buttonHover");
								}
							});
							HTMLArea._addEvent(el, "mouseout", function () {
								if (obj.enabled) with (HTMLArea) {
									_removeClass(el, "htmlarea-buttonHover");
									_removeClass(el, "htmlarea-buttonActive");
									(obj.active) && _addClass(el, "htmlarea-buttonPressed");
								}
							});
							HTMLArea._addEvent(el, "mousedown", function (ev) {
								if (obj.enabled) with (HTMLArea) {
									_addClass(el, "htmlarea-buttonActive");
									_removeClass(el, "htmlarea-buttonPressed");
									_stopEvent(is_ie ? window.event : ev);
								}
							});
							// when clicked, do the following:
							HTMLArea._addEvent(el, "click", function (ev) {
								if (obj.enabled) with (HTMLArea) {
									_removeClass(el, "htmlarea-buttonActive");
									_removeClass(el, "htmlarea-buttonHover");
									editor._buttonClicked(txt);
									_stopEvent(is_ie ? window.event : ev);
								}
							});

							var buttonID = editor._uniqueID + "_" + txt;
							buttonID = buttonID.toLowerCase();
							el.id = buttonID + "_span";
							var btnTop = Math.floor(btn[3] / 5);
							var btnLeft = ((btn[3] - (btnTop * 5)) * 18);
							el.style.background = "url(" + editor.imgURL("toolbar.png") + ") scroll -" + btnLeft + "px -" + (btnTop * 20) + "px";
							el.innerHTML = '<img src="' + editor.imgURL("blank.gif") + '" width="17" height="19" />';

						} else if (!el) {
							el = createSelect(txt);
						}
						if (el) {
							container.appendChild(el);
						} else if (txt != 'addkeyword') {
                         	// Do not throw error if the missing plugin is "add keyword" because
                            // its possible for plugin to get miss out from js code if a div content having keywords
                            // is linked (new link) elsewhere next to the div content without any keywords
							// See bug #4475
							alert(js_translate('unknown_toolbar_item', txt));
						}
						return el;
					};

					for (var i in this.config.toolbar) {
						var group = this.config.toolbar[i];
						for (var j in group) {
							var menu = group[j];
							if (menu.length < 1) {
								continue;
							} else if (menu.length == 1) {
								createButton(innerToolbar, menu[0]);
							} else {
								var div = document.createElement("div");
								editor._toolbar.appendChild(div);
								div.style.display = "none";
								div.style.position = "absolute";
								div.className = "htmlarea-menu";
								div.id = menu[0] + "_menu";

								createButton(innerToolbar, menu[0]);
								this.config.btnList[menu[0]][0] = "htmlarea-showmenu-" + menu[0];

								for (var x in menu) {
									if (x == 0) { continue; }

									var table = document.createElement("table");
									var tb_body = document.createElement("tbody");
									var tb_row = document.createElement("tr");
									var tb_cell = document.createElement("td");
									table.appendChild(tb_body);
									tb_body.appendChild(tb_row);
									tb_row.appendChild(tb_cell);

									table.className = "htmlarea-menuRow";
									createButton(tb_cell, menu[x])

									tb_cell = document.createElement("td");
									tb_cell.className = "htmlarea-menuText";
									tb_row.appendChild(tb_cell);

									var title = document.createTextNode(this.config.btnList[menu[x]][1]);
									tb_cell.appendChild(title);

									div.appendChild(table);
								}
								this._toolbarMenus[div.id] = div;
							}
						}
					}
				};

				// updates enabled/disable/active state of the toolbar elements
				HTMLArea.prototype.updateToolbar = function(allEnabled, noStatus) {
					var doc = this._doc;
					var text = (this._editMode == "textmode");
					var ancestors = null;
					var htmlareaFound = false;

					if (!text) {
						ancestors = this.getAllAncestors();
						if (this.config.statusBar && !noStatus) {
							this._statusBarTree.innerHTML = ''; // clear
							for (var i = ancestors.length; --i >= 0;) {
								var el = ancestors[i];
								if (!el) {
									// hell knows why we get here; this
									// could be a classic example of why
									// it's good to check for conditions
									// that are impossible to happen ;-)
									continue;
								}

								// work out the display name for this tag
								var txt = el.tagName.toLowerCase();
								if (el.id) {
									txt += "#" + el.id;
								}
								if (el.className) {
									txt += "." + el.className;
								}

								// if we are playing with an editable DIV, we need to tread carefully
								// because there is no body tag expect the one for the whole page
								// so we fudge the results a little to skip tags outside of the editor
								if (HTMLArea.is_ie && this.config.bodyType.toLowerCase() != 'iframe') {
									if (el.id == "htmlarea") {
										htmlareaFound = true;
										i--;
										var el = ancestors[i];
										txt = 'body';
									}
									if (!htmlareaFound) { continue; }
								}

								var a = document.createElement("a");
								a.href = "#";
								a.el = el;
								a.editor = this;
								a.onclick = function() {
									this.blur();
									this.editor.selectNodeContents(this.el);
									this.editor.updateToolbar(true);
									return false;
								};
								a.oncontextmenu = function() {
									// TODO: add context menu here
									this.blur();
									var info = "Inline style:\n\n";
									info += this.el.style.cssText.split(/;\s*/).join(";\n");
									alert(info);
									return false;
								};
								a.title = el.style.cssText;
								// NOTE: Using appendChild here causes IE to clear its undo/redo queue
								a.appendChild(document.createTextNode(txt));
								this._statusBarTree.appendChild(a);
								if (i != 0) {
									this._statusBarTree.appendChild(document.createTextNode(String.fromCharCode(0xbb)));
								}
							}
						}
					}

					for (var i in this._toolbarObjects) {
						var btn = this._toolbarObjects[i];
						var cmd = btn.cmd;

						var inContext = true;
						if (btn.context && !text) {
							inContext = false;
							var context = btn.context;
							var attrs = [];
							if (/(.*)\[(.*?)\]/.test(context)) {
								context = RegExp.$1;
								attrs = RegExp.$2.split(",");
							}
							context = context.toLowerCase();
							var match = (context == "*");
							htmlareaFound = false;
							for (var k = ancestors.length; --k >= 0;) {
								var el = ancestors[k];
								if (!el) {
									// the impossible really happens.
									continue;
								}

								// if we are playing with an editable DIV, we need to tread carefully
								// so we can skip tags outside of the editor

								if (HTMLArea.is_ie && this.config.bodyType.toLowerCase() != 'iframe') {
									if (el.id == "htmlarea") {
										htmlareaFound = true;
										k--;
										el = ancestors[k];
									}
									if (!htmlareaFound) { continue; }
								}

								if (match || (el.tagName.toLowerCase() == context)) {
									inContext = true;
									for (var ka in attrs) {
										if (!eval("ancestors[k]." + attrs[ka])) {
											inContext = false;
											break;
										}
									}
									if (inContext) { break; }
								}
							}
						}

						if (typeof cmd == "function") { continue; }

						if (allEnabled == null) {
							var btnEnabled = (!text || btn.text) && inContext;
						} else {
							var btnEnabled = allEnabled;
						}
						btn.state("enabled", btnEnabled);

						cmd = cmd.toLowerCase();
						switch (cmd) {
<?php $this->print_plugin_update_toolbar()?>
							default:
								try {
									btn.state("active", (!text && doc.queryCommandState(cmd)));
								} catch (e) {}
							break;
						}
					}
				};

				// gets called before the form is submitted
				HTMLArea.prototype._formSubmit = function() {
					var editor = this;           // needed in nested functions
					var html   = this.getHTML(); // HTML that will be set
												 // can be modified by plugins

<?php $this->print_plugin_form_submit()?>

					// retrieve the HTML
					html = this.make_relative_hrefs(html);
					this._textArea.value = html;
				};
			</script>

<?php $this->print_plugin_generic_functions()?>

			<script type="text/javascript">
				// txt is the name of the button, as in config.toolbar
				HTMLArea.prototype._buttonClicked = function(txt) {
					//stores original scrollbar position to circumvent
					//some IE craziness.
					var original_scroll = document.body.scrollTop;
					var editor = this; // needed in nested functions

					this.focusEditor();
					//set scrollbar back
					document.body.scrollTop = original_scroll;
					var btn = this.config.btnList[txt];
					if (!btn) {
						alert("FIXME: Unconfigured button!");
						return false;
					}
					var cmd = btn[0];
					if (typeof cmd == "function") {
						return cmd(this, txt);
					}

					if (/htmlarea-showmenu-/.test(cmd.toLowerCase())) {
						re = new RegExp("htmlarea-showmenu-");
						var menuID = cmd.replace(re, "");
						var menu = document.getElementById(menuID + "_menu");

						if (menu.style.display == "none") {
							// hide all menus that are showing
							for (var x in this._toolbarMenus) {
								this._toolbarMenus[x].style.display = "none";
							}

							menu.style.display = "block";

							var buttonID = editor._uniqueID + "_" + menuID + "_span";
							buttonID = buttonID.toLowerCase();
							var button = document.getElementById(buttonID);

							if (HTMLArea.is_ie) {
								menu.style.pixelTop = button.offsetTop + 23;
								menu.style.pixelLeft = button.offsetLeft;
							} else {
								menu.style.top = button.offsetTop + 23;
								menu.style.left = button.offsetLeft;
							}
						} else {
							menu.style.display = "none";
						}

					} else {
						// hide all menus that are showing
						for (var x in this._toolbarMenus) {
							this._toolbarMenus[x].style.display = "none";
						}

						switch (cmd.toLowerCase()) {
	<?php $this->print_plugin_button_clicks()?>
							default:
								this._execCommand(btn[0], false, "");
							break;
						}
					}
					this.updateToolbar();
					//set scrollbar back
					document.body.scrollTop = original_scroll;
					return false;
				};

				// el is reference to the SELECT object
				// txt is the name of the select field, as in config.toolbar
				HTMLArea.prototype._comboSelected = function(el, txt) {
					this.focusEditor();
					var value = el.options[el.selectedIndex].value;
					switch (txt) {
<?php $this->print_plugin_combo_selected()?>
						default:
							alert(js_translate('combo_box_not_implemented', txt));
						break;
					}
				};

				// the execCommand function (intercepts some commands and replaces them with
				// our own implementation)
				HTMLArea.prototype._execCommand = function(cmdID, UI, param) {
					switch (cmdID.toLowerCase()) {
<?php $this->print_plugin_exec_commands()?>
						default:
							this._doc.execCommand(cmdID, UI, param);
						break;
					}
					this.focusEditor();
				};

//bunch of tags to compare when examining nodes
HTMLArea.prototype.inline_tags = ['a', 'abbr', 'acronym', 'b', 'bdo', 'big', 'cite', 'code', 'dfn', 'em', 'font', 'i', 'kbd', 'label', 'q', 's', 'samp', 'select', 'small', 'span', 'strike', 'strong', 'sub', 'sup', 'textarea', 'tt', 'u', 'var'];

// Finds the first parent element of a given node whose display is probably not inline
HTMLArea.prototype.parentBlock = function(node) {

	while ( node.parentNode && ( node.nodeType != 1 || this.inArray( node.tagName.toLowerCase(), this.inline_tags ) ) ) node = node.parentNode;
	return node;
};

// Returns true if a given value is in an array, false otherwise
HTMLArea.prototype.inArray = function(val, arr) {

	for (var i=0; i < arr.length; i++)
		if (val == arr[i]) return true;
	return false;
};

// Sets dir attribute value
HTMLArea.prototype.set_dir_attr = function(val) {
	this._dir_attr = val;
};

// Internal function for recursively itterating over a all nodes in a fragment
// If a callback function returns a non-null value, that is returned and the crawl is therefore broken
HTMLArea.prototype.traverseChildrenNodes = function(me, callback) {

	if ( me.firstChild ) {
		var myChild = me.firstChild;
		var retVal;
		while (myChild) {
			if ( (retVal=callback(myChild)) != null ) return retVal;
			if ( (retVal=this.traverseChildrenNodes(myChild, callback)) != null ) return retVal;
			myChild = myChild.nextSibling;
		}
	}
};

// Callback function to be performed on each node in the hierarchy
// Sets flag to true if we find actual text or an element that's not usually displayed inline
HTMLArea.prototype.containsText = function(node) {

	if (node.nodeType == 1 && HTMLArea.prototype.inArray(node.nodeName.toLowerCase(), HTMLArea.prototype.inline_tags) && node.hasChildNodes() && node.firstChild.nodeValue) return true;
	else if (node.nodeType == 3 && node.nodeValue != '') return true;
	else return null;

};

// Inserts a node down into a node, on the left side of the tree under target
HTMLArea.prototype.insertDeepLeft = function(target, toInsert) {

	var digger = target;
	while ( digger.firstChild && digger.firstChild.nodeType == 1 ) digger = digger.firstChild;
	var refNode = digger.firstChild ? digger.firstChild : null;
	//falling.insertBefore(toInsert, refNode);
	digger.innerHTML = toInsert;
};

				HTMLArea.prototype.isElem = function(node,type) {
					return node.nodeName.toLowerCase() == type.toLowerCase();
				};


				// A generic event handler for things that happen in the IFRAME's document.
				// This function also handles key bindings.

				HTMLArea.prototype._editorEvent = function(ev) {
					var editor = this;
					// Make enter without shift add <p> tags in Moz as well as IE
					if (ev.type == "keypress" && !HTMLArea.is_ie) {
						var keyCode = ev.keyCode;
						if (keyCode == 13 && !ev.shiftKey && this._iframe.contentWindow.getSelection) {

							// Get the selection and solid references to what we're dealing with chopping
							var sel = this._iframe.contentWindow.getSelection();

							// Set the start and end points such that they're going /forward/ through the document
							var leftRange = this._doc.createRange();
							var rightRange = this._doc.createRange();

							// While getting anchorNode and focusNode, for some reason Firefox is getting parent node of the
							// selected node instead of selected node itself when non-text node is selected
							// See bug #3826
							var sAnchorNode = sel.anchorNode;
							var sFocusNode = sel.focusNode;
							var sAnchorOffset = sel.anchorOffset;
							var sFocusOffset = sel.focusOffset;

							if (sAnchorNode.nodeName == 'BODY' && sAnchorNode.childNodes[sAnchorOffset]) {
								sAnchorNode = sAnchorNode.childNodes[sAnchorOffset];
								sAnchorOffset = 0;

								while(sAnchorNode.childNodes[0] && sAnchorNode.nodeName != '#text')
									sAnchorNode = sAnchorNode.childNodes[0];
							}

							if (sFocusNode.nodeName == 'BODY' && sFocusNode.childNodes[sFocusOffset]) {
								sFocusNode = sFocusNode.childNodes[sFocusOffset];
								sFocusOffset = 0;

								while(sFocusNode.childNodes[0] && sFocusNode.nodeName != '#text')
									sFocusNode = sFocusNode.childNodes[0];
							}

							if (sAnchorNode.nodeName != '#text') {
								empty_node = document.createTextNode('');
								if (sAnchorNode.nodeName != 'BR') {
									sAnchorNode.appendChild(empty_node);
								} else {
									sAnchorNode.parentNode.insertBefore(empty_node, sAnchorNode.nextSibling);
								}
								sAnchorNode = empty_node;
								sAnchorOffset = 0;
							}

							if (sFocusNode.nodeName != '#text') {
								empty_node = document.createTextNode('');
								if (sFocusNode.nodeName != 'BR') {
									sFocusNode.appendChild(empty_node);
								} else {
									sFocusNode.parentNode.insertBefore(empty_node, sFocusNode.nextSibling);
								}
								sFocusNode = empty_node;
								sFocusOffset = 0;
							}

							leftRange.setStart(sAnchorNode,sAnchorOffset);
							rightRange.setStart(sFocusNode,sFocusOffset);
							leftRange.collapse(true);
							rightRange.collapse(true);

							var direct = leftRange.compareBoundaryPoints(leftRange.START_TO_END,rightRange);

							var startNode = (direct<0) ? sAnchorNode : sFocusNode;
							var startOffset = (direct<0) ? sAnchorOffset : sFocusOffset;
							var endNode = (direct<0) ? sFocusNode : sAnchorNode;
							var endOffset = (direct<0) ? sFocusOffset : sAnchorOffset;

							// When Focus node and Anchor node are same point and foucs node is at end of an anchor node, add an extra blank node
							// This is to make sure that paragraphs are created outside the node. See bug #3978
							if (startOffset == endOffset && startNode == endNode && endNode.nodeValue && endNode.nodeValue.length == endOffset && endNode.parentNode && this.isElem(endNode.parentNode,'a') && endNode.parentNode.parentNode) {

								blank_node = document.createTextNode('');
								endNode.parentNode.parentNode.appendChild(blank_node);

								startNode = blank_node;
								startOffset = 0;
								endNode = blank_node;
								endOffset = 0;
							}

							// Find the parent blocks of nodes at either end, and get paragraph attributes
							var startBlock = this.parentBlock(startNode);
							var endBlock = this.parentBlock(endNode);
							var leftParagraphAttributes = new Array();
							var rightParagraphAttributes = new Array();

							// Inside a list Item, let the browser handle it.
							if ( this.isElem(startBlock,'li') || this.isElem(endBlock,'li') ) return;

							// if line starts with a header
							var isInsideHeader = this.isElem(startBlock, 'h1') || this.isElem(startBlock, 'h2') || this.isElem(startBlock, 'h3') || this.isElem(startBlock, 'h4') || this.isElem(startBlock, 'h5') || this.isElem(startBlock, 'h6');
							var headerType = null;
							if (isInsideHeader) {
								if (this.isElem(startBlock, 'h1')) headerType = 'h1';
								else if (this.isElem(startBlock, 'h2')) headerType = 'h2';
								else if (this.isElem(startBlock, 'h3')) headerType = 'h3';
								else if (this.isElem(startBlock, 'h4')) headerType = 'h4';
								else if (this.isElem(startBlock, 'h5')) headerType = 'h5';
								else if (this.isElem(startBlock, 'h6')) headerType = 'h6';
							}

							if ( this.isElem(startBlock,'p') ) {
								for ( var i=0; i < startBlock.attributes.length; i++ ) {
									leftParagraphAttributes[ startBlock.attributes[i].nodeName ] = startBlock.attributes[i].nodeValue;
								}
							}
							if ( this.isElem(endBlock,'p') ) {
								for ( var i=0; i < endBlock.attributes.length; i++ ) {
									rightParagraphAttributes[ endBlock.attributes[i].nodeName ] = endBlock.attributes[i].nodeValue;
								}
							}
							var choppingStart = startNode;
							var choppingEnd = endNode;

							while ( ( choppingStart.previousSibling && !this.isElem(choppingStart.previousSibling,'p') ) || ( choppingStart.parentNode && choppingStart.parentNode != startBlock && choppingStart.parentNode.nodeType != 9 ) )
								choppingStart = choppingStart.previousSibling ? choppingStart.previousSibling : choppingStart.parentNode;

							while ( ( choppingEnd.nextSibling && !this.isElem(choppingEnd.nextSibling,'p') ) || ( choppingEnd.parentNode && choppingEnd.parentNode != endBlock && choppingEnd.parentNode.nodeType != 9 ) )
								choppingEnd = choppingEnd.nextSibling ? choppingEnd.nextSibling : choppingEnd.parentNode;

							// Set up new paragraphs
							var leftParagraph = null;
							var rightParagraph = null;

							// this element will be used if u r in a header
							var newParagraph = null;

							if (!isInsideHeader) {
								// if we are not inside a heading keep the original behaviour
								leftParagraph = this._doc.createElement('p');
								rightParagraph = this._doc.createElement('p');
							} else {
								if (headerType != null) {
									// create two paragraph with the header
									leftParagraph = this._doc.createElement(headerType);
									rightParagraph = this._doc.createElement(headerType);
								} else {
									// error headerType should not be null!
								}

								newParagraph = this._doc.createElement('p')
							}

							for ( var attrName in leftParagraphAttributes ) {
								var thisAttr = this._doc.createAttribute(attrName);
								thisAttr.value = leftParagraphAttributes[attrName];
								leftParagraph.setAttributeNode(thisAttr);
							}
							for ( var attrName in rightParagraphAttributes ) {
								var thisAttr = this._doc.createAttribute(attrName);
								thisAttr.value = rightParagraphAttributes[attrName];
								rightParagraph.setAttributeNode(thisAttr);
							}

							// Split the data into left and right paragraphs

							//For Left paragraph
							leftRange.setStartBefore(choppingStart);
							leftRange.setEnd(startNode,startOffset);
							leftParagraph.appendChild(leftRange.cloneContents());

							//For Right Paragraph
							rightRange.setEndAfter(choppingEnd);
							rightRange.setStart(endNode,endOffset);
							rightParagraph.appendChild(rightRange.cloneContents());

							// containsData is true if there is data in the paragraph,
							//if false, insert a non-breaking space
							var isLeftParagraphEmpty = false;
							var containsData = false;
							containsData = this.traverseChildrenNodes(leftParagraph, this.containsText);

							if ( containsData != true ) {
								this.insertDeepLeft(leftParagraph, '&nbsp;');
								isLeftParagraphEmpty = true;
							}

							// containsData is true if there is data in the paragraph,
							//if false, insert a non-breaking space
							var isRightParagraphEmpty = false;
							containsData = false;
							containsData = this.traverseChildrenNodes(rightParagraph, this.containsText);
							if ( containsData != true ) {
								this.insertDeepLeft(rightParagraph, '&nbsp;');
								isRightParagraphEmpty = true;
							}

							// Get the range of everything to be replaced
							var fullRange = this._doc.createRange();

							if (!isInsideHeader) {
								//if no previous sibling, and parent is a paragraph, set start of range before parent, so it will be removed.
								//otherwise set to original start range
								if ( !choppingStart.previousSibling && this.isElem(choppingStart.parentNode,'p') ) {
									fullRange.setStartBefore(choppingStart.parentNode);
								} else {
									fullRange.setStart(leftRange.startContainer, leftRange.startOffset);
								}

								if ( !choppingEnd.nextSibling && this.isElem(choppingEnd.parentNode,'p') ) {
									fullRange.setEndAfter(choppingEnd.parentNode);
								} else {
									fullRange.setEnd(rightRange.endContainer, rightRange.endOffset);
								}
							} else {

								// if we are inside a header we will chop the whole line
								// and replace it with 3 new lines.
								// - first line will from starting to the cursor position of the old line
								// - second line will an empty p tag
								// - third line will be from the cursor position to the ending of the old line
								fullRange.setStartBefore(choppingStart.parentNode);
								fullRange.setEndAfter(choppingEnd.parentNode);
							}

							//remove data from range and insert new paragraphs
							fullRange.deleteContents();

							if (isInsideHeader) {
								// add the rightParagraph
								if (!isRightParagraphEmpty) fullRange.insertNode(rightParagraph);
								// add a space in the new paragraph
								newParagraph.appendChild(document.createTextNode(""+'\u00A0'));
								fullRange.insertNode(newParagraph);
								// add the rightParagraph
								if (!isLeftParagraphEmpty) fullRange.insertNode(leftParagraph);

								// set the selection to the start of the new paragraph (new)
								var cursorRange = this._doc.createRange();
								cursorRange.setStart(newParagraph,0);
								cursorRange.collapse(true);
								sel = this._iframe.contentWindow.getSelection();
								sel.removeAllRanges();
								sel.addRange(cursorRange);
							} else {
								// we keep the old behaviour
								fullRange.insertNode(rightParagraph);
								fullRange.insertNode(leftParagraph);
								if ( rightParagraph.firstChild ) {
									while ( this.inArray(rightParagraph.firstChild && rightParagraph.firstChild.nodeName.toLowerCase(), this.inline_tags) ) rightParagraph = rightParagraph.firstChild;
										if ( rightParagraph.firstChild && rightParagraph.firstChild.nodeType == 3 ) rightParagraph = rightParagraph.firstChild;	// and text, if they've got it

								// Set the selection to the start of the new paragraph (right)
									var cursorRange = this._doc.createRange();
									cursorRange.setStart(rightParagraph,0);
									cursorRange.collapse(true);
									sel = this._iframe.contentWindow.getSelection();
									sel.removeAllRanges();
									sel.addRange(cursorRange);
								}
							}


				// Stop event to prevent no IE browsers adding the <br>
					HTMLArea._stopEvent(ev);
					}
				}

<?php $this->print_plugin_event_handlers()?>

					var keyEvent = (HTMLArea.is_ie && ev.type == "keydown") || (ev.type == "keypress");
					if (keyEvent && ev.ctrlKey) {
						var key = String.fromCharCode(HTMLArea.is_ie ? ev.keyCode : ev.charCode).toLowerCase();
						var sel = null;
						var range = null;
						var cmd = null;
						var value = null;
						switch (key) {
<?php $this->print_plugin_shortcuts()?>
						}
						if (cmd) {
							// execute simple command
							this._execCommand(cmd, false, value);
							HTMLArea._stopEvent(ev);
						}
					}

					// update the toolbar state after some time
					// if we are not just generally typing
					if (!keyEvent || (keyEvent && ev.ctrlKey)) {
						// hide all menus that are showing
						for (var x in this._toolbarMenus) {
							this._toolbarMenus[x].style.display = "none";
						}

						setTimeout(function() {
							editor.updateToolbar();
						}, 50);
					}
				};

				// retrieve the HTML
				HTMLArea.prototype.getHTML = function() {
					var retVal = '';
<?php $this->print_plugin_pre_get_html()?>
					switch (this._editMode) {
						case "textmode":
							if (HTMLArea.is_gecko) {
								var html = this._iframe.contentWindow.document.body.ownerDocument.createRange();
								html.selectNodeContents(this._iframe.contentWindow.document.body);
								retVal = html.toString();
							} else if (HTMLArea.is_ie) {
								retVal = this._docContent.innerText;
							}
						break;
						case "wysiwyg":
							if (HTMLArea.is_gecko) {
								retVal = this._iframe.contentWindow.document.body.innerHTML;
								// if we are using firefox and if it is the initial container
								// we need to remove any new line character because
								// firefox will keep on adding <br> and additional \n(\n)
								// do make sure our body (div) is visible
								// see bug #5148 br tag created in the WYSIWYG as soon as it's created
								if (HTMLArea.is_ff && retVal.match(/^<br>(\n)*$/) != null) {
									retVal = retVal.replace(/^<br>(\n)*$/, '');
								}
							} else if (HTMLArea.is_ie) {
								retVal = this._docContent.innerHTML;
							}

<?php
// Don't replace entities on UTF8 systems
if (SQ_CONF_DEFAULT_CHARACTER_SET != 'utf-8') {
	echo 'retVal = this.replace_entities(retVal);';
} else {
// Exceptions. See #5051
?>
	var entities_array = {
		173 : '&shy;' //soft hyphen
	};

	retVal = this.replace_entities(retVal, entities_array);
	
<?php	
}
?>

						break;
						default:
							alert(js_translate('undefined_mode', '<' + this._editMode + '>'));
							return false;
					}
					// let any plugins modify the returned HTML if they want to
<?php $this->print_plugin_get_html()?>

					if (this._editMode == "textmode") {
//						retVal = this.make_absolute_urls(retVal);
					} else {
						retVal = this.make_relative_hrefs(retVal);
					}
					return retVal;
				};

				 ////////////////////////////////////////////////////////////////////////
				// because the editor automatically prepend the http:// business to
				// all relative urls, remove them from the html
				HTMLArea.prototype.make_relative_hrefs = function(html_code) {

					var html = new String(html_code);
					var e  = '';
					var re = null;

					// remove any references to this pop-up's url
					var current_location = document.location.toString();

					// escape any special reg exp chars in the location
					current_location = current_location.replace(/([.?+*^$\\\[\]\(\)\{\}|])/gi, '\\$1');
					re = new RegExp(current_location + '[/]?', "gi");
					html = html.replace(re, "");

					// now replace any ampersands with '&amp;' as IE seems to anyway sometimes (even in href's)
					current_location = current_location.replace(/\&/gi, "&amp;");
					re = new RegExp(current_location, "gi");
					html = html.replace(re, "");

					// any other links	 
					e = 'http[s]?://<?php echo $_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF']?>';	 
					re = new RegExp(e, "gi");	 
					html = html.replace(re, "./");

					return html;
				};

				// We want to replace actual entities in the html(like �) with
				// Their correct &bla; representations
				HTMLArea.prototype.replace_entities = function(html_code, entities_array) {

				if (entities_array == undefined){
					var entities_array = {
						160 : '&nbsp;',		// non-breaking space
						161 : '&iexcl;',	// inverted exclamation mark
						162 : '&cent;',		// cent sign
						163 : '&pound;',	// pound sterling sign
						164 : '&curren;',	// general currency sign
						165 : '&yen;',		// yen sign
						166 : '&brvbar;',	// broken vertical bar
						167 : '&sect;',		// section sign
						168 : '&uml;',		// spacing dieresis or umlaut
						169 : '&copy;',		// copyright sign
						170 : '&ordf;',		// feminine ordinal indicator
						171 : '&laquo;',	// left (double) angle quote (guillemet)
						172 : '&not;',		// logical not sign
						173 : '&shy;',		// soft hyphen
						174 : '&reg;',		// registered trademark sign
						175 : '&macr;',		// spacing macron (long) accent
						176 : '&deg;',		// degree sign
						177 : '&plusmn;',	// plus-or-minus sign
						178 : '&sup2;',		// superscript 2
						179 : '&sup3;',		// superscript 3
						180 : '&acute;',	// spacing acute accent
						181 : '&micro;',	// micro sign
						182 : '&para;',		// paragraph sigh, pilcrown sign
						183 : '&middot;',	// middle dot, centred dot
						184 : '&cedil;',	// spacing cedilla
						185 : '&sup1;',		// superscript 1
						186 : '&ordm;',		// masculine ordinal indicator
						187 : '&raquo;',	// right (double) angle quote (guillemet)
						188 : '&frac14;',	// fraction 1/4
						189 : '&frac12;',	// fraction 1/2
						190 : '&frac34;',	// fraction 3/4
						191 : '&iquest;',	// inverted question mark
						192 : '&Agrave;',	// capital A grave
						193 : '&Aacute;',	// capital A acute
						194 : '&Acirc;',	// capital A circumflex
						195 : '&Atilde;',	// capital A tilde
						196 : '&Auml;',		// capital A dieresis or umlaut
						197 : '&Aring;',	// capital A ring
						198 : '&AElig;',	// capital AE ligature
						199 : '&Ccedil;',	// capital C cedilla
						200 : '&Egrave;',	// capital E grave
						201 : '&Eacute;',	// capital E acute
						202 : '&Ecirc;',	// capital E circumflex
						203 : '&Euml;',		// capital E dieresis or umlaut
						204 : '&Igrave;',	// capital I grave
						205 : '&Iacute;',	// capital I acute
						206 : '&Icirc;',	// capital I circumflex
						207 : '&Iuml;',		// capital I dieresis or umlaut
						208 : '&ETH;',		// capital ETH
						209 : '&Ntilde;',	// capital N tilde
						210 : '&Ograve;',	// capital O grave
						211 : '&Oacute;',	// capital O acute
						212 : '&Ocirc;',	// capital O circumflex
						213 : '&Otilde;',	// capital O tilde
						214 : '&Ouml;',		// capital O dieresis or umlaut
						215 : '&times;',	// multiplication sign
						216 : '&Oslash;',	// capital O slash
						217 : '&Ugrave;',	// capital U grave
						218 : '&Uacute;',	// capital U acute
						219 : '&Ucirc;',	// capital U circumflex
						220 : '&Uuml;',		// capital U dieresis or umlaut
						221 : '&Yacute;',	// capital Y acute
						222 : '&THORN;',	// capital THORN
						223 : '&szlig;',	// small sharp s, sz ligature
						224 : '&agrave;',	// small a grave
						225 : '&aacute;',	// small a acute
						226 : '&acirc;',	// small a circumflex
						227 : '&atilde;',	// small a tilde
						228 : '&auml;',		// small a dieresis or umlaut
						229 : '&aring;',	// small a ring
						230 : '&aelig;',	// small ae ligature
						231 : '&ccedil;',	// small c cedilla
						232 : '&egrave;',	// small e grave
						233 : '&eacute;',	// small e acute
						234 : '&ecirc;',	// small e circumflex
						235 : '&euml;',		// small e dieresis or umlaut
						236 : '&igrave;',	// small i grave
						237 : '&iacute;',	// small i acute
						238 : '&icirc;',	// small i circumflex
						239 : '&iuml;',		// small i dieresis or umlaut
						240 : '&eth;',		// small eth
						241 : '&ntilde;',	// small n tilde
						242 : '&ograve;',	// small o grave
						243 : '&oacute;',	// small o acute
						244 : '&ocirc;',	// small o circumflex
						245 : '&otilde;',	// small o tilde
						246 : '&ouml;',		// small o dieresis or umlaut
						247 : '&divide;',	// division sign
						248 : '&oslash;',	// small o slash
						249 : '&ugrave;',	// small u grave
						250 : '&uacute;',	// small u acute
						251 : '&ucirc;',	// small u circumflex
						252 : '&uuml;',		// small u dieresis or umlaut
						253 : '&yacute;',	// small y acute
						254 : '&thorn;',	// small thorn
						255 : '&yuml;',		// small y dieresis or umlaut
						256 : '&#256;',     // uppercase roman A with macron
						257 : '&#257;',		// lowercase roman a with macron
						274 : '&#274;',		// uppercase roman E with macron
						275 : '&#275;',		// lowercase roman e with macron
						298 : '&#298;',		// uppercase roman I with macron
						299 : '&#299;',		// lowercase roman i with macron
						332 : '&#332;',		// uppercase roman O with macron
						333 : '&#333;',		// lowercase roman o with macron
						362 : '&#362;',		// uppercase roman U with macron
						363 : '&#363;',		// lowercase roman u with macron
						338 : '&OElig;',	// capital ligature OE
						339 : '&oelig;',	// small ligature oe
						352 : '&Scaron;',	// capital S with caron
						353 : '&scaron;',	// small s with caron
						376 : '&Yuml;',		// capital Y with diaeres
						402 : '&fnof;',		// small italic f, function of, florin
						710 : '&circ;',		// modifier letter circumflex accent
						732 : '&tilde;',	// small tilde
						913 : '&Alpha;',	// capital Alhpa
						914 : '&Beta;',		// capital Beta
						915 : '&Gamma;',	// capital Gamma
						916 : '&Delta;',	// capital Delta
						917 : '&Epsilon;',	// capital Epsilon
						918 : '&Zeta;',		// capital Zeta
						919 : '&Eta;',		// capital Eta
						920 : '&Theta;',	// capital Theta
						921 : '&Iota;',		// capital Iota
						922 : '&Kappa;',	// capital Kappa
						923 : '&Lambda;',	// capital Lambda
						924 : '&Mu;',		// capital Mu
						925 : '&Nu;',		// capital Nu
						926 : '&Xi;',		// capital Xi
						927 : '&Omicron;',	// capital Omicron
						928 : '&Pi;',		// capital Pi
						929 : '&Rho;',		// capital Rho
						931 : '&Sigma;',	// capital Sigma
						932 : '&Tau;',		// capital Tau
						933 : '&Upsilon;',	// capital Upsilon
						934 : '&Phi;',		// capital Phi
						935 : '&Chi;',		// capital Chi
						936 : '&Psi;',		// capital Psi
						937 : '&Omega;',	// capital Omega
						945 : '&alpha;',	// small alpha
						946 : '&beta;',		// small beta
						947 : '&gamma;',	// small gamma
						948 : '&delta;',	// small delta
						949 : '&epsilon;',	// small epsilon
						950 : '&zeta;',		// small zeta
						951 : '&eta;',		// small eta
						952 : '&theta;',	// small theta
						953 : '&iota;',		// small iota
						954 : '&kappa;',	// small kappa
						955 : '&lambda;',	// small lambda
						956 : '&mu;',		// small mu
						957 : '&nu;',		// small nu
						958 : '&xi;',		// small xi
						959 : '&omicron;',	// small omicron
						960 : '&pi;',		// small pi
						961 : '&rho;',		// small rho
						962 : '&sigmaf;',	// small sigma final
						963 : '&sigma;',	// small sigma
						964 : '&tau;',		// small tau
						965 : '&upsilon;',	// small upsilon
						966 : '&phi;',		// small phi
						967 : '&chi;',		// small chi
						968 : '&psi;',		// small psi
						969 : '&omega;',	// small omega
						977 : '&thetasym;',	// small theta symbol
						978 : '&upsih;',	// small upsilon symbol
						982 : '&piv;',		// small pi symbol
						8224 : '&dagger;',	// dagger
						8225 : '&Dagger;',	// 2 sided dagger
						8226 : '&bull;',	// bullet
						8230 : '&hellip;',	// horizontal ellipsis
						8240 : '&permil;',	// per mile
						8242 : '&prime;',	// prime, minute mark
						8243 : '&Prime;',	// double prime, second mark
						8249 : '&lsaquo;',	// single left angle quotation
						8250 : '&rsaquo;',	// single right angle quotation
						8254 : '&oline;',	// overline
						8260 : '&frasl;',	// fraction slash
						8472 : '&weierp;',	// weierstrassian function p
						8465 : '&image;',	// imaginary part of
						8476 : '&real;',	// real part of
						8482 : '&trade;',	// trademark sign
						8501 : '&alefsym;',	// first transfinite cardinal
						8592 : '&larr;',	// left arrow
						8593 : '&uarr;',	// up arrow
						8594 : '&rarr;',	// right arrow
						8595 : '&darr;',	// down arrow
						8596 : '&harr;',	// left-right arrow
						8629 : '&crarr;',	// carriage return arrow
						8656 : '&lArr;',	// left double arrow
						8657 : '&uArr;',	// up double arrow
						8658 : '&rArr;',	// right double arrow
						8659 : '&dArr;',	// down double arrow
						8660 : '&hArr;',	// left-right double arrow
						8704 : '&forall;',	// for all
						8706 : '&part;',	// partial differential
						8707 : '&exist;',	// there exists
						8709 : '&empty;',	// empty set, diameter
						8711 : '&nabla;',	// backward difference
						8712 : '&isin;',	// element of
						8713 : '&notin;',	// not an element of
						8715 : '&ni;',		// contains as member
						8719 : '&prod;',	// product sign
						8721 : '&sum;',		// n-ary sumation
						8722 : '&minus;',	// minus sign
						8727 : '&lowast;',	// asterisk operator
						8730 : '&radic;',	// radical sign
						8733 : '&prop;',	// proportional to
						8734 : '&infin;',	// infinity
						8736 : '&ang;',		// angle
						8743 : '&and;',		// logical and, wedge
						8744 : '&or;',		// logical or, vee
						8745 : '&cap;',		// intersection, cap
						8746 : '&cup;',		// union, cup
						8747 : '&int;',		// integral
						8756 : '&there4;',	// therefore
						8764 : '&sim;',		// tilde operator, similar to
						8773 : '&cong;',	// approximatley equal to
						8776 : '&asymp;',	// asymptotic to
						8800 : '&ne;',		// not equal to
						8801 : '&equiv;',	// identical to
						8804 : '&le;',		// less-than or equal to
						8805 : '&ge;',		// greater-than or equal to
						8834 : '&sub;',		// subset of
						8835 : '&sup;',		// superset of
						8836 : '&nsub;',	// not a subset of
						8838 : '&sube;',	// subset of or equal to
						8839 : '&supe;',	// superset of or qual to
						8853 : '&oplus;',	// direct sum
						8855 : '&otimes;',	// vector product
						8869 : '&perp;',	// perpendicular
						8901 : '&sdot;',	// dot operator
						8968 : '&lceil;',	// apl upstile
						8969 : '&rceil;',	// right ceiling
						8970 : '&lfloor;',	// apl downstile
						8971 : '&rfloor;',	// right floor
						9001 : '&lang;',	// left-pointing angle bracket, bra
						9002 : '&rang;',	// right-pointing angle bracket, ket
						9674 : '&loz;',		// lozenge
						9824 : '&spades;',	// black spade suit
						9827 : '&clubs;',	// black club suit, shamrock
						9829 : '&hearts;',	// black heart suit, valentine
						9830 : '&diams;'	// black diamond suit
					};
				}

					// Carry out replacement for each entities (see bug #4182)
					for(var code in entities_array) {
						entity = entities_array[code];

						var reg = new RegExp("&#" + code + ";", "g");
						html_code = html_code.replace(reg, entity);

						var reg = new RegExp(String.fromCharCode(code), "g");
						html_code = html_code.replace(reg, entity);
					}

					return html_code;
				};

			</script>

			<script type="text/javascript" src="<?php echo $this->web_path?>core/core.js"></script>
			<script type="text/javascript" src="<?php echo $this->web_path?>core/utility.js"></script>
			<script type="text/javascript" src="<?php echo $this->web_path?>core/htmlarea-lang-en.js"></script>
			<script type="text/javascript" src="<?php echo $this->web_path?>core/dialog.js"></script>

			<style type="text/css">
				@import url("<?php echo $this->web_path?>core/htmlarea.css");
			</style>
			<?php
			$WYSIWYG_INIT = TRUE;
		}//end if
		?>

			<?php $this->print_plugin_functions()?>

			<?php $this->print_config()?>

			<script type="text/javascript">
				// Handle unknown IE tags
				if (HTMLArea.is_ie) {
					// Create a dummy "abbr" element for the DOM
					var dom_abbr = document.createElement('ABBR');
				}
			</script>

			<textarea id="<?php echo $this->name?>" name="<?php echo $this->name?>" <?php echo $this->textarea_extras?>><?php echo htmlentities($this->contents, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET); ?></textarea>

			<script type="text/javascript">
				editor_<?php echo $this->name?> = new HTMLArea("<?php echo $this->name?>");
				editor_<?php echo $this->name?>._createToolbar();
				editor_<?php echo $this->name?>._createStatusBar();
				editor_<?php echo $this->name?>.updateToolbar(false);
				editor_<?php echo $this->name?>.set_dir_attr('<?php echo $this->dir_attribute?>');
		<?php
		if ($this->init_onload) {
		?>
				OtherOnLoad_<?php echo $this->name?> = (window.onload) ? window.onload :  new Function;
				window.onload = function() {
									editor_<?php echo $this->name?>.generate();
									OtherOnLoad_<?php echo $this->name?>();
								};
		<?php
		}
		?>
			</script>
		<?php

	}//end paint()


	/**
	* Processes and returns the submitted HTML from the editor
	* This allows plugins to modify the html as they see fit.
	*
	* @return string
	* @access public
	*/
	function process()
	{
		if (!isset($_REQUEST[$this->name])) return FALSE;
		$html = $_REQUEST[$this->name];

		// replace out special form tags back with real form tags
		$html = preg_replace('|<HTMLAREA_FORM ([^>]*?)>(.*?)</HTMLAREA_FORM>|is', '<FORM \\1>\\2</FORM>', $html);

		if (!isset($_POST['ees_content_no_filter'])) {
			for (reset($this->_loaded_plugins); NULL !== ($k = key($this->_loaded_plugins)); next($this->_loaded_plugins)) {
				$plugin =& $this->_loaded_plugins[$k];
				$plugin->process($html);
			}
		}

		return $html;

	}//end process()


	/**
	* Prints the config section of the WYSIWYG
	* The config section is where the plugins define where
	* and how they print
	*
	* @return void
	* @access public
	*/
	function print_config()
	{
			?>
			<script type="text/javascript">
				HTMLArea.Config_<?php echo $this->name?> = function () {
					this.version = "3.0";

					// enable creation of a status bar?
					this.statusBar = <?php echo ($this->show_status_bar) ? 'true' : 'false'; ?>;

					this.width = "<?php echo $this->width?>";
					this.height = "<?php echo $this->height?>";

					// use an iFrame or a DIV?
					this.bodyType = "<?php echo $this->body_type?>";

					// the next parameter specifies whether the toolbar should be included
					// in the size or not.
					this.sizeIncludesToolbar = true;

					this.bodyStyle = "background-color: #ffffff; font-family: verdana,sans-serif";
					this.styleSheets = <?php echo json_encode_array($this->stylesheets); ?>;
					
					this.editorURL = "";

					// URL-s
					this.imgURL = "<?php echo $this->web_path?>images/";
					this.pluginURL = "<?php echo $this->web_path?>plugins/";

					this.replaceNextLines = 0;
					this.plainTextInput = 0;

					this.toolbar = [
							<?php
							if (!empty($this->_plugins)) {
								$output = '';
								foreach ($this->_plugins as $plugin_group) {
									if (!empty($plugin_group)) {
										$pre_group_output = $output;
										$output .= '[ ';
										$printed_buttons = FALSE;
										foreach ($plugin_group as $name) {
											$plugin = $this->_loaded_plugins[$name];
											foreach ($plugin->buttons as $id => $button) {
												if (isset($button['menu_end'])) {
													$output .= ' ], ';
													continue;
												}
												if (isset($button['menu_start'])) $output .= '[ ';
												if ($id) $output .= '["'.$id.'"], ';
												$printed_buttons = TRUE;
											}
											unset($plugin);
										}
										if ($printed_buttons) {
											$output .= '["separator"] ],'."\n\t\t\t\t\t\t\t";
										} else {
											$output = $pre_group_output;
										}//end else
									}
								}
								$output = trim($output,"\t\n,");
								echo $output;
							}
							?>
						];

					//      ID,             CMD,                         ToolTip,              In text mode?,   Image Position, Context
					this.btnList = {
						<?php
						$output = '';
						foreach ($this->_loaded_plugins as $name => $plugin) {
							foreach ($plugin->buttons as $id => $button) {
								if (isset($button['menu_end'])) continue;
								if (!$button['command']) continue;
								$output .= "$id:".str_repeat(' ',30 - strlen($id))."[\"".$button['command']."\",".str_repeat(' ',30 - strlen($button['command']))."\"".$button['tooltip']."\",".str_repeat(' ',30 - strlen($button['tooltip']))."".$button['textmode'].",     ".(($button['textmode'] === 'true') ? ' ' : '')."".$button['position'].", \"".$button['context']."\"],\n\t\t\t\t\t\t";
							}
						}
						$output = trim($output,"\t\n,");
						echo $output;
						?>
					};

					// initialize tooltips from the I18N module
					for (var i in this.btnList) {
						var btn = this.btnList[i];
						if (typeof HTMLArea.I18N.tooltips[i] != "undefined") {
							btn[1] = HTMLArea.I18N.tooltips[i];
						}
					}
				};
			</script>
			<?php

	}//end print_config()


	/**
	* Prints the JS variables defined by each plugin
	*
	* @return void
	* @access public
	*/
	function print_plugin_vars()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_vars();
		}

	}//end print_plugin_vars()


	/**
	* Prints the keyboard shortcuts defined by each plugin
	*
	* @return void
	* @access public
	*/
	function print_plugin_shortcuts()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_shortcuts();
		}

	}//end print_plugin_shortcuts()


	/**
	* Prints the code to get the options array for select box plugins
	*
	* @return void
	* @access public
	*/
	function print_plugin_create_select()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_create_select();
		}

	}//end print_plugin_create_select()


	/**
	* Prints custom JS functions used by the plugins for each editor
	*
	* @return void
	* @access public
	*/
	function print_plugin_functions()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->paint();
		}

	}//end print_plugin_functions()


	/**
	* Prints JS functions used by the plugins that are generic
	*
	* @return void
	* @access public
	*/
	function print_plugin_generic_functions()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->paint_generic();
		}

	}//end print_plugin_generic_functions()


	/**
	* Prints the JS executed on click for each plugin
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_clicks()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_button_click();
		}

	}//end print_plugin_button_clicks()


	/**
	* Prints the JS executed on select for select box plugins
	*
	* @return void
	* @access public
	*/
	function print_plugin_combo_selected()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_combo_selected();
		}

	}//end print_plugin_combo_selected()


	/**
	* Prints the JS to create special types of buttons
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_type()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_button_type();
		}

	}//end print_plugin_button_type()


	/**
	* Prints the JS executed by each plugin when the toolbar is updated
	*
	* @return void
	* @access public
	*/
	function print_plugin_update_toolbar()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_update_toolbar();
		}

	}//end print_plugin_update_toolbar()


	/**
	* Prints any JS the plugin wants executed before the editable form element has its way
	*
	* @return void
	* @access public
	*/
	function print_plugin_exec_commands()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_exec_command();
		}

	}//end print_plugin_exec_commands()


	/**
	* Prints any JS the plugin wants executed on before getHTML gets the contents
	*
	* @return void
	* @access public
	*/
	function print_plugin_pre_get_html()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_pre_get_html();
		}

	}//end print_plugin_pre_get_html()


	/**
	* Prints any JS the plugin wants executed on returned HTML from getHTML
	*
	* @return void
	* @access public
	*/
	function print_plugin_get_html()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_get_html();
		}

	}//end print_plugin_get_html()


	/**
	* Prints any JS the plugin wants executed before submitting the form
	*
	* @return void
	* @access public
	*/
	function print_plugin_form_submit()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_form_submit();
		}

	}//end print_plugin_form_submit()


	/**
	* Prints any JS the plugin wants executed when an event is fired by the WYSIWYG's iframe or div
	*
	* @return void
	* @access public
	*/
	function print_plugin_event_handlers()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->print_plugin_event_handler();
		}

	}//end print_plugin_event_handlers()


	/**
	* Adds a plugin to the plugins array - adding functionality
	*
	* This must be done BEFORE calling the paint() function.
	* If no toolbar button group is currently open, the buttons for the
	* plugin will be added to a new group, otherwise they will join the
	* currently open one.
	*
	* @param string	$name	The unique name of this plugin
	*
	* @return boolean
	* @access public
	* @see open_group(), close_group()
	*/
	function add_plugin($name)
	{
		global $ROOT_PATH;
		if (!isset($this->_loaded_plugins[$name])) {
			require_once($ROOT_PATH.'/plugins/'.$name.'/'.$name.'.inc');
			$this->_loaded_plugins[$name] = new $name($this);
		}

		$own_group = FALSE;
		if ($this->_loaded_plugins[$name]->_show_in_toolbar && !$this->_group_open) {
			if (!$this->open_group()) return FALSE;
			$own_group = TRUE;
		}

		$groupid = count($this->_plugins) - 1;
		$this->_plugins[$groupid][] = $name;

		if ($own_group) return $this->close_group();
		return TRUE;

	}//end add_plugin()


	/**
	* Open a new toolbar grouping of buttons
	*
	* @return boolean
	* @access public
	* @see close_group()
	*/
	function open_group()
	{
		if ($this->_group_open) return FALSE;
		$this->_group_open = TRUE;
		$new_group = Array();
		return array_push($this->_plugins, $new_group);

	}//end open_group()


	/**
	* Close the currently open grouping on toolbar buttons
	*
	* @return boolean
	* @access public
	* @see open_group()
	*/
	function close_group()
	{
		if (!$this->_group_open) return FALSE;
		$this->_group_open = FALSE;
		return TRUE;

	}//end close_group()


	/**
	* Checks if a plugin has been loaded into the editor
	*
	* @param string	$name	The unique name of the plugin
	*
	* @return boolean
	* @access public
	* @see add_plugin()
	*/
	function plugin_loaded($name)
	{
		return isset($this->_loaded_plugins[$name]);

	}//end plugin_loaded()


	/**
	* Get a reference to a plugin object
	*
	* @param string	$name	The unique name of the plugin to get
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &get_plugin($name)
	{
		if (!isset($this->_loaded_plugins[$name])) {
			$null = NULL;
			return $null;
		}
		return $this->_loaded_plugins[$name];

	}//end get_plugin()


	/**
	* Sets the body type of the WYSIWYG editor (iframe or div)
	*
	* @param string	$type	The body type used to print/scale
	*
	* @return void
	* @access public
	*/
	function set_body_type($type='iframe')
	{
		$this->body_type = $type;

	}//end set_body_type()


	/**
	* Sets weather the WYSIWYG initilises itself onLoad for the window
	*
	* @param boolean	$init	Init or not
	*
	* @return void
	* @access public
	*/
	function set_init_onload($init=TRUE)
	{
		$this->init_onload = (bool) $init;

	}//end set_init_onload()


	/**
	* Sets weather we should show the status bar or not
	*
	* @param boolean	$show	Show or not
	*
	* @return void
	* @access public
	*/
	function set_show_status_bar($show=TRUE)
	{
		$this->show_status_bar = (bool)$show;

	}//end set_show_status_bar()


	/**
	* Sets the width of the WYSIWYG editor
	*
	* @param string	$width	The width of the editor
	*
	* @return void
	* @access public
	*/
	function set_width($width='auto')
	{
		if (is_numeric($width))	$width .= 'px';
		$this->width = $width;

	}//end set_width()


	/**
	* Sets the height of the WYSIWYG editor
	*
	* @param string	$height	The height of the editor
	*
	* @return void
	* @access public
	*/
	function set_height($height='auto')
	{
		if (is_numeric($height)) $height .= 'px';
		$this->height = $height;

	}//end set_height()


	/**
	* Sets extra HTML you want in the TEXTAREA tag
	* Usually would be used to set width and height
	*
	* @param string	$extras	The extra HTML
	*
	* @return void
	* @access public
	*/
	function set_textarea_extras($extras='')
	{
		$this->textarea_extras = trim($extras);

	}//end set_textarea_extras()


	/**
	* Sets the initial contents of the WYSIWYG editor
	*
	* @param string	$contents	Some HTML content
	*
	* @return void
	* @access public
	*/
	function set_contents($contents)
	{
		foreach ($this->_loaded_plugins as $name => $plugin) {
			$plugin->set_contents($contents);
		}

		// replace form tags in the HTML with special form tags that will not cuase JS errors
		// due to nested forms on the page
		$contents = preg_replace('|<FORM ([^>]*?)>(.*?)</FORM>|is', '<HTMLAREA_FORM \\1>\\2</HTMLAREA_FORM>', $contents);
		$this->contents = $contents;

	}//end set_contents()


	/**
	* Sets the stylesheet for the WYSIWYG editor to use
	*
	* @param string	$stylesheet	The href of the stylesheet
	*
	* @return void
	* @access public
	*/
	function set_stylesheet($stylesheet)
	{
		$this->stylesheets[] = $stylesheet;

	}//end set_stylesheet()


	/**
	* Sets the dir attribute for the body in WYSIWYG editor
	*
	* @param string	$dir_attribute	value of dir attribute
	*
	* @return void
	* @access public
	*/
	function set_dir_attr($dir_attribute)
	{
		$this->dir_attribute = $dir_attribute;
	}//end set_dir_attr()


}//end class

?>
