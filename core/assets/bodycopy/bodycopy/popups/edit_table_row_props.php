<?php
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
* $Id: edit_table_row_props.php,v 1.8.12.1 2009/05/15 07:17:11 wszymanski Exp $
*
*/

/**
* Table Row Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8.12.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
header('Pragma: no-cache');
header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');

include(dirname(__FILE__).'/header.php');
?>
<script language="JavaScript" type="text/javascript">

	function popup_init() {

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		var f = document.main_form;

		f.height.value  = (data['height'] == null)  ? "" : data['height'];
		f.bgcolor.value = (data['bgcolor'] == null) ? "" : data['bgcolor'];
		f.dir.value = (data['dir'] == null) ? "" : data['dir'];

	}// end popup_init()

	function save_props(f) {
		var data = new Object();
		data["height"]  = owner.form_element_value(f.height);
		data["bgcolor"] = owner.form_element_value(f.bgcolor);
		data["dir"] = owner.form_element_value(f.dir);
		owner.bodycopy_save_table_row_properties(data);
	}
</script>

<div class="title"><?php echo translate('table_row_properties'); ?></div>

<form name="main_form">
<table width="100%" border="0" class="bodycopy-popup-table">
	<tr>
		<td>
			<fieldset>
			<legend><?php echo translate('properties'); ?></legend>
			<table border="0" cellpadding="0" cellspacing="4">
				<tr>
					<td class="label"><?php echo translate('height'); ?>:</td>
					<td><input type="text" name="height" value="" size="5"></td>
				</tr>
				<tr>
					<td class="label"><?php echo translate('background_colour'); ?>:</td>
					<td><?php colour_box('bgcolor', '', TRUE, '*',TRUE, FALSE, FALSE);?></td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td>
			<fieldset>
			<legend><?php echo translate('text_direction'); ?></legend>
			<table border="0" cellpadding="0" cellspacing="4">
				<tr>
					<td class="label"><?php echo translate('bodycopy_direction'); ?></td>
					<td>
						<select name="dir">
							<option value=""><?php echo translate('content_type_no_change'); ?></option>
							<option value="ltr"><?php echo translate('bodycopy_left_to_right'); ?></option>
							<option value="rtl"><?php echo translate('bodycopy_right_to_left'); ?></option>
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
			<button type="button" name="ok" onClick="javascript: save_props(this.form)"><?php echo translate('ok'); ?></button>
			&nbsp;
			<button type="button" name="cancel" onClick="javascript: popup_close();"><?php echo translate('cancel'); ?></button>
			</div>
		</td>
	</tr>
</table>
</form>
<?php include(dirname(__FILE__).'/footer.php'); ?>
