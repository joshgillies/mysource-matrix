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
* $Id: insert_div.php,v 1.3.2.3 2004/03/02 18:35:51 brobertson Exp $
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
* @subpackage __core__
*/
include(dirname(__FILE__)."/header.php");
?> 
<script language="JavaScript" type="text/javascript">

	function popup_init() {
		var f = document.main_form;
	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"]  = owner.form_element_value(f.identifier);
		data["layout_type"] = owner.form_element_value(f.layout_type);
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
				<tr>
					<td>&nbsp;</td>
					<td class="bodycopy-popup-heading">Presentation:</td>
					<td>
						<select name="layout_type">
							<option value="div" >Block-level</option>
							<option value="span">Inline</option>
							<option value="none">Raw HTML</option>
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
			<input type="button" name="save_button" value="Save" onclick="javascript: popup_save(this.form)">
			<input type="button" value="Cancel" onclick="javascript: popup_close();">
		</td>
	</tr>
</form>
</table>
<?php include(dirname(__FILE__)."/footer.php"); ?>