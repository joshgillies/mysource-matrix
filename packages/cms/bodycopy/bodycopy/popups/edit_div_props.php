<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: edit_div_props.php,v 1.2 2003/11/10 04:34:10 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* DIV Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

include(dirname(__FILE__)."/header.php");
?>
<script type="text/javascript" language="javascript" src="<?php echo sq_web_path('lib')?>/js/general.js"></script>
<script language="JavaScript" type="text/javascript">

	function popup_init() {

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		var f = document.main_form;

		f.identifier.value = (data['identifier'] == null) ? "" : data['identifier'];
		f.css_class.value  = (data['css_class'] == null)  ? "" : data['css_class'];

		f.divid.value = owner.bodycopy_current_edit["data"]["divid"];
		f.bodycopy_name.value = owner.bodycopy_current_edit["bodycopy_name"];

	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"] = owner.form_element_value(f.identifier);
		data["css_class"]  = owner.form_element_value(f.css_class);
		owner.bodycopy_save_div_properties(data);
	}

</script>

<div class="title">
	<table border="0" cellspacing="0" cellpadding="0">
		<tr>
			<td><a href="javascript: owner.bodycopy_delete_div(document.main_form.bodycopy_name.value, document.main_form.divid.value);" style="cursor: pointer;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/icons/delete.png", "16", "16", "Delete this DIV");</script></a></td>
			<td class="title" width="100%" align="right">DIV Properties</td>
		</tr>
	</table>
</div>

<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="divid" value="">
<table width="100%" border="0">
	<tr>
		<td>
		<fieldset>
			<legend><b>Identification</b></legend>
			<table style="width:100%">
				<tr>
					<td class="label">Name:</td>
					<td><input type="text" name="identifier" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		<fieldset>
			<legend><b>Style Information</b></legend>
			<table style="width:100%">
				<tr>
					<td class="label">Class:</td>
					<td><input type="text" name="css_class" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		</td>
	</tr>
	<tr>
		<td>
		<div style="text-align: right;">
		<button type="button" name="ok" onClick="javascript: popup_save(this.form)">OK</button>
		&nbsp;
		<button type="button" name="cancel" onClick="javascript: popup_close();">Cancel</button>
		</div>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__)."/footer.php"); ?> 