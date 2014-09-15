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
* $Id: edit_table_cell_props.php,v 1.10 2012/08/30 01:09:05 ewang Exp $
*
*/

/**
* Table Cell Properties Pop-Up
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
header('Expires: '.gmdate('D, d M Y H:i:s',time()-3600).' GMT');

include(dirname(__FILE__).'/header.php');
?>
<script language="JavaScript" type="text/javascript">

	function popup_init() {

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		available_types = owner.bodycopy_current_edit["data"]["available_types"];

		var f = document.main_form;
		f.width.value	= (data['width']  == null) ? "" : data['width'];
		f.height.value  = (data['height'] == null) ? "" : data['height'];
		f.colspan.value = (data['colspan'] == null) ? "" : data['colspan'];
		f.dir.value 	= (data['dir'] == null) ? "" : data['dir'];
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
		data["width"]	 = owner.form_element_value(f.width);
		data["height"]   = owner.form_element_value(f.height);
		data["colspan"]  = owner.form_element_value(f.colspan);
		data["dir"]		 = owner.form_element_value(f.dir);
		data["bgcolor"]  = owner.form_element_value(f.bgcolor);
		data["align"]    = owner.form_element_value(f.align);
		data["valign"]   = owner.form_element_value(f.valign);
		data["nowrap"]   = owner.form_element_value(f.nowrap);
		data["type"] 	 = owner.form_element_value(f.type);
		owner.bodycopy_save_table_cell_properties(data);
	}
</script>


<h1 class="title">
	<a href="#" onclick="javascript: popup_close(); return false;">
		<img src="<?php echo sq_web_path('lib')?>/web/images/icons/cancel.png" alt="Cancel" title="<?php echo translate('cancel');?>" class="sq-icon">
	</a>
	Table Cell Properties
</h1>
<form name="main_form">
<table>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('layout'); ?></h2>
			<fieldset>
				<table>
					<tr>
						<td class="label"><?php echo translate('width'); ?>:</td>
						<td><input type="text" name="width" value="" size="5"></td>
					</tr>
					<tr>
						<td class="label"><?php echo translate('height'); ?>:</td>
						<td><input type="text" name="height" value="" size="5"></td>
					</tr>
					<tr>
						<td class="label"><?php echo translate('colspan'); ?>:</td>
						<td><input type="text" name="colspan" value="" size="5"></td>
					</tr>
				</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('alignment'); ?></h2>
			<fieldset>
				<table>
					<tr>
						<td class="label"><?php echo translate('horizontal'); ?>:</td>
						<td>
						<select name="align">
							<option value="">
							<option value="left"  ><?php echo translate('left'); ?>
							<option value="center"><?php echo translate('centre'); ?>
							<option value="right" ><?php echo translate('right');?>
						</select>
						</td>
					</tr>
					<tr>
						<td class="label"><?php echo translate('vertical'); ?>:</td>
						<td>
						<select name="valign">
							<option value="">
							<option value="middle"  ><?php echo translate('middle'); ?>
							<option value="top"     ><?php echo translate('top'); ?>
							<option value="bottom"  ><?php echo translate('bottom'); ?>
						</select>
						</td>
					</tr>
				</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('text_direction'); ?></h2>
			<fieldset>
				<table style="width:100%">
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
		<td colspan="2">
			<h2><?php echo translate('cell_styles-colours'); ?></h2>
			<fieldset>
			<table style="width:100%">
				<tr>
					<td class="label"><?php echo translate('background_colour'); ?>:</td>
					<td><?php colour_box('bgcolor', '', TRUE, 'Colour picker',TRUE, FALSE, FALSE);?></td>
				</tr>
				<tr>
					<td class="label"><?php echo translate('no_text_wrap'); ?>:</td>
					<td>
					<select name="nowrap">
						<option value=""><?php echo translate('off'); ?>
						<option value="on"><?php echo translate('on'); ?>
					</select>
					</td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('cell_type'); ?></h2>
			<fieldset class="last">
			<table style="width:100%">
				<tr>
					<td class="label"><?php echo translate('cell_type'); ?>:</td>
					<td>
					<select name="type">
						<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
						<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
						<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
					</select>
					</td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr class="sq-popup-footer">
		<td align="left">
			<input type="button" class="" name="cancel" onClick="javascript: popup_close();" value="<?php echo translate('cancel'); ?>"/>
		</td>
		<td align="right">
			<input type="button" class="sq-btn-blue" name="ok" onClick="javascript: popup_save(this.form)" value="<?php echo translate('save'); ?>"/>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__).'/footer.php'); ?>