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
* $Id: bodycopy_edit_fns.js,v 1.8 2012/08/30 01:09:05 ewang Exp $
*
*/

	// this is an array of data that each element (divs, tables, rows, and cells)
	// can place data for use in editing
	var bodycopy_current_data = new Object();
	var bodycopy_saved        = new Object();

	// this is an object that gets set everytime something is getting edited
	// (as apposed to inserted or deleted) so that the pop-ups can get the info once
	// they have loaded themselves
	var bodycopy_current_edit = new Object();
	bodycopy_current_edit["data"] = null;
	bodycopy_current_edit["bodycopy_name"] = null;

	var bodycopy_initialised = false;	// true once init() has been run
	var bodycopy_popup = null;			// pointer to the popup Layer_Handler Object
	var bodycopy_popup_visible = false;	// boolean indicating whether the popup is visisble ( not access directly )
	var bodycopy_nested_doc = null;		// pointer to the Netscape Layer or the IE Iframe

	// initialise the popup
	function bodycopy_init() {
		bodycopy_popup   = new Layer_Handler("bodycopyPopupDiv", 0,init_layer_width,init_layer_height,0);
		set_bodycopy_nested_doc();
		bodycopy_hide_popup();
		bodycopy_initialised = true;
		bodycopy_otheronload();
	}

	var bodycopy_otheronload = (window.onload) ? window.onload :  new Function;
	window.onload = bodycopy_init;

	// generic function used everywhere to send the form
	function bodycopy_submit(bodycopy_action, bodycopy_name, bodycopy_data) {
		var form = document.main_form;

		form.bodycopy_action.value = bodycopy_action;
		form.bodycopy_name.value   = bodycopy_name;

		// pack up the passed object
		form.bodycopy_data.value = var_serialise(bodycopy_data);

		// need to call the onsubmit event explicitly or it will
		// not get called when we do a form.submit()
		form.onsubmit();
		form.submit();
	}//end bodycopy_submit()

	function set_bodycopy_nested_doc() {
		if (is_ie4up) {
			bodycopy_nested_doc = bodycopyFrame;
		} else if (is_dom) {
			bodycopy_nested_doc = document.getElementById("bodycopyFrame");
		} else {
			bodycopy_nested_doc = bodycopy_popup.layer;
		}//end if
	}

	function get_bodycopy_popup_visibilty() {
		return bodycopy_popup_visible;
	}

	function bodycopy_show_popup(file, width, height, bodycopy_id, bodycopy_type) {
		if (!bodycopy_initialised) {
			if (confirm(js_translate('page_not_loaded_yet'))) {
				document.edit.action.value='';
				document.edit.submit()
			}
			return;
		}

		var w = (width  != null) ? width  : 500;
		var h = (height != null) ? height : 400;
		var border = 15;
		var top_offset = 20;

		var page_w = (is_nav4 || is_gecko) ? w - 17 : w;
		var page_h = (is_nav4 || is_gecko) ? h - 17 : h;

		file  = backendHref + '&popup_file=' + file;
		file += '&page_width=' + page_w;
		file += '&page_height=' + page_h;
		file += '&body_extra=';
		file += '&browser=' + ((is_dom) ? "dom" : ((is_ie4up) ? "ie" : "ns"));
		file += '&assetid=' + asset_id;

		bodycopy_popup.w = w;
		bodycopy_popup.h = h;
		bodycopy_popup.clip(null, w, h, null);
		bodycopy_popup_visible = true;

		if (is_ie4up) {
			// frameElement is IE 5.5 only
			bodycopy_nested_doc.width  = w - border;
			bodycopy_nested_doc.height = h - border;
			bodycopy_nested_doc.location = file;
		} else if (is_dom) {
			bodycopy_nested_doc.width  = w - border;
			bodycopy_nested_doc.height = h - border;
			bodycopy_nested_doc.src = file;
		} else {
			bodycopy_nested_doc.clip.right     = w - border;
			bodycopy_nested_doc.clip.width     = w - border;
			bodycopy_nested_doc.clip.height    = h - border;
			bodycopy_nested_doc.clip.bottom    = h - border;
			bodycopy_nested_doc.load(file, w - 5);
		}

		// How far down the page we want to display this popup?
		// If we can get the bodycopy divs id, we can figure out exactly where to position the popup so it is 
		// relative to the button the user clicks on instead of where they are scrolled to for better user experience
		// Also, only do this if we are in _admin mode and in IE11 or above, if not, result to old method
		if ((bodycopy_has_class(document.getElementById('sq-content'), 'main')) && (bodycopy_type != undefined && bodycopy_id != undefined)) {
			var id_selector = asset_id + '_' + bodycopy_type + '_' + bodycopy_id;
			var bodycopy_parent_td = document.getElementById('bodycopy_' + id_selector);
			//check if we are dealing with a simple edit layout, the above will return null, change the id to a layout type id instead
			if (bodycopy_parent_td == null || bodycopy_parent_td == undefined) {
				bodycopy_parent_td = document.getElementById('layout_' +  id_selector);
			}
			var xPosition = 0;
    		var yPosition = 0;
		    while(bodycopy_parent_td.className != 'sq-backend-section-table-inner') {
		        xPosition += (bodycopy_parent_td.offsetLeft);
		        yPosition += (bodycopy_parent_td.offsetTop);
		        bodycopy_parent_td = bodycopy_parent_td.offsetParent;
		    }
  			bodycopy_popup.move(20, yPosition + 27);
		} else {
			var scroll_top  = ((is_ie4up) ? (document.documentElement && document.documentElement.scrollTop) || document.body.scrollTop  : self.pageYOffset);
			var top_position = 0;
			if (is_chrome) {
				top_position = scroll_top - 100 + top_offset;
			} else {
				top_position = scroll_top + 50 + top_offset;
			}
			bodycopy_popup.move(null, top_position);
		}
		bodycopy_popup.show();
	}//end bodycopy_show_popup()

	function bodycopy_hide_popup() {
		if (!bodycopy_initialised) { return; }
		bodycopy_show_popup("blank.php");
		bodycopy_popup_visible = false;
		bodycopy_popup.hide();
	}//end bodycopy_hide_popup()

	// get the available cell types
	var BODYCOPY_AVAILABLE_CONTENT_TYPES = null;
	function get_bodycopy_available_content_types() {
		if (BODYCOPY_AVAILABLE_CONTENT_TYPES == null) {
			BODYCOPY_AVAILABLE_CONTENT_TYPES = var_unserialise(bodycopy_types);
		}
		return BODYCOPY_AVAILABLE_CONTENT_TYPES;
	}

	function bodycopy_data_exists(args) {
		var str = 'bodycopy_saved';
		for (var i = 0; i < args.length; i++) {
			switch (typeof(args[i])) {
				case "number" :
					str += '[' + args[i] + ']';
				break;
				default :
					str += '["' + args[i] + '"]';
			}
			eval('var exists = (' + str + ') ? true : false;');
			if (!exists) return false;
		}
		return true;
	}

	function bodycopy_chgColor(id, colour) {
		if (is_dom) {
			var chgcell
			//if (!colour) { colour = '559AE7'; }
			chgcell = "document.getElementById('"+ id + "').className = document.getElementById('"+ id + "').className + ' sq-container-changed' ";
			eval(chgcell);
		}
	}

	/*function bodycopy_insert_container(bodycopy_name, containerid, before) {
		var form = document.main_form;
		var container_type = form_element_value(form._prefix._insert_container_type);
		eval('bodycopy_insert_' + container_type + '("' + bodycopy_name + '", ' + containerid + ', ' + before + ');');
	}*/

	function bodycopy_insert_container(bodycopy_name, containerid, before) {
		var form = document.main_form;
		var container_type;
		eval('container_type=form_element_value(form.' + _prefix + '_insert_container_type);');
		eval('bodycopy_insert_' + container_type + '("' + bodycopy_name + '", ' + containerid + ', ' + before + ');');
	}

	function bodycopy_has_class(element, className) {
		if(element != null && element != undefined) {
    		return element.className && new RegExp("(^|\\s)" + className + "(\\s|$)").test(element.className);
    	} else {
    		return false;
    	}
	}

	bodycopy_current_data[_prefix] = new Object();
