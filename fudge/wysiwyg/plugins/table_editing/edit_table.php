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
* $Id: edit_table.php,v 1.35 2013/07/25 23:25:17 lwright Exp $
*
*/

/**
* Table Edit Popup for the WYSIWYG
*
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.35 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once dirname(__FILE__).'/../../wysiwyg_plugin.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

$wysiwyg = null;
$plugin = new wysiwyg_plugin($wysiwyg);
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
	<title>Tables</title>
	<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" />
	<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css" />
	<style type="text/css">




	</style>
	<script type="text/javascript" src="../../core/popup.js"></script>
	<script type="text/javascript" src="../../core/dialog.js"></script>
	<script type="text/javascript">
	//<![CDATA[

		var testtable = '<table id="test2" border="1" cellpadding="0" cellspacing="1" style="width:400px;background:#CCC;background-color:#CCC; background-image:url(images/i.gif)" summary="test2" onclick="if (this.id=\'test4\') {alert(\'s\');}">' +
						'<caption>Caption</caption>'+
						'<tr>'+
						'<td id="test_td0_0" colspan="3" align="center" style="border-color:#000;border-style:solid;border-width:1px;background-color:#6CF;">&nbsp;</td>'+
						'</tr>'+
						'<tr>'+
						'<td id="test_td1_0" rowspan="2" align="center" style="border-color:#000;border-style:solid;border-width:1px;background-color:#30F;">&nbsp;</td>'+
						'<td id="test_td1_1" headers="test_td1_0 test_td0_0 ">test</td>'+
						'<td id="test_td1_2" headers="test_td0_0 test_td1_0 ">&nbsp;</td>'+
						'</tr>'+
						'<tr>'+
						'<td id="test_td2_1" headers="test_td0_0 test_td1_0 ">&nbsp;</td>'+
						'<td id="test_td2_2" headers="test_td0_0 test_td1_0 ">&nbsp;</td>'+
						'</tr>'+
						'</table>';

		var table;
		var init_finished = false;

		function Init() {
			__dlg_init("editTableProperties");
			table = new TTable("table");
			table.Import(window.dialogArguments.table_structure);

			// initialise the drop down for css/html table width
			if (table.width != "") {
				switchWidthView('css');
			} else if (table.htmlwidth != "") {
				switchWidthView('html');
			}
			//alert("style="+table.borderStyle+"\nborder="+ table.border+"\nhtmlborder="+table.htmlborder);
			// initialise the drop down for css/html table border
			if (table.border != null && table.border != "") {
				switchBorderView('css');
			} else {
				switchBorderView('html');
			}

			window.focus();
			init_finished = true;
		};

		function onOK() {
			var message = "";
			var obj = "";
			if (document.getElementById("tid").value == "") {
				message = "Table ID is blank. Continue?";
				obj = document.getElementById("tid");
			}
			if (message == "" || (message != "" && confirm(message) == true)) {
				__dlg_close("editTableProperties", table.Export());
			} else {
				switchPanels('table');
				obj.select();
			}
			return false;

		};

		function onCancel() {
			__dlg_close("editTableProperties", null);
			return false;

		};

		// Called to avoid the mouse move event being called prematurely
		function onMove(event) {
			if ((init_finished == false) || (table.selector == 'table')) return false;
			table.mouse.Move(event);
			return false;

		};

		//Function to switch between the UI elements for each selector
		function switchPanels(_selector) {
			table.selector = _selector;
			var panels = new Array('cell', 'row', 'table', 'col');
			var i = 0;
			while (i < panels.length) {
				var panel = document.getElementById(panels[i] + '_panel');
				var button = document.getElementById('button_' + panels[i]);
				if (table.selector == panels[i]) {
					panel.style.display = "";
					button.style.border = "2px solid #FF8040";
					if (table.r != null) {
						if (table.c == null) {
							table.select(document.getElementById("td" + table.r + "_0"));
						} else {
							table.select(document.getElementById("td" + table.r + "_" + table.c));
						}
					}
				} else {
					panel.style.display = "none";
					button.style.border = "0";
				}
				i++;
			}
			return false;

		};

		function switchWidthView(value) {
			document.getElementById("widthView").value = value;
			if (value == 'css') {
				document.getElementById('width').style['display'] = '';
				document.getElementById('widthtype').style['display'] = '';
				document.getElementById('htmlwidth').style['display'] = 'none';
				document.getElementById('htmlwidthtype').style['display'] = 'none';
				table.setHTMLWidth("");
				table.setWidth(parseInt(document.getElementById('width').value) + document.getElementById('widthtype').value);
			} else if (value == 'html') {
				document.getElementById('width').style['display'] = 'none';
				document.getElementById('widthtype').style['display'] = 'none';
				document.getElementById('htmlwidth').style['display'] = '';
				document.getElementById('htmlwidthtype').style['display'] = '';
				table.setHTMLWidth(parseInt(document.getElementById('htmlwidth').value) + document.getElementById('htmlwidthtype').value);
				table.setWidth("");
			}

		}

		function switchBorderView(value) {
			document.getElementById("borderView").value = value;
			if (value == 'css') {
				document.getElementById('table_border').style['display'] = '';
				document.getElementById('table_bordertype').style['display'] = '';
				document.getElementById('html_table_border').style['display'] = 'none';
				table.setTableHtmlBorder("");
				table.setElementBorder(document.getElementById('table_border').value, document.getElementById('table_bordertype').value);
			} else if (value == 'html') {
				document.getElementById('table_border').style['display'] = 'none';
				document.getElementById('table_bordertype').style['display'] = 'none';
				document.getElementById('html_table_border').style['display'] = '';
				table.setTableHtmlBorder(document.getElementById('html_table_border').value);
				table.setElementBorder("", "");
			}

		}

		//]]>
		</script>
		<script type="text/javascript" src="table-editor.js"></script>
</head>
<body onload="Init();">

	<div class="sq-popup-heading-frame">
		<h1>Table Properties</h1>
	</div>
<div id="main-form">	

	<div id="table_container" onmousemove="onMove(event)" onmouseout="if(init_finished) table.mouse.Out();"></div>

	<div id="mouse_pointers">
		<img alt="" src="images/mousec.gif" id="mtable" style="position:absolute;display:none" />
		<img alt="" src="images/mousec.gif" id="mcell" style="position:absolute;display:none" />
		<img alt="" src="images/mouser.gif" id="mrow" style="position:absolute;display:none" />
		<img alt="" src="images/mouseh.gif" id="mhead" style="position:absolute;display:none" />
		<img alt="" src="images/mousecol.gif" id="mcol" style="position:absolute;display:none" />
	</div>



	<div id="panels" class="sq-fieldsets-table sq-popup-clean-fieldset">
		<fieldset id="global_panel">
			<legend><b><?php echo translate('Selectors'); ?></b></legend>
			<div>
				<img id ="button_table" style="border: 2px solid #FF8040;" alt="" src="images/mtable.gif" onclick="switchPanels('table');" />
				<img id ="button_cell" style="border: 0px solid #FF8040;" alt="" src="images/mc.gif" onclick="switchPanels('cell');" />
				<img id ="button_row" style="border: 0px solid #FF8040;" alt="" src="images/mr.gif" onclick="switchPanels('row');" />
				<img id ="button_col" style="border: 0px solid #FF8040;" alt="" src="images/mcol.gif" onclick="switchPanels('col');" />
			</div>
		</fieldset>

		<!-- Properties for the table selector -->
		<fieldset id="table_panel">
			<legend><b><?php echo translate('Table'); ?></b></legend>
			<label for="tid"><?php echo translate('ID'); ?></label>
			<input type="text" id="tid" name="tid" onkeyup="table.setID(this.value)" /><br />
			<label for="caption"><?php echo translate('Caption'); ?>:</label>
			<input type="text" id="caption" name="caption" onkeyup="table.setCaption(this.value)"/><br />
			<hr />
			<label for="class"><?php echo translate('Class'); ?>:</label>
			<input type="text" id="class" name="class" onkeyup="table.setClass(this.value)" /><br />

			<!-- table html/css width -->
			<label for="width"><?php echo translate('Width'); ?>:</label>
			<select id="widthView" onChange="switchWidthView(this.value)" style="width:65px">
				<option value="html">HTML</option>
				<option value="css" selected="1">CSS</option>
			</select>
			<input type="text" id="width" name="width" onkeyup="table.setWidth(parseInt(document.getElementById('width').value) + document.getElementById('widthtype').value)" style="width:50px" />
			<select id="widthtype" name="widthtype" style="width:45px" onchange="table.setWidth(parseInt(document.getElementById('width').value) + document.getElementById('widthtype').value)">
				<option value="px">px</option>
				<option value="%" selected="selected">%</option>
				<option value="pt">pt</option>
				<option value="em">em</option>
				<option value="ex">ex</option>
			</select>
			<input type="text" id="htmlwidth" name="htmlwidth" onkeyup="table.setHTMLWidth(parseInt(document.getElementById('htmlwidth').value) + document.getElementById('htmlwidthtype').value)" style="width:50px" />
			<select id="htmlwidthtype" name="htmlwidthtype" style="width:45px" onchange="table.setHTMLWidth(parseInt(document.getElementById('htmlwidth').value) + document.getElementById('htmlwidthtype').value)">
				<option value="">px</option>
				<option value="%" selected="selected">%</option>
			</select>

			<!-- table html/css border -->
			<label for="table_border"><?php echo translate('Border'); ?>:</label>
			<select id="borderView" onChange="switchBorderView(this.value)" style="width:65px">
				<option value="html">HTML</option>
				<option value="css" selected="1">CSS</option>
			</select>
			<input type="text"  id="table_border" name="table_border" style="width: 40px" onkeyup="table.setElementBorder((parseInt(document.getElementById('table_border').value)), document.getElementById('table_bordertype').value);" value="0" />
			<select id="table_bordertype" name="table_bordertype" style="width:70px" onchange="table.setElementBorder((parseInt(document.getElementById('table_border').value)), document.getElementById('table_bordertype').value);" >
				<option value="solid" selected="selected">solid</option>
				<option value="dotted">dotted</option>
				<option value="dashed">dashed</option>
				<option value="double">double</option>
				<option value="groove">groove</option>
				<option value="ridge">ridge</option>
				<option value="inset">inset</option>
				<option value="outset">outset</option>
				<option value="none">none</option>
				<option value="hidden">hidden</option>
			</select>
			<input type="text" id="html_table_border" name="html_table_border" style="width: 50px" onkeyup="table.setTableHtmlBorder(parseInt(document.getElementById('html_table_border').value));" value="0" /><br />

			<label for="cellspacing"><?php echo translate('Cell Spacing'); ?>:</label>
			<input type="text" id="cellspacing" name="cellspacing" onkeyup="table.setCellSpacing(parseInt(this.value))" value=""/><br />
			<label for="cellpadding"><?php echo translate('Cell Padding'); ?>:</label>
			<input type="text" id="cellpadding" name="cellpadding" onkeyup="table.setCellPadding(parseInt(this.value))" value=""/><br />
			<hr />
			<label for="summary"><?php echo translate('Summary'); ?>:</label>
			<textarea id="summary" cols="10" rows="3" onkeyup="table.setSummary(this.value)"></textarea>
			<label for="frame"><?php echo translate('Frame'); ?>:</label>
			<select class="sq-popup-select-fixed-width" id="frame" name="frame" onchange="table.setFrame(this.value)">
				<option value=""><?php echo strtolower(translate('Empty')); ?></option>
				<option value="void"><?php echo translate('No sides'); ?></option>
				<option value="above"><?php echo translate('The top side only'); ?></option>
				<option value="below"><?php echo translate('The bottom side only'); ?></option>
				<option value="hsides"><?php echo translate('The top and bottom sides only'); ?></option>
				<option value="vsides"><?php echo translate('The right and left sides only'); ?></option>
				<option value="lhs"><?php echo translate('The left-hand side only'); ?></option>
				<option value="rhs"><?php echo translate('The right-hand side only'); ?></option>
				<option value="box"><?php echo translate('All four sides'); ?></option>
			</select><br />
			<label for="rules"><?php echo translate('Rules'); ?>:</label>
			<select class="sq-popup-select-fixed-width" id="rules" name="rules" onchange="table.setRules(this.value)">
				<option value=""><?php echo strtolower(translate('Empty')); ?></option>
				<option value="none"><?php echo translate('No rules'); ?></option>
				<option value="rows"><?php echo translate('Rules will appear between rows only'); ?></option>
				<option value="cols"><?php echo translate('Rules will appear between columns only'); ?></option>
				<option value="all"><?php echo translate('Rules will appear between all rows and columns'); ?></option>
			</select><br />
		</fieldset>

		<!-- Properties for the row selector -->
		<fieldset id="row_panel" style="display:none">
			<legend><b><?php echo translate('Row Properties') ?></b></legend>
			<label for="row_class"><?php echo translate('Class'); ?></label>
			<input type="text" id="row_class" name="row_class" onkeyup="table.setRowClass(document.getElementById('row_class').value)" disabled="disabled"/><br />
			<label for="row_width"><?php echo translate('Height') ?></label>
			<input type="text" id="row_width" name="row_width" onkeyup="table.setRowHeight(parseInt(document.getElementById('row_width').value) + document.getElementById('row_widthtype').value)" style="width:100px" value="100" disabled="disabled"/>
			<select id="row_widthtype" name="row_widthtype" style="width:50px" onchange="table.setRowHeight(parseInt(document.getElementById('row_width').value) + document.getElementById('row_widthtype').value)" disabled="disabled">
			<option value="px">px</option>
				<option value="%" selected="selected">%</option>
				<option value="pt">pt</option>
				<option value="em">em</option>
				<option value="ex">ex</option>
			</select><br />
			<label for="row_border"><?php echo translate('Border'); ?></label>
			<input type="text" id="row_border" name="row_border" style="width:50px" onkeyup="table.setElementBorder((parseInt(document.getElementById('row_border').value)), document.getElementById('row_bordertype').value);" value="0" disabled="disabled" />
			<select id="row_bordertype" name="row_bordertype" style="width:80px" onchange="table.setElementBorder((parseInt(document.getElementById('row_border').value)), document.getElementById('row_bordertype').value);" disabled="disabled">
				<option value="solid" selected="selected">solid</option>
				<option value="dotted">dotted</option>
				<option value="dashed">dashed</option>
				<option value="double">double</option>
				<option value="groove">groove</option>
				<option value="ridge">ridge</option>
				<option value="inset">inset</option>
				<option value="outset">outset</option>
				<option value="none">none</option>
				<option value="hidden">hidden</option>
			</select>
			<br />
			<hr />
			<label><?php echo 'Modify'; ?></label>
			<img id="row_add" alt="Add Row" title="Add Row" src="images/addrow.gif" onclick="if (table.lastSelect == 'row') table.addrow();"  />
			<img id="row_delete" alt="Delete Row" title="Delete Row" src="images/delrow.gif" onclick="if (table.lastSelect == 'row') table.delrow();"  />
			<img alt="divider" src="images/div.gif" />
			<img id="row_THead" alt="Head Cells" title="Head Cell" src="images/th.gif" onclick="if (table.lastSelect == 'row') table.th();"/><br />
			<label><?php echo 'Horizontal'; ?></label>
			<img id="row_aleft" title="Align Left" alt="Align Left" src="images/al.gif" onclick="if (table.lastSelect == 'row') table.setAlign('left');" />
			<img id="row_acenter" title="Align Center" alt="Align Center" src="images/ac.gif" onclick="if (table.lastSelect == 'row') table.setAlign('center');" />
			<img id="row_aright" title="Align Right" alt="Align Right" src="images/ar.gif" onclick="if (table.lastSelect == 'row') table.setAlign('right');" /><br />
			<label><?php echo 'Vertical'; ?></label>
			<img id="row_atop" title="Align Top" alt="Align Top" src="images/at.gif" onclick="if (table.lastSelect == 'row') table.setVAlign('top');" />
			<img id="row_amiddle" title="Align Middle" alt="Align Middle" src="images/am.gif" onclick="if (table.lastSelect == 'row') table.setVAlign('middle');" />
			<img id="row_abottom" title="Align Bottom" alt="Align Bottom" src="images/ab.gif" onclick="if (table.lastSelect == 'row') table.setVAlign('bottom');" /><br />
		</fieldset>

		<!-- Properties for the column selector -->
		<fieldset id="col_panel" style="display:none;">
			<legend><b>Column Properties</b></legend>
			<label for="col_class">Class Name:</label>
			<input type="text" id="col_class" name="col_class" onkeyup="table.setColClass(document.getElementById('col_class').value)" disabled="disabled"/><br />
			<label for="col_width"><?php echo translate('Width'); ?></label>
			<input type="text" id="col_width" name="col_width" onkeyup="table.setColumnWidth(parseInt(document.getElementById('col_width').value) + document.getElementById('col_widthtype').value)" style="width:100px" value="100" disabled="disabled" />
			<select id="col_widthtype" name="col_widthtype" style="width:50px" onchange="table.setColumnWidth(parseInt(document.getElementById('col_width').value) + document.getElementById('col_widthtype').value)" disabled="disabled">
			<option value="px">px</option>
				<option value="%" selected="selected">%</option>
				<option value="pt">pt</option>
				<option value="em">em</option>
				<option value="ex">ex</option>
			</select>
			<label for="col_border"><?php echo translate('Border'); ?></label>
			<input type="text" id="col_border" name="col_border" style="width: 50px" onkeyup="table.setElementBorder((parseInt(document.getElementById('col_border').value)), document.getElementById('col_bordertype').value);" value="0" disabled="disabled" />
			<select id="col_bordertype" name="col_bordertype" style="width:80px" onchange="table.setElementBorder((parseInt(document.getElementById('col_border').value)), document.getElementById('col_bordertype').value);" disabled="disabled">
				<option value="solid" selected="selected">solid</option>
				<option value="dotted">dotted</option>
				<option value="dashed">dashed</option>
				<option value="double">double</option>
				<option value="groove">groove</option>
				<option value="ridge">ridge</option>
				<option value="inset">inset</option>
				<option value="outset">outset</option>
				<option value="none">none</option>
				<option value="hidden">hidden</option>
			</select>
			<br />
			<hr />
			<label><?php echo translate('Modify'); ?></label>
			<img id="col_add" alt="Add Column" title="Add Column" src="images/addcol.gif" onclick="if (table.lastSelect == 'col') table.addcol(); return false;"  />
			<img id="col_delete" alt="Delete Column" title="Delete Column" src="images/delcol.gif" onclick="if (table.lastSelect == 'col') table.delcol();"  />
			<img alt="divider" src="images/div.gif" />
			<img id="col_THead" alt="Head Cells" title="Head Cell" src="images/th.gif" onclick="if (table.lastSelect == 'col') table.th();"/><br />
			<label><?php echo translate('horizontal'); ?></label>
			<img id="col_aleft" title="Align Left" alt="Align Left" src="images/al.gif" onclick="if (table.lastSelect == 'col') table.setAlign('left');" />
			<img id="col_acenter" title="Align Center" alt="Align Center" src="images/ac.gif" onclick="if (table.lastSelect == 'col') table.setAlign('center');" />
			<img id="col_aright" title="Align Right" alt="Align Right" src="images/ar.gif" onclick="if (table.lastSelect == 'col') table.setAlign('right');" /><br />
			<label><?php echo 'Vertical'; ?></label>
			<img id="col_atop" title="Align Top" alt="Align Top" src="images/at.gif" onclick="if (table.lastSelect == 'col') table.setVAlign('top');" />
			<img id="col_amiddle" title="Align Middle" alt="Align Middle" src="images/am.gif" onclick="if (table.lastSelect == 'col') table.setVAlign('middle');" />
			<img id="col_abottom" title="Align Bottom" alt="Align Bottom" src="images/ab.gif" onclick="if (table.lastSelect == 'col') table.setVAlign('bottom');" /><br />
		</fieldset>

		<!-- Properties for the cell selector -->
		<fieldset id="cell_panel" style="display:none">
			<legend><b><?php echo translate('Cell Properties'); ?></b></legend>
			<label for="cell_class"><?php echo translate('Class'); ?>:</label>
			<input type="text" id="cell_class" name="cell_class" onkeyup="table.setCellClass(document.getElementById('cell_class').value)" disabled="disabled"/><br />
			<label for="cell_width"><?php echo translate('Width'); ?></label>
			<input type="text" id="cell_width" name="cell_width" onkeyup="table.setCellWidth(parseInt(document.getElementById('cell_width').value) + document.getElementById('cell_widthtype').value)" style="width:100px" value="100" disabled="disabled" />
			<select id="cell_widthtype" name="cell_widthtype" onchange="table.setCellWidth(parseInt(document.getElementById('cell_width').value) + document.getElementById('cell_widthtype').value)" disabled="disabled">
			<option value="px">px</option>
				<option value="%" selected="selected">%</option>
				<option value="pt">pt</option>
				<option value="em">em</option>
				<option value="ex">ex</option>
			</select>
			<label for="cell_height"><?php echo translate('Height'); ?></label>
			<input type="text" id="cell_height" name="cell_height" onkeyup="table.setCellHeight(parseInt(document.getElementById('cell_height').value) + document.getElementById('cell_heighttype').value)" style="width:100px" value="100" disabled="disabled" />
			<select id="cell_heighttype" name="cell_heighttype" onchange="table.setCellHeight(parseInt(document.getElementById('cell_height').value) + document.getElementById('cell_heighttype').value)" disabled="disabled">
			<option value="px">px</option>
				<option value="%" selected="selected">%</option>
				<option value="pt">pt</option>
				<option value="em">em</option>
				<option value="ex">ex</option>
			</select>
			<label for="cell_border"><?php echo translate('Border'); ?></label>
			<input type="text" id="cell_border" name="cell_border" style="width:50px" onkeyup="table.setElementBorder((parseInt(document.getElementById('cell_border').value)), document.getElementById('cell_bordertype').value);" value="0" disabled="disabled" />
			<select id="cell_bordertype" name="cell_bordertype" onchange="table.setElementBorder((parseInt(document.getElementById('cell_border').value)), document.getElementById('cell_bordertype').value);" disabled="disabled">
				<option value="solid" selected="selected">solid</option>
				<option value="dotted">dotted</option>
				<option value="dashed">dashed</option>
				<option value="double">double</option>
				<option value="groove">groove</option>
				<option value="ridge">ridge</option>
				<option value="inset">inset</option>
				<option value="outset">outset</option>
				<option value="none">none</option>
				<option value="hidden">hidden</option>
			</select>
			<br />
			<hr />
			<label><?php echo translate('Modify'); ?></label>
			<img id="addcolspan" title="Add ColSpan" alt="Add ColSpan" src="images/thr.gif" onclick="if (table.lastSelect == 'cell') table.addColSpan();"  />
			<img id="delcolspan" alt="Delete ColSpan" title="Delete ColSpan" src="images/thl.gif" onclick="if (table.lastSelect == 'cell') table.delColSpan();" />
			<img id="addrowspan" alt="Add RowSpan" title="Add RowSpan" src="images/tvd.gif" onclick="if (table.lastSelect == 'cell') table.addRowSpan();" />
			<img id="delrowspan" alt="Delete RowSpan" title="Delete RowSpan" src="images/tvu.gif" onclick="if (table.lastSelect == 'cell') table.delRowSpan();" />
			<img alt="divider" src="images/div.gif" />
			<img id="THead" alt="Head Cell" title="Head Cell" src="images/th.gif" onclick="if (table.lastSelect == 'cell') table.th();"/><br />
			<label><?php echo 'Horizontal'; ?></label>
			<img id="cell_aleft" title="Align Left" alt="Align Left" src="images/al.gif" onclick="if (table.lastSelect == 'cell') table.setAlign('left');" />
			<img id="cell_acenter" title="Align Center" alt="Align Center" src="images/ac.gif" onclick="if (table.lastSelect == 'cell') table.setAlign('center');" />
			<img id="cell_aright" title="Align Right" alt="Align Right" src="images/ar.gif" onclick="if (table.lastSelect == 'cell') table.setAlign('right');" /><br />
			<label><?php echo 'Vertical'; ?></label>
			<img id="cell_atop" title="Align Top" alt="Align Top" src="images/at.gif" onclick="if (table.lastSelect == 'cell') table.setVAlign('top');" />
			<img id="cell_amiddle" title="Align Middle" alt="Align Middle" src="images/am.gif" onclick="if (table.lastSelect == 'cell') table.setVAlign('middle');" />
			<img id="cell_abottom" title="Align Bottom" alt="Align Bottom" src="images/ab.gif" onclick="if (table.lastSelect == 'cell') table.setVAlign('bottom');" /><br />
			<hr />
			<label for="abbr"><?php echo translate('Abbr'); ?></label>
			<input type="text" id="abbr" name="abbr" onkeyup="table.setAbbr(this.value)" disabled="disabled" /><br />
			<label for="axis"><?php echo translate('Axis'); ?></label>
			<input type="text" id="axis" name="axis"  onkeyup="table.setAxis(this.value)" disabled="disabled" /><br />
			<label for="scope"><?php echo translate('Scope'); ?></label>
			<select id="scope" name="scope" onchange="table.setScope(this.value)" disabled="disabled">
				<option value=""><?php echo translate('empty'); ?></option>
				<option value="row"><?php echo translate('row'); ?></option>
				<option value="col"><?php echo translate('col'); ?></option>
			</select><br />
			<label for="headings"><?php echo translate('Headings'); ?></label>
			<button id="headings" name="headings" onclick="table.toggleHeaders();" disabled="disabled" ><?php echo translate('Click to Select'); ?></button>
		</fieldset>

		<fieldset id="color_panel">
			<legend><b><?php echo translate('Colour'); ?></b></legend>
			<div style="width:40px;height:40px;position:relative;" id="bgborder" onclick="table.toggleBgBorder();">
				<div id="border" style="width:30px;height:30px;position:absolute;left:10px;top:10px;border:inset 1px;background:url(images/empty.gif)">
					<div style="width:19px;height:19px;position:absolute;left:5px;top:5px;border:outset 1px;background:#FFF;font-size:5px"></div>
				</div>
				<div id="bg" style="width:30px;height:30px;position:absolute;left:0px;top:0px;border:inset 1px;background:url(images/empty.gif)"></div>
			</div>
			<div id="colors" style="margin-top:5px" class="colors">
				<script type="text/javascript">
				//<![CDATA[
					var Clr = Array("0", "3", "6", "9", "C", "F");
					var out = '<table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse">';
					for (h1 = 0; h1 < Clr.length; h1++) {
						if (h1 == 0) out += '<tr>' + '<td style="width:10px;height:10px;font-size:4px;" title="none" onclick="table.setColor(null)"><img src="images/empty.gif" width="10" height="10" alt="none" /></td>';
						else out += '<tr>' + '<td style="width:10px;height:10px;font-size:4px;background:#' + Clr[h1] + Clr[h1] + Clr[h1] + '" title="' + Clr[h1] + Clr[h1] + Clr[h1] + Clr[h1] + Clr[h1] + Clr[h1] + '" onclick="table.setColor(\'' + Clr[h1] + Clr[h1] + Clr[h1] + '\')"></td>';
						for (h2 = 0; h2 < Clr.length; h2++)
							for (h3 = 0; h3 < Clr.length; h3++) {
								color = Clr[h1] + Clr[h2] + Clr[h3];
								title = Clr[h1] + Clr[h1] + Clr[h2] + Clr[h2] + Clr[h3] + Clr[h3]
								out += '<td style="width:10px;height:10px;font-size:4px;background:#' + color + '" title="' + title + '" onclick="table.setColor(\'' + color + '\')"></td>';
							}
						out += '</tr>';
						}
					out += '</table>';
					document.write(out);
				//]]>
				</script>
			</div>
		</fieldset>

		<div class="sq-popup-button-wrapper">			
			<button type="button" onclick="onCancel()"><?php echo translate('Cancel'); ?></button>
			<button accesskey="s" type="button" onclick="onOK()" class="sq-btn-green"><?php echo translate('OK'); ?></button>
		</div>
	</div>
</div>


	<img alt="" src="images/semigray.gif" style="display:none;" id="semigray" />
	<img alt="" src="images/semired.gif" style="display:none;" id="semired" />
	<img alt="" src="images/empty.gif" style="display:none;" id="empty" />
</body>
</html>
