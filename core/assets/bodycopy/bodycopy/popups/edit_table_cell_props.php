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
* $Id: edit_table_cell_props.php,v 1.2.2.1 2004/02/18 11:39:04 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Table Cell Properties Pop-Up
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
<script language="JavaScript" type="text/javascript">

	function popup_init() {

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		available_types = owner.bodycopy_current_edit["data"]["available_types"];

		var f = document.main_form;
		f.width.value   = (data['width']  == null) ? "" : data['width'];
		f.height.value  = (data['height'] == null) ? "" : data['height'];
		f.colspan.value = (data['colspan'] == null) ? "" : data['colspan'];
		f.bgcolor.value = (data['bgcolor'] == null) ? "" : data['bgcolor'];

		owner.highlight_combo_value(f.align,  data['align']);
		owner.highlight_combo_value(f.valign, data['valign']);
		owner.highlight_combo_value(f.nowrap, data['nowrap']);

		// remove the existing values
		for(var i = f.type.options.length - 1; i >= 0; i--) {
			f.type.options[i] = null;
		}
		var i = 0;
		for(var key in available_types) {
			if (available_types[key] == null) continue;
			if(available_types[key]["name"] != null) {
				f.type.options[i] = new Option(available_types[key]["name"], key);
				i++;
			}
		}

		owner.highlight_combo_value(f.type, data["content_type"]);

	}// end popup_init()
	
	function save_props(f) {

		var data = new Object();
		data["width"]    = owner.form_element_value(f.width);
		data["height"]   = owner.form_element_value(f.height);
		data["colspan"]  = owner.form_element_value(f.colspan);
		data["bgcolor"]  = owner.form_element_value(f.bgcolor);
		data["align"]    = owner.form_element_value(f.align);
		data["valign"]   = owner.form_element_value(f.valign);
		data["nowrap"]   = owner.form_element_value(f.nowrap);
		data["type"]     = owner.form_element_value(f.type);
		owner.bodycopy_save_table_cell_properties(data);
	}
</script>

<div class="title">Table Cell Properties</div>

<form name="main_form">
<table width="100%" border="0">
	<tr>
		<td>
			<table width="100%" cellspacing="0" cellpadding="0">
				<tr>
					<td valign="top" width="50%">
						<fieldset>
						<legend><b>Layout</b></legend>
						<table style="width:100%">
							<tr>
								<td class="label">Width:</td>
								<td><input type="text" name="width" value="" size="5"></td>
							</tr>
							<tr>
								<td class="label">Height:</td>
								<td><input type="text" name="height" value="" size="5"></td>
							</tr>
							<tr>
								<td class="label">Colspan:</td>
								<td><input type="text" name="colspan" value="" size="5"></td>
							</tr>
						</table>
						</fieldset>
					</td>
					<td>&nbsp;</td>
					<td valign="top" width="50%">
						<fieldset>
						<legend><b>Alignment</b></legend>
						<table style="width:100%">
							<tr>
								<td class="label">Horizontal:</td>
								<td>
								<select name="align">
									<option value=""      >
									<option value="left"  >Left
									<option value="center">Centre
									<option value="right" >Right
								</select>
								</td>
							</tr>
							<tr>
								<td class="label">Vertical:</td>
								<td>
								<select name="valign">
									<option value=""        >
									<option value="middle"  >Middle
									<option value="top"     >Top
									<option value="bottom"  >Bottom
								</select>
								</td>
							</tr>
						</table>
						</fieldset>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr>
		<td>
			<fieldset>
			<legend><b>Cell Styles / Colours</b></legend>
			<table style="width:100%">
				<tr>
					<td class="label">Background Colour:</td>
					<td><?php colour_box('bgcolor', '', true, '*',true, false, false);?></td>
				</tr>
				<tr>
					<td class="label">No Text Wrap:</td>
					<td>
					<select name="nowrap">
						<option value="">Off
						<option value="on">On
					</select>
					</td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td>
			<fieldset>
			<legend><b>Cell Type</b></legend>
			<table style="width:100%">
				<tr>
					<td class="label">Cell Type:</td>
					<td>
					<select name="type">
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
			<button type="button" name="ok" onClick="javascript: save_props(this.form)">OK</button>
			&nbsp;
			<button type="button" name="cancel" onClick="javascript: popup_close();">Cancel</button>
			</div>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__)."/footer.php"); ?> 