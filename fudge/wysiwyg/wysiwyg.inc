<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: wysiwyg.inc,v 1.25 2003/11/11 23:35:51 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class wysiwyg {

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
	var $init_onload  = true;


	/**
	* Should we show the status bar at the top of the editor?
	*
	* @var boolean
	* @see print_config(), set_show_status_bar()
	*/
	var $show_status_bar = true;

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
	var $stylesheet  = '';

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
	* Regular expressions to convert URLs to HREFs
	*
	* @var string
	* @see print_config(), add_relative_href_check();
	*/
	var $relative_href_checks = Array();

	/**
	* Regular expressions to convert HREFs to URLs
	*
	* @var string
	* @see print_config(), add_absolute_url_check();
	*/
	var $absolute_url_checks = Array();

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
	var $_group_open = false;

	/**
	* Has this WYSIWYG printed its header info yet?
	* We need this in case we have multiple WYSIWYG's on
	* the same page. We dont want to print the JS more than once
	*
	* @var boolean
	* @see paint()
	*/
	var $init = false;


	/**
	* Constructor
	*
	* @param string	$name A unique name for this WYSIWYG
	*
	* @access public
	*/
	function wysiwyg($name='wysiwyg', $web_path='')
	{
		$this->name = $name;
		$this->web_path = $web_path;
		global $ROOT_PATH;
		$ROOT_PATH = realpath(dirname(__FILE__)).'/';

	}//end constructor()


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
			<script type="text/javascript" language="Javascript">
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

			<script type="text/javascript" language="Javascript">
				// Creates the toolbar and appends it to the _htmlarea
				HTMLArea.prototype._createToolbar = function () {
					var editor = this; // to access this in nested functions

					var toolbar = document.createElement("div");
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
						var buttonID = editor._uniqueID + "_" + this['name'];
						buttonID = buttonID.toLowerCase();
						var img = document.getElementById(buttonID);
						if (img && oldval != newval) {
							switch (id) {
								case "enabled":
									if (newval) {
										HTMLArea._removeClass(el, "htmlarea-buttonDisabled");
										if (img) { img.src = editor.imgURL("toolbar.png"); }
										el.disabled = false;
									} else {
										HTMLArea._addClass(el, "htmlarea-buttonDisabled");
										if (img) { img.src = editor.imgURL("toolbar_disabled.png"); }
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
							el = document.createElement("span");
							el.style.position = "relative";
							el.innerHTML += "<img src=\"" + editor.imgURL("blank.gif") + "\" width=" + sel.offsetWidth + " height=20 border=0>";
							el.appendChild(sel);
							el.style.top = -5;

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
								el = document.createElement("span");
								el.className = "htmlarea-separator";
								el.style.top = (HTMLArea.is_gecko ? -4 : 0);
							break;
							case "space":
								el = document.createElement("span");
								el.className = "htmlarea-space";
								el.style.top = (HTMLArea.is_gecko ? -4 : 0);
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
							el = document.createElement("span");
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

							// Mozilla does like us setting the width and height of SPANs in the
							// stylesheet (or at all actually) so we need to fill the SPAN with
							// some dummy HTML to size it
							if (HTMLArea.is_gecko) {
								el.innerHTML = "<img src=\"" + editor.imgURL("blank.gif") + "\" width=18 height=20 border=0>";
							}//end gecko smells!!

							// output an image tag with the src of the toolbar image
							// the image is loaded once and then the server is not hit again
							var buttonID = editor._uniqueID + "_" + txt;
							buttonID = buttonID.toLowerCase();
							el.id = buttonID + "_span";
							document.write("<img src=\"" + editor.imgURL("toolbar.png") + "\" id=\"" + buttonID + "\">");
							var img = document.getElementById(buttonID);

							// work out where this button image is in the big toolbar image
							var btnTop = Math.floor(btn[3] / 5);
							var btnLeft = ((btn[3] - (btnTop * 5)) * 18);

							// using a single image, position each of the span tags for the buttons
							el.style.position = "relative";
							el.style.top = (HTMLArea.is_gecko ? -4 : 2);

							img.style.position = "absolute";
							img.style.left = -1 * (btnLeft); // use the index to get the offset
							img.style.top = -1 * (btnTop*20);

							img.style.clip = "rect("+(btnTop*20)+" "+(btnLeft+18)+" "+(btnTop*20+20)+" "+btnLeft+")"; // clipping rectangle

							el.appendChild(img);
						} else if (!el) {
							el = createSelect(txt);
						}
						if (el) {
							container.appendChild(el);
						} else {
							alert("FIXME: Unknown toolbar item: " + txt);
						}
						return el;
					};

					// preload the disabled button image
					document.write("<img src=\"" + editor.imgURL("toolbar_disabled.png") + "\" id=\"toolbar_disabled\" style=\"visibility: hidden;\" width=\"1\" height=\"1\">");

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
					this._textArea.value = escape(html);
				};
			</script>

<?php $this->print_plugin_generic_functions()?>

			<script type="text/javascript" language="Javascript">
				// txt is the name of the button, as in config.toolbar
				HTMLArea.prototype._buttonClicked = function(txt) {
					var editor = this; // needed in nested functions

					this.focusEditor();

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
							alert("FIXME: combo box " + txt + " not implemented");
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

				// A generic event handler for things that happen in the IFRAME's document.
				// This function also handles key bindings.
				HTMLArea.prototype._editorEvent = function(ev) {
					var editor = this;
					var keyEvent = (HTMLArea.is_ie && ev.type == "keydown") || (ev.type == "keypress");
					if (keyEvent && ev.ctrlKey) {
						var sel = null;
						var range = null;
						var key = String.fromCharCode(HTMLArea.is_ie ? ev.keyCode : ev.charCode).toLowerCase();
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
							} else if (HTMLArea.is_ie) {
								retVal = this._docContent.innerHTML;
							}
						break;
						default:
							alert("Mode <" + this._editMode + "> not defined!");
							return false;
					}
					// let any plugins modify the returned HTML if they want to
<?php $this->print_plugin_get_html()?>

					if (this._editMode == "textmode") {
						retVal = this.make_absolute_urls(retVal);
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
					re = new RegExp(current_location, "gi");
					html = html.replace(re, "");

					// now replace any ampersands with '&amp;' as IE seems to anyway sometimes (even in href's)
					current_location = current_location.replace(/\&/gi, "&amp;");
					re = new RegExp(current_location, "gi");
					html = html.replace(re, "");
					for (var i = 0; i < this.config.relativeHrefChecks.length; i++) {
						e = this.config.relativeHrefChecks[i][0];
						re = new RegExp(e, "gi");
						html = html.replace(re, this.config.relativeHrefChecks[i][1]);
					}

					// any other links
					e = 'http[s]?://<?php echo $_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF']?>';
					re = new RegExp(e, "gi");
					html = html.replace(re, "");

					return html;
				};

				 ////////////////////////////////////////////////////////////////////////
				// reconstruct all relative HREFs into absolute URLs before the browser
				// decides to do it for us anyway
				HTMLArea.prototype.make_absolute_urls = function(html_code) {

					var html = new String(html_code);
					var e  = '';
					var re = null;

					for (var i = 0; i < this.config.absoluteUrlChecks.length; i++) {
						e = this.config.absoluteUrlChecks[i][0];
						re = new RegExp(e, "gi");
						html = html.replace(re, this.config.absoluteUrlChecks[i][1]);

					}

					return html;
				};

			</script>

			<script type="text/javascript" language="Javascript" src="<?php echo $this->web_path?>core/core.js"></script>
			<script type="text/javascript" language="Javascript" src="<?php echo $this->web_path?>core/utility.js"></script>
			<script type="text/javascript" language="Javascript" src="<?php echo $this->web_path?>core/htmlarea-lang-en.js"></script>
			<script type="text/javascript" language="Javascript" src="<?php echo $this->web_path?>core/dialog.js"></script>

			<style type="text/css">
				@import url("<?php echo $this->web_path?>core/htmlarea.css");
			</style>
			<?php
			$WYSIWYG_INIT = true;
		}
		?>

			<?php $this->print_plugin_functions()?>

			<?php $this->print_config()?>

			<textarea id="<?php echo $this->name?>" name="<?php echo $this->name?>" <?php echo $this->textarea_extras?>><?php echo rawurlencode($this->contents)?></textarea>

			<script type="text/javascript" language="Javascript">
				editor_<?php echo $this->name?> = new HTMLArea("<?php echo $this->name?>");
				editor_<?php echo $this->name?>._createToolbar();
				editor_<?php echo $this->name?>._createStatusBar();
				editor_<?php echo $this->name?>.updateToolbar(false);
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
		$html = rawurldecode((isset($_POST[$this->name])) ? $_POST[$this->name] : '');

		// replace out special form tags back with real form tags
		$html = preg_replace('|<HTMLAREA_FORM ([^>]*?)>(.*?)</HTMLAREA_FORM>|i', '<FORM \\1>\\2</FORM>', $html);

		for(reset($this->_loaded_plugins); null !== ($k = key($this->_loaded_plugins)); next($this->_loaded_plugins)) {
			$plugin =& $this->_loaded_plugins[$k];
			$plugin->process($html);
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
			<script type="text/javascript" language="Javascript">
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
					this.styleSheet = "<?php echo $this->stylesheet?>";
					this.editorURL = "";

					// URL-s
					this.imgURL = "<?php echo $this->web_path?>images/";
					this.pluginURL = "<?php echo $this->web_path?>plugins/";

					this.replaceNextLines = 0;
					this.plainTextInput = 0;

					this.relativeHrefChecks = Array(
						<?php
						$i = count($this->relative_href_checks);
						foreach ($this->relative_href_checks as $find => $replace) {
							echo "Array(\"$find\", \"$replace\")".(($i > 1) ? ',' : '')."\n";
							$i--;
						}
						?>
					);
					this.absoluteUrlChecks = Array(
						<?php
						$i = count($this->absolute_url_checks);
						foreach ($this->absolute_url_checks as $find => $replace) {
							echo "Array(\"$find\", \"$replace\")".(($i > 1) ? ',' : '')."\n";
							$i--;
						}
						?>
					);

					this.toolbar = [
							<?php
							if (!empty($this->_plugins)) {
								$output = '';
								foreach ($this->_plugins as $plugin_group) {
									$output .= "[ ";
									foreach ($plugin_group as $name) {
										$plugin = $this->_loaded_plugins[$name];
										foreach ($plugin->buttons as $id => $button) {
											if (isset($button['menu_end'])) {
												$output .= ' ], ';
												continue;
											}
											if (isset($button['menu_start'])) $output .= '[ ';
											if ($id) $output .= '["'.$id.'"], ';
										}
										unset($plugin);
									}
									$output .= '["separator"] ],'."\n\t\t\t\t\t\t\t";
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
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_vars();

	}//end print_plugin_vars()


	/**
	* Prints the keyboard shortcuts defined by each plugin
	*
	* @return void
	* @access public
	*/
	function print_plugin_shortcuts()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_shortcuts();

	}//end print_plugin_shortcuts()


	/**
	* Prints the code to get the options array for select box plugins
	*
	* @return void
	* @access public
	*/
	function print_plugin_create_select()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_create_select();

	}//end print_plugin_create_select()


	/**
	* Prints custom JS functions used by the plugins for each editor
	*
	* @return void
	* @access public
	*/
	function print_plugin_functions()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->paint();

	}//end print_plugin_functions()


	/**
	* Prints JS functions used by the plugins that are generic
	*
	* @return void
	* @access public
	*/
	function print_plugin_generic_functions()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->paint_generic();

	}//end print_plugin_generic_functions()


	/**
	* Prints the JS executed on click for each plugin
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_clicks()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_button_click();

	}//end print_plugin_button_clicks()


	/**
	* Prints the JS executed on select for select box plugins
	*
	* @return void
	* @access public
	*/
	function print_plugin_combo_selected()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_combo_selected();

	}//end print_plugin_combo_selected()


	/**
	* Prints the JS to create special types of buttons
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_type()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_button_type();

	}//end print_plugin_button_type()


	/**
	* Prints the JS executed by each plugin when the toolbar is updated
	*
	* @return void
	* @access public
	*/
	function print_plugin_update_toolbar()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_update_toolbar();

	}//end print_plugin_update_toolbar()


	/**
	* Prints any JS the plugin wants executed before the editable form element has its way
	*
	* @return void
	* @access public
	*/
	function print_plugin_exec_commands()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_exec_command();

	}//end print_plugin_exec_commands()


	/**
	* Prints any JS the plugin wants executed on returned HTML from getHTML
	*
	* @return void
	* @access public
	*/
	function print_plugin_get_html()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_get_html();

	}//end print_plugin_get_html()


	/**
	* Prints any JS the plugin wants executed before submitting the form
	*
	* @return void
	* @access public
	*/
	function print_plugin_form_submit()
	{
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->print_plugin_form_submit();

	}//end print_plugin_form_submit()


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

		$own_group = false;
		if ($this->_loaded_plugins[$name]->_show_in_toolbar && !$this->_group_open) {
			if (!$this->open_group()) return false;
			$own_group = true;
		}

		$groupid = count($this->_plugins) - 1;
		$this->_plugins[$groupid][] = $name;

		if ($own_group) return $this->close_group();
		return true;

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
		if ($this->_group_open) return false;
		$this->_group_open = true;
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
		if (!$this->_group_open) return false;
		$this->_group_open = false;
		return true;

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
	* @return object | NULL
	* @access public
	*/
	function &get_plugin($name)
	{
		if (!isset($this->_loaded_plugins[$name])) return null;
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
	function set_init_onload($init=true)
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
	function set_show_status_bar($show=true)
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
		foreach ($this->_loaded_plugins as $name => $plugin) $plugin->set_contents($contents);

		// replace form tags in the HTML with special form tags that will not cuase JS errors
		// due to nested forms on the page
		$contents = preg_replace('|<FORM ([^>]*?)>(.*?)</FORM>|i', '<HTMLAREA_FORM \\1>\\2</HTMLAREA_FORM>', $contents);
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
		$this->stylesheet = $stylesheet;

	}//end set_stylesheet()


	/**
	* Add a relative HREF check to the editor
	*
	* @param string	$find		the pattern to match the absolute URL
	* @param string	$replace	the replacement to make the relative HREF
	*
	* @return void
	* @access public
	*/
	function add_relative_href_check($find, $replace)
	{
		$find = str_replace('\\', '\\\\\\', $find);
		$replace = str_replace('\\', '\\\\\\', $replace);
		$this->relative_href_checks[$find] = $replace;

	}//end add_relative_href_check()


	/**
	* Add an absolute URL check to the editor
	*
	* @param string	$find		the pattern to match the relative HREF
	* @param string	$replace	the replacement to make the absolute URL
	*
	* @return void
	* @access public
	*/
	function add_absolute_url_check($find, $replace)
	{
		$find = str_replace('\\', '\\\\\\', $find);
		$replace = str_replace('\\', '\\\\\\', $replace);
		$this->absolute_url_checks[$find] = $replace;

	}//end add_absolute_url_check()


}//end class

?>