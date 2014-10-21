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
* $Id: edit_table_row_props.php,v 1.10 2012/08/30 01:09:05 ewang Exp $
*
*/

/**
* Table Row Properties Pop-Up
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


<h1 class="title">
	<a href="#" onclick="javascript: popup_close(); return false;">
		<img src="<?php echo sq_web_path('lib')?>/web/images/icons/cancel.png" alt="Cancel" title="<?php echo translate('Cancel');?>" class="sq-icon">
	</a>
	<?php echo translate('Edit Table Row Properties'); ?>
</h1>
<form name="main_form">
<table class="bodycopy-popup-table">
	<tr>
		<td colspan="2">
			<h2><?php echo translate('Properties'); ?></h2>
			<fieldset>
				<table border="0" cellpadding="0" cellspacing="4">
					<tr>
						<td class="label"><?php echo translate('Height'); ?>:</td>
						<td><input type="text" name="height" value="" size="5"></td>
					</tr>
					<tr>
						<td class="label"><?php echo translate('Background colour'); ?>:</td>
						<td><?php colour_box('bgcolor', '', TRUE, 'Colour picker',TRUE, FALSE, FALSE);?></td>
					</tr>
				</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('Text Direction'); ?></h2>
			<fieldset class="last">
				<table border="0" cellpadding="0" cellspacing="4">
					<tr>
						<td class="label"><?php echo translate('Direction:'); ?></td>
						<td>
							<select name="dir">
								<option value=""><?php echo translate('-- Leave Unchanged --'); ?></option>
								<option value="ltr"><?php echo translate('Left to right'); ?></option>
								<option value="rtl"><?php echo translate('Right to left'); ?></option>
							</select>
						</td>
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
			<input type="button" class="sq-btn-blue" name="ok" onClick="javascript: popup_save(this.form)" value="<?php echo translate('Save'); ?>"/>
		</td>
	</tr>
</table>
</form>
<?php include(dirname(__FILE__).'/footer.php'); ?>
