<?php
header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

include(dirname(__FILE__)."/header.php");
?> 
<script language="JavaScript">

	function popup_init() {

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		var f = document.main_form;

		f.width.value		= (data['width']	  == null) ? "" : data['width'];
		f.height.value		= (data['height']	  == null) ? "" : data['height'];
		f.bgcolor.value		= (data['bgcolor']	  == null) ? "" : data['bgcolor'];
		//var bgimage			= (data['background'] == null) ? "" : data['background'];
		//bgfileid = bgimage.substr(5,bgimage.length);
		//f.background.value = bgimage;
		//owner.highlightComboElement(f.fileid,      bgfileid);
		owner.highlight_combo_value(f.align,	   data['align']);
		owner.highlight_combo_value(f.border,	   data['border']);
		owner.highlight_combo_value(f.cellspacing, data['cellspacing']);
		owner.highlight_combo_value(f.cellpadding, data['cellpadding']);

		f.tableid.value = owner.bodycopy_current_edit["data"]["tableid"];
		f.bodycopy_name.value = owner.bodycopy_current_edit["bodycopy_name"];

	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["width"]			= owner.element_value(f.width);
		data["height"]			= owner.element_value(f.height);
		data["bgcolor"]			= owner.element_value(f.bgcolor);
		//data["background"]		= owner.element_value(f.background);
		data["align"]			= owner.element_value(f.align);
		data["border"]			= owner.element_value(f.border);
		data["cellspacing"]		= owner.element_value(f.cellspacing);
		data["cellpadding"]		= owner.element_value(f.cellpadding);
		owner.bodycopy_save_table_properties(data);
	}

	/*
	function set_file_url(f, changed) {

		var fileid = "";
		if (changed == "fileid") {
			fileid = f.fileid.options[f.fileid.selectedIndex].value;
		} else if (changed == "other_fileid") {
			fileid = f.other_fileid.value;
			f.fileid.options[0].selected = true;
		} else {
		}

		if (fileid != "") {
			f.background.value = "./?f=" + fileid;
		} else {
			f.background.value = "";
		}

	}// end set_file_url()
	*/

</script>
<table width="100%" border="0">
<form name="main_form">
	<input type="hidden" name="bodycopy_name" value="">
	<input type="hidden" name="tableid" value="">
	<tr>
		<td nowrap align="left">
		<a href="javascript: owner.bodycopy_delete_table(document.main_form.bodycopy_name.value, document.main_form.tableid.value);"><img src="<?php echo sq_web_path('data')?>/bodycopy/files/images/icons/delete.gif" width="20" height="20" border="0"></a></td>
		<td nowrap class="bodycopy-popup-heading">Edit Table Properties&nbsp;</td>
	</tr>
	<tr>
		<td colspan="2">
			<hr>
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<table border="0" cellpadding="0" cellspacing="4">
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Width :</td>
					<td valign="middle">
						<input type="text" name="width" value="" size="5">
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Height :</td>
					<td valign="middle">
						<input type="text" name="height" value="" size="5">
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Background Colour :</td>
					<td valign="middle">
						<?php colour_box('bgcolor', '', true, '*',true, false, false);?>
					</td>
				</tr>
				<!-- <tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Background Image :</td>
					<td valign="middle">
						<input type="hidden" name="background" value="">
						<?php
							/*
							# let's see if the passed URL can help us with a fileid
							if (ereg("^\\./\\?f=([0-9]+)", $url, $regs)){
								$fileid = $regs[1];
							}#end if

							$web  = &get_web_system();
							$page = &$web->get_page($pageid);

							if ($page->id) $fileids = &$page->file_index;
							else $fileids = Array();

							$files = Array("" => "");

							if ($fileids) {
								foreach($fileids as $id) {
									$file = &$page->get_file($id);
									if (is_image($file->filename)) {
										$files[$id] = $file->filename;
									}#end if
								}#end foreach
							}#end if
							echo combo_box("fileid", $files, $fileid, "onChange=\"javascript: set_file_url(this.form, 'fileid');\"",20);
							*/
						?>
					</td>
				</tr> -->
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Alignment :</td>
					<td valign="middle">
						<select name="align">
							<option value=""      >
							<option value="left"  >Left
							<option value="center">Centre
							<option value="right" >Right
						</select>
					</td>
				</tr>
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Border :</td>
					<td valign="middle">
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
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Cell Spacing :</td>
					<td valign="middle">
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
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Cell Padding :</td>
					<td valign="middle">
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
		</td>
	</tr>
	<tr>
		<td>
			<hr>
		</td>
	</tr>
	<tr>
		<td align="center">
			<input type="button" value="Save"   onClick="javascript: popup_save(this.form)">
			<input type="button" value="Cancel" onClick="javascript: popup_close();">
		</td>
	</tr>
</form>
</table>
<?php include(dirname(__FILE__)."/footer.php"); ?> 