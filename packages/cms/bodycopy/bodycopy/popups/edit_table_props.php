<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: edit_table_props.php,v 1.10 2003/10/03 00:03:26 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Table Properties Pop-Up
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
<script language="JavaScript" type="text/javascript">

	function popup_init() {

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		var f = document.main_form;

		f.width.value		= (data['width']	  == null) ? "" : data['width'];
		f.height.value		= (data['height']	  == null) ? "" : data['height'];
		f.bgcolor.value		= (data['bgcolor']	  == null) ? "" : data['bgcolor'];
		f.background.value	= (data['background'] == null) ? "" : data['background'];
		f.table_name.value	= (data['table_name'] == null) ? "" : data['table_name'];
		owner.highlight_combo_value(f.align,	   data['align']);
		owner.highlight_combo_value(f.border,	   data['border']);
		owner.highlight_combo_value(f.cellspacing, data['cellspacing']);
		owner.highlight_combo_value(f.cellpadding, data['cellpadding']);

		f.tableid.value = owner.bodycopy_current_edit["data"]["tableid"];
		f.bodycopy_name.value = owner.bodycopy_current_edit["bodycopy_name"];

	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["table_name"]		= owner.form_element_value(f.table_name);
		data["width"]			= owner.form_element_value(f.width);
		data["height"]			= owner.form_element_value(f.height);
		data["bgcolor"]			= owner.form_element_value(f.bgcolor);
		data["background"]		= owner.form_element_value(f.background);
		data["align"]			= owner.form_element_value(f.align);
		data["border"]			= owner.form_element_value(f.border);
		data["cellspacing"]		= owner.form_element_value(f.cellspacing);
		data["cellpadding"]		= owner.form_element_value(f.cellpadding);
		owner.bodycopy_save_table_properties(data);
	}

</script>

<div class="title">
	<table border="0" cellspacing="0" cellpadding="0">
		<tr>
			<td><a href="javascript: owner.bodycopy_delete_table(document.main_form.bodycopy_name.value, document.main_form.tableid.value);"><img src="<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/icons/delete.png" width="16" height="16" border="0"></a></td>
			<td class="title" width="100%" align="right">Table Properties</td>
		</tr>
	</table>
</div>

<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="tableid" value="">
<table width="100%" border="0">
	<tr>
		<td>
		<fieldset>
			<legend><b>Identification</b></legend>
			<table style="width:100%">
				<tr>
					<td class="label">Name:</td>
					<td><input type="text" name="table_name" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		</td>
	</tr>
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
								<td class="label">Alignment:</td>
								<td>
								<select name="align">
									<option value=""      >
									<option value="left"  >Left
									<option value="center">Centre
									<option value="right" >Right
								</select>
								</td>
							</tr>
						</table>
						</fieldset>
					</td>
					<td>&nbsp;</td>
					<td valign="top" width="50%">
						<fieldset>
						<legend><b>Spacing and Padding</b></legend>
						<table style="width:100%">
							<tr>
								<td class="label">Spacing:</td>
								<td>
								<select name="cellspacing">
									<option value="" >
									<option value="0">0
									<option value="1">1
									<option value="2">2
									<option value="3">3
									<option value="4">4
									<option value="5">5
									<option value="6">6
									<option value="7">7
									<option value="8">8
									<option value="9">9
									<option value="10">10
								</select>
								</td>
							</tr>
							<tr>
								<td class="label">Padding:</td>
								<td>
								<select name="cellpadding">
									<option value="" >
									<option value="0">0
									<option value="1">1
									<option value="2">2
									<option value="3">3
									<option value="4">4
									<option value="5">5
									<option value="6">6
									<option value="7">7
									<option value="8">8
									<option value="9">9
									<option value="10">10
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
		<td width="100%">
			<fieldset>
			<legend><b>Table Styles / Colours</b></legend>
				<table width="100%">
					<tr>
						<td class="label">Background Colour:</td>
						<td><?php colour_box('bgcolor', '', true, '*',true, false, false);?></td>
					</tr>
					<tr>
						<td class="label">Background Image:</td>
						<td><input type="text" name="background" value="" size="5"></td>
					</tr>
					<tr>
						<td class="label" valign="top">Border:</td>
						<td valign="top">
						<select name="border">
							<option value="" >
							<option value="0">0
							<option value="1">1
							<option value="2">2
							<option value="3">3
							<option value="4">4
							<option value="5">5
							<option value="6">6
							<option value="7">7
							<option value="8">8
							<option value="9">9
							<option value="10">10
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