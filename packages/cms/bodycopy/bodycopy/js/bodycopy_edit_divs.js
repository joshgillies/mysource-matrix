/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: bodycopy_edit_divs.js,v 1.2 2003/11/18 15:43:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

function bodycopy_insert_div(bodycopy_name, divid, before) {
	bodycopy_current_edit["bodycopy_name"]          = bodycopy_name;
	bodycopy_current_edit["data"]                   = new Object();
	bodycopy_current_edit["data"]["containerid"]    = divid;
	bodycopy_current_edit["data"]["container_type"] = 'div';
	bodycopy_current_edit["data"]["before"]         = before;
	bodycopy_show_popup("insert_div.php", 300, 220);
}// end bodycopy_insert_div()

function bodycopy_save_insert_div(attributes) {
	bodycopy_current_edit["data"]["attributes"] = attributes;
	bodycopy_hide_popup();
	bodycopy_submit("insert_container", bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"]);
}// end bodycopy_save_insert_div()

function bodycopy_delete_div(bodycopy_name, divid) {
	if (confirm('Are you sure you want to delete this DIV?') && confirm('Really Sure? This is irreversible.')) {
		var data = new Object();
		data["containerid"] = divid;
		bodycopy_submit("delete_container", bodycopy_name, data);
	}
}// end bodycopy_delete_div()

function bodycopy_edit_div_properties(bodycopy_name, divid) {
	bodycopy_current_edit["bodycopy_name"]   = bodycopy_name;
	bodycopy_current_edit["data"]            = new Object();
	bodycopy_current_edit["data"]["divid"] = divid;
	var data = get_bodycopy_current_div_data(bodycopy_name, divid);
	bodycopy_current_edit["data"]["attributes"] = var_unserialise(data["attributes"]);
	bodycopy_show_popup("edit_div_props.php", 320, 330);
}// end bodycopy_edit_div_properties()

function bodycopy_save_div_properties(attributes) {
	bodycopy_current_edit["data"]["attributes"] = attributes;
	bodycopy_hide_popup();
	var id = bodycopy_current_edit["bodycopy_name"] + '_div_' + bodycopy_current_edit["data"]["divid"]; 
	bodycopy_chgColor(id);
	serialise_div(bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"], bodycopy_current_edit["data"]["divid"]);
}// end bodycopy_save_div_properties()