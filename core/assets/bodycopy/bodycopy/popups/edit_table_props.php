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
* $Id: edit_table_props.php,v 1.15 2012/08/30 01:09:05 ewang Exp $
*
*/

/**
* Table Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
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

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		var f = document.main_form;

		f.width.value		= (data['width']	  == null) ? "" : data['width'];
		f.height.value		= (data['height']	  == null) ? "" : data['height'];
		f.bgcolor.value		= (data['bgcolor']	  == null) ? "" : data['bgcolor'];
		//f.background.value	= (data['background'] == null) ? "" : data['background'];
		f.identifier.value	= (data['identifier'] == null) ? "" : data['identifier'];
		f.desc.value		= (data['desc']       == null) ? "" : data['desc'];
		owner.highlight_combo_value(f.align,	   data['align']);
		owner.highlight_combo_value(f.border,	   data['border']);
		owner.highlight_combo_value(f.cellspacing, data['cellspacing']);
		owner.highlight_combo_value(f.cellpadding, data['cellpadding']);

		f.tableid.value = owner.bodycopy_current_edit["data"]["tableid"];
		f.bodycopy_name.value = owner.bodycopy_current_edit["bodycopy_name"];

		f.disable_keywords.checked = (data["disable_keywords"] == "1");
		f.dir.value  = (data['dir']  == null) ? "" : data['dir'];

	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"]		 = owner.form_element_value(f.identifier);
		data["desc"]			 = owner.form_element_value(f.desc);
		data["width"]			 = owner.form_element_value(f.width);
		data["height"]			 = owner.form_element_value(f.height);
		data["bgcolor"]			 = owner.form_element_value(f.bgcolor);
		//data["background"]		= owner.form_element_value(f.background);
		data["background"]		 = '';
		data["align"]			 = owner.form_element_value(f.align);
		data["border"]			 = owner.form_element_value(f.border);
		data["cellspacing"]		 = owner.form_element_value(f.cellspacing);
		data["cellpadding"]		 = owner.form_element_value(f.cellpadding);
		data["disable_keywords"] = owner.form_element_value(f.disable_keywords);
		data["dir"] = owner.form_element_value(f.dir);
		owner.bodycopy_save_table_properties(data);
	}

</script>


<h1 class="title">
	<a href="#" onclick="javascript: popup_close(); return false;">
		<img src="<?php echo sq_web_path('lib')?>/web/images/icons/cancel.png" alt="Cancel" title="<?php echo translate('cancel');?>" class="sq-icon">
	</a>
	<?php echo translate('Edit Table Properties'); ?>
</h1>
<script language="JavaScript">
if (owner.bodycopy_current_edit["can_delete"] == false) { document.getElementById('sq_edit_div_props_delete').innerHTML = '&nbsp;'; }
</script>

<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="tableid" value="">
<table>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('Identification'); ?></h2>
			<fieldset>
				<table>
					<tr>
						<td class="label"><?php echo translate('Name'); ?>:</td>
						<td><input type="text" name="identifier" value="" size="20"></td>
					</tr>
					<tr>
						<td class="label"><?php echo translate('Description'); ?>:</td>
						<td><textarea name="desc" rows="3" size="20" value=""></textarea></td>
					</tr>
				</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('Layout'); ?></h2>
			<fieldset>
				<table>
					<tr>
						<td class="label"><?php echo translate('Width'); ?>:</td>
						<td><input type="text" name="width" value="" size="5"></td>
					</tr>
					<tr>
						<td class="label"><?php echo translate('Height'); ?>:</td>
						<td><input type="text" name="height" value="" size="5"></td>
					</tr>
					<tr>
						<td class="label"><?php echo translate('Alignment'); ?>:</td>
						<td>
						<select name="align">
							<option value="">
							<option value="left"  ><?php echo translate('Left'); ?>
							<option value="center"><?php echo translate('Centre'); ?>
							<option value="right" ><?php echo translate('Right'); ?>
						</select>
						</td>
					</tr>
				</table>
			</fieldset>
			<h2><?php echo translate('Spacing and Padding'); ?></h2>
			<fieldset>
				<table>
					<tr>
						<td class="label"><?php echo translate('Spacing'); ?>:</td>
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
						<td class="label"><?php echo translate('Padding'); ?>:</td>
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
	<tr>
		<td colspan="2">
			<h2><?php echo translate('Text Direction'); ?></h2>
			<fieldset>
				<table style="width:100%">
					<tr>
						<td class="bodycopy-popup-heading"><?php echo translate('Direction:'); ?></td>
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
	<tr>
		<td colspan="2">
			<h2><?php echo translate('Table Styles / Colours'); ?></h2>
			<fieldset>
				<table>
					<tr>
						<td class="label"><?php echo translate('Background Colour'); ?>:</td>
						<td><?php colour_box('bgcolor', '', TRUE, 'Colour picker',TRUE, FALSE, FALSE);?></td>
					</tr>
					<!-- <tr>
						<td class="label">Background Image:</td>
						<td><input type="text" name="background" value="" size="5"></td>
					</tr> -->
					<tr>
						<td class="label" valign="top"><?php echo translate('Border'); ?>:</td>
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
		<td colspan="2">
			<h2><?php echo translate('Keywords'); ?></h2>
			<fieldset>
				<table>
					<tr>
						<td class="label"><?php echo translate('Disable keywords'); ?>:</td>
						<td>
							<input type="checkbox" id="disable_keywords" name="disable_keywords" value="1">
							<label for="disable_keywords"><?php echo translate('Yes') ?></label>
						</td>
					</tr>
				</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('Delete This Table'); ?></h2>
			<fieldset class="last">
			<table>
				<tr>
					<td class="label"><?php echo translate('Click Icon to Delete'); ?>:</td>
					<td>
						<a href="javascript: owner.bodycopy_delete_table(document.main_form.bodycopy_name.value, document.main_form.tableid.value);" style="cursor: pointer;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/icons/delete.png", "16", "16", "Delete this table");</script></a>
					</td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
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
