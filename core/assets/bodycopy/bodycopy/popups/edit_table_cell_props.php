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
	<?php echo translate('Edit Table Cell Properties'); ?>
</h1>
<form name="main_form">
<table>
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
						<td class="label"><?php echo translate('Colspan'); ?>:</td>
						<td><input type="text" name="colspan" value="" size="5"></td>
					</tr>
				</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('Alignment'); ?></h2>
			<fieldset>
				<table>
					<tr>
						<td class="label"><?php echo translate('Horizontal'); ?>:</td>
						<td>
						<select name="align">
							<option value="">
							<option value="left"  ><?php echo translate('Left'); ?>
							<option value="center"><?php echo translate('Centre'); ?>
							<option value="right" ><?php echo translate('Right');?>
						</select>
						</td>
					</tr>
					<tr>
						<td class="label"><?php echo translate('Vertical'); ?>:</td>
						<td>
						<select name="valign">
							<option value="">
							<option value="middle"  ><?php echo translate('Middle'); ?>
							<option value="top"     ><?php echo translate('Top'); ?>
							<option value="bottom"  ><?php echo translate('Bottom'); ?>
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
						<td class="label"><?php echo translate('Direction'); ?></td>
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
			<h2><?php echo translate('Cell Styles / Colours'); ?></h2>
			<fieldset>
			<table style="width:100%">
				<tr>
					<td class="label"><?php echo translate('Background Colour'); ?>:</td>
					<td><?php colour_box('bgcolor', '', TRUE, 'Colour picker',TRUE, FALSE, FALSE);?></td>
				</tr>
				<tr>
					<td class="label"><?php echo translate('No Text Wrap'); ?>:</td>
					<td>
					<select name="nowrap">
						<option value=""><?php echo translate('Off'); ?>
						<option value="on"><?php echo translate('On'); ?>
					</select>
					</td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('Cell Type'); ?></h2>
			<fieldset class="last">
			<table style="width:100%">
				<tr>
					<td class="label"><?php echo translate('Cell Type'); ?>:</td>
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
			<input type="button" class="" name="cancel" onClick="javascript: popup_close();" value="<?php echo translate('Cancel'); ?>"/>
		</td>
		<td align="right">
			<input type="button" class="sq-btn-blue" name="ok" onClick="javascript: popup_save(this.form)" value="<?php echo translate('Save'); ?>"/>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__).'/footer.php'); ?>