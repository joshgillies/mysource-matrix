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
* $Id: insert_table.php,v 1.4 2004/06/24 03:29:37 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Insert Table Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

include(dirname(__FILE__)."/header.php");
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
			alert("Please enter a positive number\n");
			field.value = input_default;
			field.focus();
		} else {
			field.value = num;
		}
	}

</script>

<div class="title" style="text-align: right;">Insert Table</div>

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
						<legend><b>Layout</b></legend>
						<table style="width:100%">
							<tr>
								<td class="label">Columns:</td>
								<td><input type="text" name="rows" value="1" size="3" onChange="javascript: set_pos_int(this, 1);"></td>
							</tr>
							<tr>
								<td class="label">Rows:</td>
								<td><input type="text" name="cols" value="1" size="3" onChange="javascript: set_pos_int(this, 1);"></td>
							</tr>
							<tr>
								<td class="label">Width:</td>
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
			<legend><b>Table Styles / Colours</b></legend>
				<table width="100%">
					<tr>
						<td class="label">Background Colour:</td>
						<td><?php colour_box('bgcolor', '', true, '*',true, false, false);?></td>
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