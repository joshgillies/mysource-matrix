<?php
/**
* Table Cell Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

include(dirname(__FILE__)."/header.php");
?> 
<script language="JavaScript">

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
<table width="100%" border="0">
<form name="main_form">
	<tr>
		<td nowrap class="bodycopy-popup-heading">Edit Table Cell Properties&nbsp;</td>
	</tr>
	<tr>
		<td>
			<hr>
		</td>
	</tr>
	<tr>
		<td>
			<table border="0" cellpadding="0" cellspacing="4">
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Width :</td>
					<td valign="middle">
						<input type="text" name="width" value="" size="5">
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Height :</td>
					<td valign="middle">
						<input type="text" name="height" value="" size="5">
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Colspan :</td>
					<td valign="middle">
						<input type="text" name="colspan" value="" size="5">
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Background Colour :</td>
					<td valign="middle">
						<?php colour_box('bgcolor', '', true, '*',true, false, false);?>
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Alignment :</td>
					<td valign="middle">
						<select name="align">
							<option value=""      >
							<option value="left"  >Left
							<option value="center">Centre
							<option value="right" >Right
						</select>
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Vertical Alignment :</td>
					<td valign="middle">
						<select name="valign">
							<option value=""        >
							<option value="middle"  >Middle
							<option value="top"     >Top
							<option value="bottom"  >Bottom
						</select>
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">No Text Wrap :</td>
					<td valign="middle">
						<select name="nowrap">
							<option value="">Off
							<option value="on">On
						</select>
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Cell Type :</td>
					<td valign="middle">
						<select name="type">
							<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
							<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
							<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
						</select>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr>
		<td>
			<hr>
		</td>
	</tr>
	<tr>
		<td align="center">
			<input type="button" value="Save" onclick="javascript: save_props(this.form)">
			<input type="button" value="Cancel" onclick="javascript: popup_close();">
		</td>
	</tr>
</form>
</table>
<?php include(dirname(__FILE__)."/footer.php"); ?> 