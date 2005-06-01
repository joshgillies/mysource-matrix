<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |

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
* $Id: edit_div_props.php,v 1.11.2.1 2005/06/01 06:12:27 rhoward Exp $
* $Name: not supported by cvs2svn $
*/

/**
* DIV Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

include(dirname(__FILE__)."/header.php");
?>
<script type="text/javascript" src="<?php echo sq_web_path('lib')?>/js/general.js"></script>
<script type="text/javascript">

	function popup_init() {

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		available_types = owner.bodycopy_current_edit["data"]["available_types"];
		var f = document.main_form;

		f.identifier.value = (data['identifier'] == null) ? "" : data['identifier'];
		f.css_class.value  = (data['css_class'] == null)  ? "" : data['css_class'];

		f.divid.value = owner.bodycopy_current_edit["data"]["divid"];
		f.bodycopy_name.value = owner.bodycopy_current_edit["bodycopy_name"];
		owner.highlight_combo_value(f.layout_type, data['layout_type']);

		// remove the existing values
		for(var i = f.content_type.options.length - 1; i >= 0; i--) {
			f.content_type.options[i] = null;
		}
		var i = 0;

		// add default "Leave Unchanged" value to the top
		f.content_type.options[i] = new Option("-- Leave Unchanged --", "");
		i++;

		for(var key in available_types) {
			if (available_types[key] == null) continue;
			if(available_types[key]["name"] != null) {
				f.content_type.options[i] = new Option(available_types[key]["name"], key);
				i++;
			}
		}

		if (typeof data["content_type"] == 'undefined' && typeof available_types['content_type_wysiwyg'] != 'undefined') {
			data["content_type"] = 'content_type_raw_html';
		}

		owner.highlight_combo_value(f.content_type, "");
		f.disable_keywords.checked = (data["disable_keywords"] == "1");

	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"]       = owner.form_element_value(f.identifier);
		data["css_class"]        = owner.form_element_value(f.css_class);
		data["layout_type"]      = owner.form_element_value(f.layout_type);
		data["content_type"]     = owner.form_element_value(f.content_type);
		data["disable_keywords"] = owner.form_element_value(f.disable_keywords);
		owner.bodycopy_save_div_properties(data);
	}

</script>

<div class="title">
	<table border="0" cellspacing="0" cellpadding="0"  width="100%">
		<tr>
			<td class="title" align="left">DIV Properties</td>
		</tr>
	</table>
</div>
<script language="JavaScript">
if (owner.bodycopy_current_edit["can_delete"] == false) { document.getElementById('sq_edit_div_props_delete').innerHTML = '&nbsp;'; }
</script>
<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="divid" value="">
<table width="100%" border="0">
	<tr>
		<td>
		<fieldset>
			<legend><b>Identification</b></legend>
			<table style="width:100%">
				<tr>
					<td class="label">Name:</td>
					<td><input type="text" name="identifier" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		<fieldset>
			<legend><b>Style Information</b></legend>
			<table style="width:100%">
				<tr>
					<td class="bodycopy-popup-heading">Presentation:</td>
					<td>
						<select name="layout_type">
							<option value="div" >Block-level</option>
							<option value="span">Inline</option>
							<option value="none">Raw HTML</option>
						</select>
					</td>
				</tr>
				<tr>
					<td class="label">Class:</td>
					<td><input type="text" name="css_class" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		</td>
	</tr>
	<tr>
		<td>
			<fieldset>
			<legend><b>Content Type</b></legend>
			<table style="width:100%">
				<tr>
					<td class="label">Content Type:</td>
					<td>
					<select name="content_type">
						<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
						<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
						<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
					</select>
					</td>
				</tr>
				<tr>
					<td class="label">Disable Keywords:</td>
					<td><input type="checkbox" name="disable_keywords" value="1"></td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td>
			<fieldset>
			<legend><b>Delete This Div</b></legend>
			<table>
				<tr>
					<td class="label">Click Icon to Delete:</td>
					<td><a href="javascript: owner.bodycopy_delete_div(document.main_form.bodycopy_name.value, document.main_form.divid.value);" style="cursor: pointer;" ><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/icons/delete.png", "16", "16", "Delete this DIV");</script></a>
					</td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td>
		<div style="text-align: center;">
		<button type="button" name="ok" onClick="javascript: popup_save(this.form)">OK</button>
		&nbsp;
		<button type="button" name="cancel" onClick="javascript: popup_close();">Cancel</button>
		</div>
		</td>
	</tr>
</table>
</form>
<?php include(dirname(__FILE__)."/footer.php"); ?>