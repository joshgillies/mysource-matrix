<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: edit_table_orderer.php,v 1.10 2006/12/05 05:39:48 emcdonald Exp $
*
*/

/**
* Table Orderer Pop-Up
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
<script language="JavaScript" type="text/javascript">

	var order_type = null;

	function popup_init() {

		order_type = owner.bodycopy_current_edit["data"]["order_type"];
		var type_order = owner.bodycopy_current_edit["data"][order_type + "_order"];
		var f = document.main_form;

		// remove all old entries
		while(f.type_order.options.length) {
			f.type_order.options[0] = null;
		}

		for(var i = 0; i < type_order.length; i++) {
			f.type_order.options[i] = new Option(type_order[i], i);
		}

	}// end popup_init()

	function popup_save(f) {

		var type_order = new Array();
		for(var i = 0; i < f.type_order.options.length; i++) {
			type_order[i] = f.type_order.options[i].value;
		}

		switch(order_type) {
			case "table" :
				owner.bodycopy_save_table_order(type_order);
			break;

			case "row" :
				owner.bodycopy_save_table_row_order(type_order);
			break;

			case "col" :
				owner.bodycopy_save_table_col_order(type_order);
			break;

			default :
				alert(js_translate('order_type_unknown', order_type));

		}//end switch

	}// end popup_save()

	function popup_move_type(move_up) {
		owner.move_combo_selection(document.main_form.type_order, move_up);
	}

</script>
<table width="100%" border="0">
<form name="main_form">
	<tr>
		<td nowrap class="bodycopy-popup-heading"><?php echo translate('reorderer'); ?></td>
	</tr>
	<tr>
		<td><hr></td>
	</tr>
	<tr>
		<td align="center">
			<table border="0" cellspacing="3" cellpadding="0">
				<tr>
					<td>
						<select name="type_order" size="10">
							<!-- good old Netscape :) -->
							<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
						</select>
					</td>
					<td>
						<a href="#" onClick="javascript: popup_move_type(true); return false;" onMouseOver="window.status='Move the Selection Up'; return true;" onMouseOut="javascript: window.status=''; return true;"><img src="<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/up_arrow.gif" width="15" height="15" border="0"></a><br>
						<br>
						<br>
						<a href="#" onClick="javascript: popup_move_type(false); return false;" onMouseOver="window.status='Move the Selection Down'; return true;" onMouseOut="javascript: window.status=''; return true;"><img src="<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/down_arrow.gif" width="15" height="15" border="0"></a>
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
			<input type="button" value="Save" onclick="javascript: popup_save(this.form)">
			<input type="button" value="Cancel" onclick="javascript: popup_close();">
		</td>
	</tr>
</form>
</table>
<?php include(dirname(__FILE__).'/footer.php'); ?>
