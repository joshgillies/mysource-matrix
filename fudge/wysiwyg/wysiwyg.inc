<?php

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
	* @var string
	* @see paint()
	*/
	var $name  = 'wysiwyg';

	/**
	* If the WYSIWYG should use an iframe or div to print/scale
	* A div will scale better than an iframe
	* @var string
	* @see paint(), set_body_type()
	*/
	var $body_type  = 'iframe';

	/**
	* Should the WYSIWYG initilise itself onLoad for the window?
	* Set to 'no' if you want to init the WYSIWYG yourself
	* @var string
	* @see paint(), set_init_onload()
	*/
	var $init_onload  = 'yes';

	/**
	* The relative path for web accessable files
	* @var string
	* @see paint()
	*/
	var $web_path  = '';

	/**
	* The stylesheet to use in the editor
	* @var string
	* @see paint(), set_stylesheet()
	*/
	var $stylesheet  = '';

	/**
	* The width of the WYSIWYG editor
	* @var string
	* @see print_config(), set_width()
	*/
	var $width  = 'auto';

	/**
	* The height of the WYSIWYG editor
	* @var string
	* @see print_config(), set_height();
	*/
	var $height  = 'auto';

	/**
	* Extra HTML code to add in the TEXTAREA tag
	* @var string
	* @see print_config(), set_textarea_extras();
	*/
	var $textarea_extras = '';

	/**
	* The HTML to print in the editor
	* @var string
	* @see paint(), set_contents();
	*/
	var $contents = '';

	/**
	* An array of plugins objects to provide functionality to the editor
	* @var array
	* @see add_plugin()
	*/
	var $plugins = Array();

	/**
	* Has this WYSIWYG printed its header info yet?
	* We need this in case we have multiple WYSIWYG's on
	* the same page. We dont want to print the JS more than once
	* @var boolean
	* @see paint()
	*/
	var $init = false;


	/**
	* Constructor
	*
	* @param   string $name A unique name for this WYSIWYG
	* @returns object WYSIWYG
	* @access  public
	*/
	function wysiwyg($name='wysiwyg', $web_path='') {
		$this->name = $name;
		$this->web_path = $web_path;
		global $ROOT_PATH;
		$ROOT_PATH = realpath(dirname(__FILE__)).'/';
	}


	/**
	* Paints the JS needed for the WYSIWYG and the editable textarea
	*
	* @returns void
	* @access  public
	*/
	function paint() {
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
						this._mode = "wysiwyg";
						this._initialised = false;
<?php $this->print_plugin_vars() ?>
					}
				};
			</script>

			<script type="text/javascript" language="Javascript">
				// Creates the toolbar and appends it to the _htmlarea
				HTMLArea.prototype._createToolbar = function () {
					var editor = this; // to access this in nested functions

					var toolbar = document.createElement("div");
					this._toolbar = toolbar;
					toolbar.className = "toolbar";
					toolbar.unselectable = "1";
					var tb_objects = new Object();
					this._toolbarObjects = tb_objects;

					// appends a new button to toolbar
					function createButton(txt) {
						// updates the state of a toolbar element
						function setButtonStatus(id, newval) {
							var oldval = this[id];
							var el = this.element;
							if (oldval != newval) {
								switch (id) {
									case "enabled":
									if (newval) {
										HTMLArea._removeClass(el, "buttonDisabled");
										el.disabled = false;
									} else {
										HTMLArea._addClass(el, "buttonDisabled");
										el.disabled = true;
									}
									break;
									case "active":
									if (newval) {
										HTMLArea._addClass(el, "buttonPressed");
									} else {
										HTMLArea._removeClass(el, "buttonPressed");
									}
									break;
								}
								this[id] = newval;
							}
						};

						// this function will handle creation of combo boxes
						function createSelect() {
							var options = null;
							var el = null;
							var sel = null;
							var cmd = null;
							switch (txt) {
<?php $this->print_plugin_create_select()?>
							}
							if (options) {
								sel = document.createElement("select");
								var obj = {
									name: txt,     // field name
									element: sel,   // the UI element (SELECT)
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
						// the element that will be created
						var el = null;
						var btn = null;
						switch (txt) {
							case "separator":
								el = document.createElement("span");
								el.className = "separator";
								el.style.top = (HTMLArea.is_gecko ? -9 : 0);
							break;
							case "space":
								el = document.createElement("span");
								el.className = "space";
								el.style.top = (HTMLArea.is_gecko ? -9 : 0);
							break;
							case "linebreak":
								el = document.createElement("div");
								el.className = "space";
							break;
<?php $this->print_plugin_button_type()?>
							default:
								btn = editor.config.btnList[txt];
							break;
						}
						if (!el && btn) {
							el = document.createElement("span");
							el.title = btn[1];
							el.className = "button";

							// let's just pretend we have a button object, and
							// assign all the needed information to it.
							var obj = {
								name: txt,     // the button name (i.e. 'bold')
								element: el,   // the UI element (DIV)
								enabled: true, // is it enabled?
								active: false, // is it pressed?
								text: btn[2],  // enabled in text mode?
								cmd: btn[0],   // the command ID
								state: setButtonStatus // for changing state
							};
							tb_objects[txt] = obj;

							// handlers to emulate nice flat toolbar buttons
							HTMLArea._addEvent(el, "mouseover", function () {
								if (obj.enabled) {
									HTMLArea._addClass(el, "buttonHover");
								}
							});
							HTMLArea._addEvent(el, "mouseout", function () {
								if (obj.enabled) with (HTMLArea) {
									_removeClass(el, "buttonHover");
									_removeClass(el, "buttonActive");
									(obj.active) && _addClass(el, "buttonPressed");
								}
							});
							HTMLArea._addEvent(el, "mousedown", function (ev) {
								if (obj.enabled) with (HTMLArea) {
									_addClass(el, "buttonActive");
									_removeClass(el, "buttonPressed");
									_stopEvent(is_ie ? window.event : ev);
								}
							});
							// when clicked, do the following:
							HTMLArea._addEvent(el, "click", function (ev) {
								if (obj.enabled) with (HTMLArea) {
									_removeClass(el, "buttonActive");
									_removeClass(el, "buttonHover");
									editor._buttonClicked(txt);
									_stopEvent(is_ie ? window.event : ev);
								}
							});

							var buttonID = editor._uniqueID + "_" + btn[0];
							el.innerHTML += "<img src=\"" + editor.imgURL("blank.gif") + "\" width=18 height=20 border=0>";

							// output an image tag with the src of the toolbar image
							// the image is loaded once and then the server is not hit again
							document.write("<img src=\"" + editor.imgURL("toolbar.gif") + "\" id=\"" + buttonID + "\">");
							var img = document.getElementById(buttonID);
							
							// work out where this button image is in the big toolbar image
							var btnTop = Math.floor(btn[3] / 5);
							var btnLeft = ((btn[3] - (btnTop * 5)) * 18);

							// using a single image, position each of the div tags for the buttons
							el.style.position = "relative";
							el.style.top = (HTMLArea.is_gecko ? -9 : 2);
					
							img.style.position = "absolute";
							img.style.left = -1 * (btnLeft); // use the index to get the offset
							img.style.top = -1 * (btnTop*20);
							
							img.style.clip = "rect("+(btnTop*20)+" "+(btnLeft+18)+" "+(btnTop*20+20)+" "+btnLeft+")"; // clipping rectangle

							el.appendChild(img);
						} else if (!el) {
							el = createSelect();
						}
						if (el) {
							toolbar.appendChild(el);
						} else {
							alert("FIXME: Unknown toolbar item: " + txt);
						}
						return el;
					};

					for (var i in this.config.toolbar) {
						var group = this.config.toolbar[i];
						for (var j in group) {
							createButton(group[j]);
						}
					}

				};

				// updates enabled/disable/active state of the toolbar elements
				HTMLArea.prototype.updateToolbar = function() {
					var doc = this._doc;
					var text = (this._mode == "textmode");
					for (var i in this._toolbarObjects) {
						var btn = this._toolbarObjects[i];
						var cmd = btn.cmd;
						if (typeof cmd == "function") {
							continue;
						}
						cmd = cmd.toLowerCase();
						btn.state("enabled", !text || btn.text);
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
					switch (cmd.toLowerCase()) {
<?php $this->print_plugin_button_clicks()?>
						default:
							this._execCommand(btn[0], false, "");
						break;
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
					setTimeout(function() {
						editor.updateToolbar();
					}, 50);
				};

				// retrieve the HTML
				HTMLArea.prototype.getHTML = function() {
					var retVal = '';
					switch (this._mode) {
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
							alert("Mode <" + this._mode + "> not defined!");
							return false;
					}
					// let any plugins modify the returned HTML if they want to
<?php $this->print_plugin_get_html()?>

					return this.make_relative_hrefs(retVal);
				};

				 ////////////////////////////////////////////////////////////////////////
				// because the editor automatically prepend the http:// business to 
				// all relative urls, remove them from the html
				HTMLArea.prototype.make_relative_hrefs = function(html_code) {

					var html = new String(html_code);
					var e  = '';
					var re = null;

					// now firstly remove any references to this pop-ups url
					var current_location = document.location.toString();
//alert(html);
					// escape any special reg exp chars in the location
					current_location = current_location.replace(/([.?+*^$\\\[\]\(\)\{\}|])/gi, '\\$1');
					re = new RegExp(current_location, "gi");
					html = html.replace(re, "");
//alert(html);
					// now replace any ampersands with '&amp;' as IE seems to some times for some reason (even in href's)
					current_location = current_location.replace(/\&/gi, "&amp;");
					re = new RegExp(current_location, "gi");
					html = html.replace(re, "");
//alert(html);
					<?php
						//$site_url  = str_replace('.', '\\\\.', $site->get_url());
						//$site_href = str_replace('.', '\\\\.', $site->get_href());
					?>
					// replace every god-dam occurance of the full-urls

					// links to assets in the backend 
					e = 'http[s]?://<?php echo $_SERVER['HTTP_HOST'].$_SERVER["PHP_SELF"]?>?(\\?a=[0-9]+)';
//alert(e);
					re = new RegExp(e, "gi");
					html = html.replace(re, "./$1");
//alert(html);
					// links to stuff in the backend directory
					e = 'http[s]?://<?php echo $_SERVER['HTTP_HOST'].$_SERVER["PHP_SELF"]?>';
//alert(e);
					re = new RegExp(e, "gi");
					html = html.replace(re, "");

					// links to sites, pages or files in the frontend
					//e = '\\.*((<?=$site_url?>)|(<?=$site_href?>))/?(\\?[spf]=[0-9]+)';
					//re = new RegExp(e, "gi");
					//html = html.replace(re, "./$4");
//alert(html);
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

			<textarea id="<?php echo $this->name?>" name="<?php echo $this->name?>" <?php echo $this->textarea_extras?>><?php echo $this->contents?></textarea>

			<script type="text/javascript" language="Javascript">
				editor_<?php echo $this->name?> = new HTMLArea("<?php echo $this->name?>");
				editor_<?php echo $this->name?>._createToolbar();
		<?php
		if ($this->init_onload == 'yes') {
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
	}


	/**
	* Prints the config section of the WYSIWYG
	* The config section is where the plugins define where
	* and how they print
	*
	* @returns void
	* @access  public
	*/
	function print_config() {
			?>
			<script type="text/javascript" language="Javascript">
				HTMLArea.Config_<?php echo $this->name?> = function () {
					this.version = "3.0";

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

					this.toolbar = [ 
							<?php
							if (!empty($this->plugins)) {
								$buttons_done = 0;
								$orphaned = array();
								$output = '';
								foreach ($this->plugins as $name => $plugin) {
									if (count($plugin->buttons) > 1) {
										// a group of buttons
										$output .= "[ ";
										foreach ($plugin->buttons as $id => $button) if ($id) $output .= '"'.$id.'", ';
										$output .= '"separator" ],'."\n\t\t\t\t\t\t\t";
									} else {
										foreach ($plugin->buttons as $id => $button) $orphaned[] = $id;
									}
								}

								if (!empty($orphaned)) {
									$output .= "[ ";
									foreach ($orphaned as $id) $output .= '"'.$id.'", ';
									$output = trim($output,', ');
									$output .= ' ],'."\n\t\t\t\t\t\t\t";
								}
								$output = trim($output,"\t\n,");
								echo $output;
							}
							?>
						];
					
					//      ID,             CMD,                         ToolTip,              In text mode?,   Image Position
					this.btnList = {
						<?php
						$output = '';
						foreach ($this->plugins as $name => $plugin) {
							foreach ($plugin->buttons as $id => $button) {
								if (!$button['command']) continue;
									$output .= "$id:".str_repeat(' ',19 - strlen($id))."[\"".$button['command']."\",".str_repeat(' ',25 - strlen($button['command']))."\"".$button['tooltip']."\",".str_repeat(' ',25 - strlen($button['tooltip']))."".$button['textmode'].",     ".(($button['textmode'] === 'true') ? ' ' : '')."".$button['position']."],\n\t\t\t\t\t\t";
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
	}


	/**
	* Prints the JS variables defined by each plugin
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_vars() {
		foreach ($this->plugins as $name => $plugin) $plugin->print_plugin_vars();
	}


	/**
	* Prints the keyboard shortcuts defined by each plugin
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_shortcuts() {
		foreach ($this->plugins as $name => $plugin) $plugin->print_plugin_shortcuts();
	}


	/**
	* Prints the code to get the options array for select box plugins
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_create_select() {
		foreach ($this->plugins as $name => $plugin) $plugin->print_plugin_create_select();
	}


	/**
	* Prints custom JS functions used by the plugins for each editor
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_functions() {
		foreach ($this->plugins as $name => $plugin) $plugin->paint();
	}


	/**
	* Prints JS functions used by the plugins that are generic
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_generic_functions() {
		foreach ($this->plugins as $name => $plugin) $plugin->paint_generic();
	}


	/**
	* Prints the JS executed on click for each plugin
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_clicks() {
		foreach ($this->plugins as $name => $plugin) $plugin->print_plugin_button_click();
	}


	/**
	* Prints the JS executed on select for select box plugins
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_combo_selected() {
		foreach ($this->plugins as $name => $plugin) $plugin->print_plugin_combo_selected();
	}


	/**
	* Prints the JS to create special types of buttons
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_type() {
		foreach ($this->plugins as $name => $plugin) $plugin->print_plugin_button_type();
	}


	/**
	* Prints the JS executed by each plugin when the toolbar is updated
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_update_toolbar() {
		foreach ($this->plugins as $name => $plugin) $plugin->print_plugin_update_toolbar();
	}


	/**
	* Prints any JS the plugin wants executed before the editable form element has its way
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_exec_commands() {
		foreach ($this->plugins as $name => $plugin) $plugin->print_plugin_exec_command();
	}


	/**
	* Prints any JS the plugin wants executed on returned HTML from getHTML
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_get_html() {
		foreach ($this->plugins as $name => $plugin) $plugin->print_plugin_get_html();
	}


	/**
	* Adds a plugin to the plugins array - adding functionality
	* This must be done BEFORE calling the paint() function
	*
	* @param   string $name The unique name of this plugin
	* @returns boolean
	* @access  public
	*/
	function add_plugin($name) {
		global $ROOT_PATH;
		if (isset($this->plugins[$name])) return;
		$filepath = $ROOT_PATH.'/plugins/'.$name.'/'.$name.'.inc';
		include_once($filepath);
		$plugin = new $name($this);
		$this->plugins[$name] = $plugin;
		return true;
	}


	/**
	* Removes a plugin from the plugins array - removing functionality
	* This must be done BEFORE calling the paint() function
	*
	* @param   string $name The unique name of this plugin
	* @returns boolean
	* @access  public
	*/
	function remove_plugin($name) {
		unset($this->plugins[$name]);
		return true;
	}


	/**
	* Sets the body type of the WYSIWYG editor (iframe or div)
	*
	* @param   string $type The body type used to print/scale
	* @returns void
	* @access  public
	*/
	function set_body_type($type='iframe') {
		$this->body_type = $type;
	}


	/**
	* Sets weather the WYSIWYG initilises itself onLoad for the window
	*
	* @param   string $init Init or not (yes or no)
	* @returns void
	* @access  public
	*/
	function set_init_onload($init='yes') {
		$this->init_onload = trim(strtolower($init));
	}


	/**
	* Sets the width of the WYSIWYG editor
	*
	* @param   string $width The width of the editor
	* @returns void
	* @access  public
	*/
	function set_width($width='auto') {
		$this->width = $width;
	}


	/**
	* Sets the height of the WYSIWYG editor
	*
	* @param   string $height The height of the editor
	* @returns void
	* @access  public
	*/
	function set_height($height='auto') {
		$this->height = $height;
	}


	/**
	* Sets extra HTML you want in the TEXTAREA tag
	* Usually would be used to set width and height
	*
	* @param   string $extras The extra HTML
	* @returns void
	* @access  public
	*/
	function set_textarea_extras($extras='') {
		$this->textarea_extras = trim($extras);
	}


	/**
	* Sets the initial contents of the WYSIWYG editor
	*
	* @param   string $content Some HTML content
	* @returns void
	* @access  public
	*/
	function set_contents($contents) {
		$this->contents = $contents;
	}


	/**
	* Sets the stylesheet for the WYSIWYG editor to use
	*
	* @param   string $stylesheet The href of the stylesheet
	* @returns void
	* @access  public
	*/
	function set_stylesheet($stylesheet) {
		$this->stylesheet = $stylesheet;
	}
}

?>