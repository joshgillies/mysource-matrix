<?php
/**
* Table Row Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
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
		var f = document.main_form;

		f.height.value  = (data['height'] == null)  ? "" : data['height'];
		f.bgcolor.value = (data['bgcolor'] == null) ? "" : data['bgcolor'];

	}// end popup_init()

	function save_props(f) {
		var data = new Object();
		data["height"]  = owner.form_element_value(f.height);
		data["bgcolor"] = owner.form_element_value(f.bgcolor);
		owner.bodycopy_save_table_row_properties(data);
	}
</script>

<div class="title">Table Row Properties</div>

<form name="main_form">
<table width="100%" border="0" class="bodycopy-popup-table">
	<tr>
		<td>
			<fieldset>
			<legend>Properties</legend>
			<table border="0" cellpadding="0" cellspacing="4">
				<tr>
					<td class="label">Height :</td>
					<td><input type="text" name="height" value="" size="5"></td>
				</tr>
				<tr>
					<td class="label">Background Colour :</td>
					<td><?php colour_box('bgcolor', '', true, '*',true, false, false);?></td>
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
