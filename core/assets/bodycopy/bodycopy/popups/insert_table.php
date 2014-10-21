<?php
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
* $Id: insert_table.php,v 1.11 2012/08/30 01:09:05 ewang Exp $
*
*/

/**
* Insert Table Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
* @deprecated since 5.0
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
			alert(js_translate('Please enter a positive number.'));
			field.value = input_default;
			field.focus();
		} else {
			field.value = num;
		}
	}

</script>

<h1 class="title">
	<a href="#" onclick="javascript: popup_close(); return false;">
		<img src="<?php echo sq_web_path('lib')?>/web/images/icons/cancel.png" alt="Cancel" title="<?php echo translate('Cancel');?>" class="sq-icon">
	</a>
	<?php echo translate('Insert Table'); ?>
</h1>
<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="tableid" value="">
<table>
	<tr>
		<td colspan="2">
			<table>
				<tr>
					<td>
						<h2><?php echo translate('Layout'); ?></h2>
						<fieldset>
						<table>
							<tr>
								<td class="label"><?php echo translate('Columns'); ?>:</td>
								<td><input type="text" name="cols" value="1" size="3" onChange="javascript: set_pos_int(this, 1);"></td>
							</tr>
							<tr>
								<td class="label"><?php echo translate('Rows'); ?>:</td>
								<td><input type="text" name="rows" value="1" size="3" onChange="javascript: set_pos_int(this, 1);"></td>
							</tr>
							<tr>
								<td class="label"><?php echo translate('Width'); ?>:</td>
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
		<td colspan="2">
			<h2><?php echo translate('Table Styles / Colours'); ?></h2>
			<fieldset class="last">
				<table>
					<tr>
						<td class="label"><?php echo translate('Background Colour'); ?>:</td>
						<td><span title="Use colour picker"><?php colour_box('bgcolor', '', TRUE, 'Colour picker',TRUE, FALSE, FALSE, 'class="sq-btn-small"');?><span></td>
					</tr>
				</table>
			</fieldset>
		</td>
	</tr>
	<tr class="sq-popup-footer">
		<td align="left">
			<input type="button" class="" name="cancel" onClick="javascript: popup_close();" value="<?php echo translate('Cancel'); ?>"/>
		</td>
		<td align="right">
			<input type="button" class="sq-btn-blue" name="ok" onClick="javascript: popup_save(this.form)" value="<?php echo translate('Insert'); ?>"/>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__).'/footer.php'); ?>