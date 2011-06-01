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
* $Id: insert_table.php,v 1.10 2006/12/05 05:34:16 emcdonald Exp $
*
*/

/**
* Insert Table Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
header('Pragma: no-cache');
header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');

include(dirname(__FILE__).'/header.php');
?>
<script type="text/javascript" language="javascript" src="<?php echo sq_web_path('lib')?>/js/general.js"></script>
<script language="JavaScript" type="text/javascript">

	function popup_init() {
		var f = document.main_form;
	}

	function popup_save(f) {
		var data = new Object();
		data["width"]   = owner.form_element_value(f.width);
		data["bgcolor"] = owner.form_element_value(f.bgcolor);
		owner.bodycopy_save_insert_table(owner.form_element_value(f.cols), owner.form_element_value(f.rows), data);
	}

	function set_pos_int(field, input_default) {

		var num = parseInt(owner.form_element_value(field));
		if (isNaN(num) || num < 0) {
			alert(js_translate('enter_positive_number'));
			field.value = input_default;
			field.focus();
		} else {
			field.value = num;
		}
	}

</script>

<div class="title" style="text-align: right;"><?php echo translate('bodycopy_insert_table'); ?></div>

<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="tableid" value="">
<table width="100%" border="0">
	<tr>
		<td>
			<table width="100%" cellspacing="0" cellpadding="0">
				<tr>
					<td valign="top" width="50%">
						<fieldset>
						<legend><b><?php echo translate('layout'); ?></b></legend>
						<table style="width:100%">
							<tr>
								<td class="label"><?php echo translate('columns'); ?>:</td>
								<td><input type="text" name="cols" value="1" size="3" onChange="javascript: set_pos_int(this, 1);"></td>
							</tr>
							<tr>
								<td class="label"><?php echo translate('rows'); ?>:</td>
								<td><input type="text" name="rows" value="1" size="3" onChange="javascript: set_pos_int(this, 1);"></td>
							</tr>
							<tr>
								<td class="label"><?php echo translate('width'); ?>:</td>
								<td><input type="text" name="width" value="" size="5"></td>
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
			<legend><b><?php echo translate('table_styles-colours'); ?></b></legend>
				<table width="100%">
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
		<div style="text-align: right;">
		<button type="button" name="ok" onClick="javascript: popup_save(this.form)"><?php echo translate('ok'); ?></button>
		&nbsp;
		<button type="button" name="cancel" onClick="javascript: popup_close();"><?php echo translate('cancel'); ?></button>
		</div>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__).'/footer.php'); ?>