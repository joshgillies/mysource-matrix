<?php
/**
* Insert Table Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
include(dirname(__FILE__)."/header.php");
?> 
<script language="JavaScript">

	function popup_init() {
		var f = document.main_form;
	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["width"]   = owner.form_element_value(f.width);
		data["bgcolor"] = owner.form_element_value(f.bgcolor);
		owner.bodycopy_save_insert_table(owner.form_element_value(f.cols), owner.form_element_value(f.rows), data);
	}

	function set_pos_int(field, input_default) {

		var num = parseInt(owner.form_element_value(field));
		if (isNaN(num) || num < 0) {
			alert("Please enter a positive number\n");
			field.value = input_default;
			field.focus();
		} else {
			field.value = num;
		}// end if

	}// end set_pos_int()

</script>
<table border="0" width="100%">
<form name="main_form">
	<tr>
		<td nowrap class="bodycopy-popup-heading">Insert Table&nbsp;</td>
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
					<td class="bodycopy-popup-heading"># of Columns:</td>
					<td>
						<input type="text" name="cols" value="1" size="3" onChange="javascript: set_pos_int(this, 1);">
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading"># of Rows:</td>
					<td>
						<input type="text" name="rows" value="1" size="3" onChange="javascript: set_pos_int(this, 1);">
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Width:</td>
					<td>
						<input type="text" name="width" value="" size="4">
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Background Colour:</td>
					<td>
						<?php colour_box('bgcolor', '', true, '*',true, false, false);?>
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
			<input type="button" name="save_button" value="Save" onclick="javascript: popup_save(this.form)">
			<input type="button" value="Cancel" onclick="javascript: popup_close();">
		</td>
	</tr>
</form>
</table>
<?php include(dirname(__FILE__)."/footer.php"); ?>