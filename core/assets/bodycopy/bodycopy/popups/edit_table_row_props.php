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
* $Id: edit_table_row_props.php,v 1.6 2006/01/17 04:55:16 lwright Exp $
*
*/

/**
* Table Row Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
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

	}// end popup_init()

	function save_props(f) {
		var data = new Object();
		data["height"]  = owner.form_element_value(f.height);
		data["bgcolor"] = owner.form_element_value(f.bgcolor);
		owner.bodycopy_save_table_row_properties(data);
	}
</script>

<div class="title"><?php echo translate('table_row_properties'); ?></div>

<form name="main_form">
<table width="100%" border="0" class="bodycopy-popup-table">
	<tr>
		<td>
			<fieldset>
			<legend><?php echo translate('properties'); ?></legend>
			<table border="0" cellpadding="0" cellspacing="4">
				<tr>
					<td class="label"><?php echo translate('height'); ?>:</td>
					<td><input type="text" name="height" value="" size="5"></td>
				</tr>
				<tr>
					<td class="label"><?php echo translate('background_colour'); ?>:</td>
					<td><?php colour_box('bgcolor', '', TRUE, '*',TRUE, FALSE, FALSE);?></td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td>
			<div style="text-align: right;">
			<button type="button" name="ok" onClick="javascript: save_props(this.form)"><?php echo translate('ok'); ?></button>
			&nbsp;
			<button type="button" name="cancel" onClick="javascript: popup_close();"><?php echo translate('cancel'); ?></button>
			</div>
		</td>
	</tr>
</table>
</form>
<?php include(dirname(__FILE__).'/footer.php'); ?>
