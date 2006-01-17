<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |

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
* $Id: edit_table_props.php,v 1.10 2006/01/17 04:55:16 lwright Exp $
*
*/

/**
* Table Properties Pop-Up
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

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		var f = document.main_form;

		f.width.value		= (data['width']	  == null) ? "" : data['width'];
		f.height.value		= (data['height']	  == null) ? "" : data['height'];
		f.bgcolor.value		= (data['bgcolor']	  == null) ? "" : data['bgcolor'];
		//f.background.value	= (data['background'] == null) ? "" : data['background'];
		f.identifier.value	= (data['identifier'] == null) ? "" : data['identifier'];
		owner.highlight_combo_value(f.align,	   data['align']);
		owner.highlight_combo_value(f.border,	   data['border']);
		owner.highlight_combo_value(f.cellspacing, data['cellspacing']);
		owner.highlight_combo_value(f.cellpadding, data['cellpadding']);

		f.tableid.value = owner.bodycopy_current_edit["data"]["tableid"];
		f.bodycopy_name.value = owner.bodycopy_current_edit["bodycopy_name"];

		f.disable_keywords.checked = (data["disable_keywords"] == "1");

	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"]		 = owner.form_element_value(f.identifier);
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
		owner.bodycopy_save_table_properties(data);
	}

</script>

<div class="title">
	Table Properties
</div>
<script language="JavaScript">
if (owner.bodycopy_current_edit["can_delete"] == FALSE) { document.getElementById('sq_edit_div_props_delete').innerHTML = '&nbsp;'; }
</script>

<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="tableid" value="">
<table width="100%" border="0">
	<tr>
		<td>
		<fieldset>
			<legend><b><?php echo translate('identification'); ?></b></legend>
			<table style="width:100%">
				<tr>
					<td class="label"><?php echo translate('name'); ?>:</td>
					<td><input type="text" name="identifier" value="" size="15"></td>
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
						<legend><b><?php echo translate('layout'); ?></b></legend>
						<table style="width:100%">
							<tr>
								<td class="label"><?php echo translate('width'); ?>:</td>
								<td><input type="text" name="width" value="" size="5"></td>
							</tr>
							<tr>
								<td class="label"><?php echo translate('height'); ?>:</td>
								<td><input type="text" name="height" value="" size="5"></td>
							</tr>
							<tr>
								<td class="label"><?php echo translate('alignment'); ?>:</td>
								<td>
								<select name="align">
									<option value="">
									<option value="left"  ><?php echo translate('left'); ?>
									<option value="center"><?php echo translate('centre'); ?>
									<option value="right" ><?php echo translate('right'); ?>
								</select>
								</td>
							</tr>
						</table>
						</fieldset>
					</td>
					<td>&nbsp;</td>
					<td valign="top" width="50%">
						<fieldset>
						<legend><b><?php echo translate('spacing_and_padding'); ?></b></legend>
						<table style="width:100%">
							<tr>
								<td class="label"><?php echo translate('spacing'); ?>:</td>
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
								<td class="label"><?php echo translate('padding'); ?>:</td>
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
			<legend><b><?php echo translate('table_styles-colours'); ?></b></legend>
				<table>
					<tr>
						<td class="label"><?php echo translate('background_colour'); ?>:</td>
						<td><?php colour_box('bgcolor', '', TRUE, '*',TRUE, FALSE, FALSE);?></td>
					</tr>
					<!-- <tr>
						<td class="label">Background Image:</td>
						<td><input type="text" name="background" value="" size="5"></td>
					</tr> -->
					<tr>
						<td class="label" valign="top"><?php echo translate('border'); ?>:</td>
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
		<td width="100%">
			<fieldset>
			<legend><b><?php echo translate('keywords'); ?></b></legend>
				<table>
					<tr>
						<td class="label"><?php echo translate('disable_keywords'); ?>:</td>
						<td><input type="checkbox" name="disable_keywords" value="1"></td>
					</tr>
				</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td>
			<fieldset>
			<legend><b><?php echo translate('delete_this_table'); ?></b></legend>
			<table>
				<tr>
					<td class="label"><?php echo translate('click_to_delete'); ?>:</td>
					<td>
						<a href="javascript: owner.bodycopy_delete_table(document.main_form.bodycopy_name.value, document.main_form.tableid.value);" style="cursor: pointer;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/icons/delete.png", "16", "16", "Delete this table");</script></a>
					</td>
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