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
* $Id: edit_table_orderer.php,v 1.11 2012/08/30 01:09:05 ewang Exp $
*
*/

/**
* Table Orderer Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
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
<h1 class="title">
	<a href="#" onclick="javascript: popup_close(); return false;">
		<img src="<?php echo sq_web_path('lib')?>/web/images/icons/cancel.png" alt="Cancel" title="<?php echo translate('cancel');?>" class="sq-icon">
	</a>
	<?php echo translate('reorderer'); ?>
</h1>
<form name="main_form">
<table>
	<tr>
		<td colspan="2">
			<h2>Reorder rows</h2>
			<fieldset class="last">
				<table>
					<tr>
						<td>
							<select name="type_order" size="10" style="width:200px">
								<!-- good old Netscape :) -->
								<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
							</select>
						</td>
						<td>
							<a href="#" onClick="javascript: popup_move_type(true); return false;" onMouseOver="window.status='Move the Selection Up'; return true;" onMouseOut="javascript: window.status=''; return true;"><img src="<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/up_arrow.png" width="15" height="15" ></a><br>
							<br>
							<br>
							<a href="#" onClick="javascript: popup_move_type(false); return false;" onMouseOver="window.status='Move the Selection Down'; return true;" onMouseOut="javascript: window.status=''; return true;"><img src="<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/down_arrow.png" width="15" height="15" ></a>
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
