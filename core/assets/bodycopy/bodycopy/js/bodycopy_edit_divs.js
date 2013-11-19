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
* $Id: bodycopy_edit_divs.js,v 1.14 2012/08/30 01:09:05 ewang Exp $
*
*/

function bodycopy_insert_div(bodycopy_name, divid, before) {
	bodycopy_current_edit["bodycopy_name"]          = bodycopy_name;
	bodycopy_current_edit["data"]                   = new Object();
	bodycopy_current_edit["data"]["containerid"]    = divid;
	bodycopy_current_edit["data"]["container_type"] = 'div';
	bodycopy_current_edit["data"]["before"]         = before;
	bodycopy_show_popup("insert_div.php", 415, 375);
}// end bodycopy_insert_div()

function bodycopy_save_insert_div(attributes) {
	bodycopy_current_edit["data"]["attributes"] = attributes;
	bodycopy_hide_popup();
	bodycopy_submit("insert_container", bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"]);
}// end bodycopy_save_insert_div()

function bodycopy_delete_div(bodycopy_name, divid) {
	if (confirm('Are you sure you want to delete this DIV?')) {
		var data = new Object();
		data["containerid"] = divid;
		bodycopy_submit("delete_container", bodycopy_name, data);
	}
}// end bodycopy_delete_div()

function bodycopy_edit_div_properties(bodycopy_name, divid, can_delete) {
	bodycopy_current_edit["bodycopy_name"]   = bodycopy_name;
	bodycopy_current_edit["can_delete"]   = can_delete;
	bodycopy_current_edit["data"]            = new Object();
	bodycopy_current_edit["data"]["divid"] = divid;
	bodycopy_current_edit["data"]["available_types"] = get_bodycopy_available_content_types();
	bodycopy_current_edit["data"]["available_classes"] = get_bodycopy_div_available_classes();
	var data = get_bodycopy_current_div_data(bodycopy_name, divid);
	if (data != null) {
		bodycopy_current_edit["data"]["attributes"] = var_unserialise(data["attributes"]);
	}
	bodycopy_show_popup("edit_div_props.php", 415, 635);
}// end bodycopy_edit_div_properties()

function bodycopy_save_div_properties(attributes) {
	bodycopy_current_edit["data"]["attributes"] = attributes;
	bodycopy_hide_popup();
	var id = bodycopy_current_edit["bodycopy_name"] + '_div_' + bodycopy_current_edit["data"]["divid"];
	bodycopy_chgColor(id);
	serialise_div(bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"], bodycopy_current_edit["data"]["divid"]);
}// end bodycopy_save_div_properties()