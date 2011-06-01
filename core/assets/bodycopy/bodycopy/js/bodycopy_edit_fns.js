/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: bodycopy_edit_fns.js,v 1.7 2006/12/05 05:34:15 emcdonald Exp $
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

	function bodycopy_show_popup(file, width, height) {
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

		// how far down the page we want to display this popup
		var scroll_top  = ((is_ie4up) ? document.body.scrollTop  : self.pageYOffset);
		bodycopy_popup.move(null, scroll_top + top_offset);
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
			if (!colour) { colour = '559AE7'; }
			chgcell = "document.getElementById('"+ id + "').style.backgroundColor = '#"+ colour +"'";
			eval(chgcell);
		}
	}

	function bodycopy_insert_container(bodycopy_name, containerid, before) {
		var form = document.main_form;
		var container_type = form_element_value(form._prefix._insert_container_type);
		eval('bodycopy_insert_' + container_type + '("' + bodycopy_name + '", ' + containerid + ', ' + before + ');');
	}

	function bodycopy_insert_container(bodycopy_name, containerid, before) {
				var form = document.main_form;
				var container_type;
				eval('container_type=form_element_value(form.' + _prefix + '_insert_container_type);');
				eval('bodycopy_insert_' + container_type + '("' + bodycopy_name + '", ' + containerid + ', ' + before + ');');
			}

	bodycopy_current_data[_prefix] = new Object();
