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
* $Id: search_and_replace.php,v 1.5 2013/07/25 23:25:17 lwright Exp $
*
*/

/**
* Search and replaces given text in the WYSIWYG
*
* @author	Chiranjivi Upreti <cupreti@squiz.com.au>
* @version 	$Revision: 1.5 $
* @package 	MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

$_GET['name'] = array_get_index($_GET, 'name', '');

?>

<html style="width: 370px; height: 280px;">

	<head>
		<title>Search and Replace</title>
		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js' ?>"></script>

		<script type="text/javascript">

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			function Init() {
				__dlg_init("SearchAndReplace");
				setTimeout('self.focus()',100);
			};

			function onOK(action_type) {

				var search_str = document.getElementById("search_str").value;
				var replace_str = document.getElementById("replace_str").value;
				
				if (document.getElementById("rep_type4").checked) {
                    var confirm_str = "WARNING!\nUnlike other match options, regular expression replacement will be applied to the actual html content, therefore appearance of the content may be affected and also the action cannot be undone.\nAre you sure you want to apply the replacement?";
				} else {
                    var confirm_str = "WARNING!\nAll the occurance of the search string will be replaced and the action cannot be undone.\nAre you sure you want to apply the replacement?";
				}

				if ((action_type != "replace_all" && action_type != "replace_selection") || confirm(confirm_str)) {
					var parameter_types = new Array();

					parameter_types.push(action_type);
					parameter_types.push(search_str);
					parameter_types.push(replace_str);					

					var i = 0;
					while (document.getElementById("rep_type"+i) != null) {
						parameter_types.push(document.getElementById("rep_type"+i++).checked);
					}

					getFocus();
					__dlg_close("SearchAndReplace", parameter_types, false);
					return true;
				}
				return false;
			};

			function enableButtons() {
				
				if (document.getElementById("search_str").value != '') {
					// Replace/Search text by text feature is not available for regular expression search
					if (!document.getElementById("rep_type4").checked) {
						document.getElementById("search_previous").disabled = false;
						document.getElementById("search_next").disabled = false;					
						document.getElementById("replace_current").disabled = false;
					} else {					
						document.getElementById("search_previous").disabled = true;
						document.getElementById("search_next").disabled = true;					
						document.getElementById("replace_current").disabled = true;
					}

					document.getElementById("replace_all").disabled = false;					
					document.getElementById("replace_selection").disabled = false;

				} else {
					document.getElementById("search_previous").disabled = true;
					document.getElementById("search_next").disabled = true;					
					document.getElementById("replace_current").disabled = true;					
					document.getElementById("replace_all").disabled = true;					
					document.getElementById("replace_selection").disabled = true;
				}
			};

			function fixMatchTypeCheckboxes(type) {
				match_types = Array('1', '2', '3', '4');
				
				if (document.getElementById("rep_type"+type).checked) {					
					for(index in match_types) {
						rep_type = match_types[index];
						if (rep_type != type) document.getElementById("rep_type"+rep_type).checked = false;
					}
					
				}//end if
				
				enableButtons();	
			};

			function onCancel() {
				__dlg_close("SearchAndReplace", null);
				return false;
			};

		</script>

		<style type="text/css">
			html, body {
				background: #FCFCFC;
				color: #000000;
				font: 11px Tahoma,Verdana,sans-serif;
				margin: 0px;
				padding: 0px;
				padding: 5px;
			}

			table {
				font: 11px Tahoma,Verdana,sans-serif;
			}

			/* main popup title */
			.title {
				background: #402F48;
				color: #FFFFFF;
				font-weight: bold;
				font-size: 120%;
				padding: 3px 10px;
				margin-bottom: 10px;
				border-bottom: 1px solid black;
				letter-spacing: 4px;
			}

			/* fieldset styles */
			fieldset {
				padding: 0px 10px 5px 5px;
				border-color: #725B7D;
			}

			.fl { width: 9em; float: left; padding: 2px 5px; text-align: right; }
			.fr { width: 7em; float: left; padding: 2px 5px; text-align: right; }

			/* form and form fields */
			form { padding: 0px; margin: 0px; }

			select, input, button {
				font: 11px Tahoma,Verdana,sans-serif;
			}

			button {
				width: 110px;
			}
		</style>
	</head>

	<body onload="Init()">

		<div class="title">Search And Replace</div>

		<form action="" method="get" name="Form1">
			<table border="0" width="100%">
				
				<tr>
					<td>
						<fieldset>
							<legend><b>Search and replace string</b></legend>
							<table style="width:100%">
								<tr>
									<td>Search string</td>
									<td><input type="text" name="search_str" id="search_str" size="45" onkeyup="enableButtons()">
								</tr>
								<tr>
									<td>Replace string</td>
									<td><input type="text" name="replace_str" id="replace_str" size="45">
								</tr>
							</table>
						</fieldset>
					</td>
				</tr>

				<tr>
					<td>
						<fieldset>
							<legend><b>Replace options</b></legend>
							<table style="width:100%">
								<tr>
									<td>
										<input type="checkbox" name="rep_type0" id="rep_type0" />
										<label for="rep_type0"> Match case</label><br/>
										
										<input type="checkbox" name="rep_type1" id="rep_type1" onClick="fixMatchTypeCheckboxes('1')" />
										<label for="rep_type1"> Match the whole word</label><br/>
										
										<input type="checkbox" name="rep_type2" id="rep_type2" onClick="fixMatchTypeCheckboxes('2')" />
										<label for="rep_type2"> Match beginning of the word</label><br/>
										
										<input type="checkbox" name="rep_type3" id="rep_type3" onClick="fixMatchTypeCheckboxes('3')" />
										<label for="rep_type2"> Match end of the word</label><br/>
										
										<input type="checkbox" name="rep_type4" id="rep_type4" onClick="fixMatchTypeCheckboxes('4')" />
										<label for="rep_type3"> Regular expression match</label><br/>

									</td>
								</tr>
							</table>
						</fieldset>
					</td>
				</tr>
			</table>

			<div style="text-align: center;">

			<button type="button" disabled id="search_next" name="search_next" onclick="if (!onOK('search_next')) return;">Search Next</button>&nbsp;&nbsp;
			<button type="button" disabled id="search_previous" name="search_previous" onclick="if (!onOK('search_previous')) return;">Search Previous</button>&nbsp;&nbsp;
			<button type="button" disabled id="replace_current" name="replace_current" onclick="if (!onOK('replace_current')) return;">Replace</button><br />
			<button type="button" disabled id="replace_all" name="replace_all" onclick="if (!onOK('replace_all')) return;">Replace All</button>&nbsp;&nbsp;			
			<button type="button" disabled id="replace_selection" name="replace_selection" onclick="if (!onOK('replace_selection')) return;">In Selection</button>&nbsp;&nbsp;

			<button type="button" name="cancel" onclick="window.close();">Cancel</button>
			</div>
		</form>
	</body>
</html>
