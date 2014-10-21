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
* $Id: edit_div_props.php,v 1.27 2012/08/30 01:09:05 ewang Exp $
*
*/


/**
* DIV Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.27 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
header('Pragma: no-cache');
header('Expires: '.gmdate('D, d M Y H:i:s',time()-3600).' GMT');

include(dirname(__FILE__).'/header.php');
?>
<script type="text/javascript" src="<?php echo sq_web_path('lib')?>/js/general.js"></script>
<script type="text/javascript">

	function popup_init() {

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		available_types = owner.bodycopy_current_edit["data"]["available_types"];
		var f = document.main_form;

		f.identifier.value = (data['identifier'] == null) ? "" : data['identifier'];
		f.desc.value  	   = (data['desc']       == null) ? "" : data['desc'];
		f.dir.value		   = (data['dir']  		 == null) ? "" : data['dir'];
		f.css_class.value  = (data['css_class']  == null) ? "" : data['css_class'];

		css_class_list = owner.bodycopy_current_edit["data"]["available_classes"];
		if (css_class_list != null) {
			var checked = 0;
			var i = 1;
			for (var key in css_class_list) {
				f.css_class_list.options[i] = new Option(css_class_list[key], key);
				if (key == f.css_class.value) {
					f.css_class_list.value = f.css_class.value;
					checked = 1;
				}
				i++;
			}
			if (checked == 0) {
				f.css_class_list.options[0].value = f.css_class.value;
			}
		}

		f.divid.value = owner.bodycopy_current_edit["data"]["divid"];
		f.bodycopy_name.value = owner.bodycopy_current_edit["bodycopy_name"];
		owner.highlight_combo_value(f.layout_type, data['layout_type']);

		// remove the existing values
		for(var i = f.content_type.options.length - 1; i >= 0; i--) {
			f.content_type.options[i] = null;
		}
		var i = 0;

		// add default "Leave Unchanged" value to the top
		f.content_type.options[i] = new Option('<?php echo translate('content_type_no_change'); ?>', "");
		i++;
		for(var key in available_types) {
			if (available_types[key] == null) continue;
			if(available_types[key]["name"] != null) {
				f.content_type.options[i] = new Option(available_types[key]["name"], key);
				i++;
			}
		}

		if (typeof data["content_type"] == 'undefined' && typeof available_types['content_type_wysiwyg'] != 'undefined')
			data["content_type"] = 'content_type_wysiwyg';
		owner.highlight_combo_value(f.content_type, '');
		f.disable_keywords.checked = (data["disable_keywords"] == "1");

	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"]       = owner.form_element_value(f.identifier);
		data["desc"]             = owner.form_element_value(f.desc);
		data["css_class"]        = owner.form_element_value(f.css_class);
		data["layout_type"]      = owner.form_element_value(f.layout_type);
		data["content_type"]     = owner.form_element_value(f.content_type);
		data["disable_keywords"] = owner.form_element_value(f.disable_keywords);
		data["dir"] 			 = owner.form_element_value(f.dir);
		if (f.css_class_list.options.length > 1) {
			classes = new Array();
			for(var i = 1; i < f.css_class_list.options.length; i++) {
				classes.push(f.css_class_list.options[i].value);
			}
			data["css_class_list"] = classes;
		}
		owner.bodycopy_save_div_properties(data);
	}

	function set_class(value) {
		document.main_form.css_class.value = value;
	}

</script>

<h1 class="title">
	<a href="#" onclick="javascript: popup_close(); return false;">
		<img src="<?php echo sq_web_path('lib')?>/web/images/icons/cancel.png" alt="Cancel" title="<?php echo translate('cancel');?>" class="sq-icon">
	</a>
	<?php echo translate('container_properties'); ?>
</h1>
<script type="text/javascript">
if (owner.bodycopy_current_edit["can_delete"] == false) { document.getElementById('sq_edit_div_props_delete').innerHTML = '&nbsp;'; }
</script>
<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="divid" value="">
<table>
	<tr>
		<td colspan="2">
		<h2><?php echo translate('identification'); ?></h2>
		<fieldset>
			<table>
				<tr>
					<td class="label"><?php echo translate('name'); ?>:</td>
					<td><input type="text" name="identifier" value="" size="25"></td>
				</tr>
				<tr>
					<td class="label"><?php echo translate('description'); ?>:</td>
					<td><textarea name="desc" rows="3" size="25" value=""></textarea></td>
				</tr>
			</table>
		</fieldset>
		<h2><?php echo translate('style_information'); ?></h2>
		<fieldset>
			<table>
				<tr>
					<td class="bodycopy-popup-heading"><?php echo translate('presentation'); ?>:</td>
					<td>
						<select name="layout_type">
							<option value="div" ><?php echo translate('block-level'); ?></option>
							<option value="span"><?php echo translate('inline'); ?></option>
							<option value="none"><?php echo translate('raw_html'); ?></option>
						</select>
					</td>
				</tr>
				<tr>
					<td class="label"><?php echo translate('class'); ?>:</td>
					<td>
						<input type="text" name="css_class" value="" size="15"><br />
						<select name="css_class_list" onchange="set_class(this.value);">
							<option value=""><?php echo translate('content_type_no_change'); ?></option>
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
			<table>
				<tr>
					<td class="bodycopy-popup-heading"><?php echo translate('bodycopy_direction'); ?></td>
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
			<h2><?php echo translate('content_type'); ?></h2>
			<fieldset>
				<table>
					<tr>
					<?php
					if ($this->status & SQ_SC_STATUS_SAFE_EDITING) {
						echo translate('changing_div_not_allowed_in_safe_edit');
					}
					else {
					?>
						<td class="label"><?php echo translate('content_type'); ?>:</td>
						<td>
						<select name="content_type">
							<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
							<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
							<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
						</select>
						</td>
					</tr>
					<?php
					}
					?>
					<tr>
						<td class="label"><?php echo translate('disable_keywords'); ?>:</td>
						<td>
							<input type="checkbox" id="disable_keywords" name="disable_keywords" value="1">
							<label for="disable_keywords">Yes</label>
						</td>
					</tr>
				</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<h2><?php echo translate('delete_this_dcontainer'); ?></h2>
			<fieldset class="last">
			<?php
			// if asset is in safe edit we dont want user to delete it
			if ($this->status & SQ_SC_STATUS_SAFE_EDITING) {
				echo translate('delete_this_div_not_allowed_safe_edit');
			}
			// if delete div is disabled by user preference, do not print the icon
			else if ($GLOBALS['SQ_SYSTEM']->getUserPrefs('bodycopy_container', 'SQ_DIV_DISABLE_DELETE') === 'yes'){
				echo translate('bodycopy_pref_cannot_insert_new');
			}
			// else print the icon and let user do as he please
			else {
			?>

			<table>
				<tr>
					<td class="label"><?php echo translate('click_to_delete'); ?>:</td>
					<td>
						<?php	sq_print_icon(sq_web_path('data').'/asset_types/bodycopy/images/icons/delete.png', 16, 16, 'Delete This Container', 'Delete This Container', 'onclick="owner.bodycopy_delete_div(document.main_form.bodycopy_name.value, document.main_form.divid.value);" style="cursor: pointer;"'); ?>
					</td>
				</tr>
				</table>
			<?php
				}
			?>
			</fieldset>
		</td>
	</tr>
	<tr class="sq-popup-footer">
		<td align="left">
			<input type="button" class="" name="cancel" onClick="javascript: popup_close();" value="<?php echo translate('cancel'); ?>"/>
		</td>
		<td align="right">
			<input type="button" class="sq-btn-blue" name="ok" onClick="javascript: popup_save(this.form)" value="<?php echo translate('set_properties'); ?>"/>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__).'/footer.php'); ?>
