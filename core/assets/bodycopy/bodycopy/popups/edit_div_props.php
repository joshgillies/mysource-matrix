<?php
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
* $Id: edit_div_props.php,v 1.3.2.1 2004/02/18 11:39:04 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* DIV Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

include(dirname(__FILE__)."/header.php");
?>
<script type="text/javascript" language="javascript" src="<?php echo sq_web_path('lib')?>/js/general.js"></script>
<script language="JavaScript" type="text/javascript">

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
		for(var key in available_types) {
			if (available_types[key] == null) continue;
			if(available_types[key]["name"] != null) {
				f.content_type.options[i] = new Option(available_types[key]["name"], key);
				i++;
			}
		}
		owner.highlight_combo_value(f.content_type, data["content_type"]);

	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"]   = owner.form_element_value(f.identifier);
		data["css_class"]    = owner.form_element_value(f.css_class);
		data["layout_type"]  = owner.form_element_value(f.layout_type);
		data["content_type"] = owner.form_element_value(f.content_type);
		owner.bodycopy_save_div_properties(data);
	}

</script>

<div class="title">
	<table border="0" cellspacing="0" cellpadding="0">
		<tr>
			<td><a href="javascript: owner.bodycopy_delete_div(document.main_form.bodycopy_name.value, document.main_form.divid.value);" style="cursor: pointer;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/icons/delete.png", "16", "16", "Delete this DIV");</script></a></td>
			<td class="title" width="100%" align="right">DIV Properties</td>
		</tr>
	</table>
</div>

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
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td>
		<div style="text-align: right;">
		<button type="button" name="ok" onClick="javascript: popup_save(this.form)">OK</button>
		&nbsp;
		<button type="button" name="cancel" onClick="javascript: popup_close();">Cancel</button>
		</div>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__)."/footer.php"); ?> 