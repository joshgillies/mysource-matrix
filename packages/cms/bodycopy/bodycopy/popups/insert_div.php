<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: insert_div.php,v 1.1 2003/11/03 01:22:28 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Insert DIV Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
include(dirname(__FILE__)."/header.php");
?> 
<script language="JavaScript" type="text/javascript">

	function popup_init() {
		var f = document.main_form;
	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"] = owner.form_element_value(f.identifier);
		owner.bodycopy_save_insert_div(data);
	}

</script>
<table border="0" width="100%">
<form name="main_form">
	<tr>
		<td nowrap class="bodycopy-popup-heading">Insert DIV&nbsp;</td>
	</tr>
	<tr>
		<td>
			<hr>
		</td>
	</tr>
	<tr>
		<td>
			<table border="0" cellpadding="0" cellspacing="4">
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Identifier:</td>
					<td>
						<input type="text" name="identifier" value="" size="15">
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
			<input type="button" name="save_button" value="Save" onclick="javascript: popup_save(this.form)">
			<input type="button" value="Cancel" onclick="javascript: popup_close();">
		</td>
	</tr>
</form>
</table>
<?php include(dirname(__FILE__)."/footer.php"); ?>