<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: insert_link.php,v 1.9 2003/11/05 01:36:53 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

error_reporting(E_ALL);
require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

$url_protocol_options = Array(
							''  => '',
							'http://'  => 'http://',
							'https://' => 'https://',
							'mailto:'  => 'mailto:',
							'ftp://'   => 'ftp://'
							);

$new_window_bool_options = Array(
								'toolbar'    => 'Show Tool Bar',
								'menubar'    => 'Show Menu Bars',
								'location'   => 'Show Location Bar',
								'status'     => 'Show Status Bar',
								'scrollbars' => 'Show Scroll Bars',
								'resizable'  => 'Allow Resizing'
								);

if (!isset($_GET['assetid']))     $_GET['assetid'] = 0;
if (!isset($_GET['url']))         $_GET['url'] = 0;
if (!isset($_GET['protocol']))    $_GET['protocol'] = 0;
if (!isset($_GET['status_text'])) $_GET['status_text'] = '';
if (!isset($_GET['new_window']))  $_GET['new_window'] = 0;

if (!isset($_GET['new_window'])) {
	foreach ($new_window_bool_options as $option => $option_text) {
		$_GET['new_window_options'][$options] = 0;
	}
} else {
	$_GET['new_window_options'] = var_unserialise($_GET['new_window_options']);
}

?>

<html>
	<head>
		<title>Insert Link</title>

		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>

		<script type="text/javascript">
			var parent_object = opener.editor_<?php echo $_REQUEST['editor_name']?>._object;
			
			window.opener.onFocus = function() { getFocus(); }
			parent_object.onFocus = function() { getFocus(); }

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			var new_window_bool_options = new Array('<?php echo implode("','", array_keys($new_window_bool_options))?>');

			function Init() {
				__dlg_init("matrixInsertLink");
				enable_new_window(document.main_form, <?php echo $_GET['new_window']?>);

				var e = '^(.+:(\/\/)?)?([^#]*)(#(.*))?$';
				var re = new RegExp(e, '');
				var results = re.exec('<?php echo $_GET['url']?>');
				setUrl(results[1], results[3]);

				var changeButton = document.getElementById('sq_asset_finder_assetid_change_btn');
				changeButton.click();
			};

			function onOK() {
				// pass data back to the calling window
				var fields = ["url"];
				var param = new Object();
				var f = document.main_form;

				param["url"]         = form_element_value(f.url_protocol) + form_element_value(f.url_link);
				param["status_text"] = form_element_value(f.status_text);
				param["new_window"]  = form_element_value(f.new_window);

				param["new_window_options"] = new Object();
				param["new_window_options"]["width"]  = form_element_value(f.width);
				param["new_window_options"]["height"] = form_element_value(f.height);
				for(var i=0; i < new_window_bool_options.length; i++) {
					param["new_window_options"][new_window_bool_options[i]] = (f.elements[new_window_bool_options[i]].checked) ? 1 : 0;
				}

				__dlg_close("matrixInsertLink", param);
				window.opener.focus();
				return false;
			};

			function onCancel() {
				__dlg_close("matrixInsertLink", null);
				window.opener.focus();
				return false;
			};

			function setUrl(protocol, link) {
				var f = document.main_form;
				
				if (protocol != null) highlight_combo_value(f.url_protocol, protocol);
				if (link     != null) {
					f.url_link.value = link;
				} else {
					var assetid = form_element_value(f.assetid);

					if (assetid > 0) {
						f.url_link.value = './?a=' + assetid;
						highlight_combo_value(f.url_protocol, '');
					}
				}
				setTimeout('self.focus()',100);
			};

			function enable_new_window(f, enable) {
				var bg_colour = '#' + ((enable == 1) ? 'ffffff' : 'c0c0c0');
				var disable = (enable != 1);

				// make sure that the new window box says what it's supposed to
				highlight_combo_value(f.new_window, enable);

				f.width.disabled  = disable;
				f.height.disabled = disable;
				f.width.style.backgroundColor  = bg_colour;
				f.height.style.backgroundColor = bg_colour;
				for(var i=0; i < new_window_bool_options.length; i++) {
					f.elements[new_window_bool_options[i]].disabled = disable;
				}
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
				width: 70px;
			}

			/* colour picker button styles */
			.buttonColor, .buttonColor-hilite {
				cursor: default;
				border: 1px solid;
				border-color: #9E86AA #725B7D #725B7D #9E86AA;
			}

			.buttonColor-hilite {
				border-color: #402F48;
			}

			.buttonColor-chooser, .buttonColor-nocolor, .buttonColor-nocolor-hilite {
				height: 0.6em;
				border: 1px solid;
				padding: 0px 1em;
				border-color: ButtonShadow ButtonHighlight ButtonHighlight ButtonShadow;
			}

			.buttonColor-nocolor, .buttonColor-nocolor-hilite { padding: 0px; }
			.buttonColor-nocolor-hilite { background: #402F48; color: #FFFFFF; }
		</style>
	</head>

	<body onLoad="Init(); if (opener) opener.blockEvents('matrixInsertLink')" onUnload="if (opener) opener.unblockEvents(); asset_finder_onunload(); parent_object._tmp['disable_toolbar'] = false; parent_object.updateToolbar();">
		
		<div class="title">Insert Link</div>
		
		<form action="" method="get" name="main_form">
			<table width="100%">
				<tr>
					<td>
						<table width="100%" cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="100%">
									<fieldset>
									<legend><b>General</b></legend>
									<table style="width:100%">
										<tr>
											<td class="label">Protocol:</td>
											<td><?php  combo_box('url_protocol', $url_protocol_options, $_GET['protocol'], 'style="font-family: courier new; font-size: 11px;"'); ?></td>
											<td class="label">Link:</td>
											<td><?php text_box('url_link', $_GET['url'], 40, 0)?></td>
										</tr>
										<tr>
											<td class="label">Select Asset:</td>
											<td colspan="3"><?php asset_finder('assetid', $_GET['assetid'], Array(), (($_GET['in_popup']) ? 'opener.opener.top' : 'opener.top'), 'setUrl'); ?></td>
										</tr>
									</table>
									</fieldset>
								</td>
							</tr>
						</table>
					</td>
				</tr>
				<tr>
					<td>
						<table width="100%" cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="100%">
									<fieldset>
										<legend>Options</legend>
										<table style="width:100%">
											<tr>
												<td class="label">Status Bar Text:</td>
												<td><?php text_box('status_text', $_GET['status_text'], 50); ?></td>
											</tr>
										</table>
									</fieldset>
								</td>
							</tr>
						</table>
					</td>
				</tr>
				<tr>
					<td>
						<table width="100%" cellspacing="0" cellpadding="0">
							<tr>
								<td valign="top" width="100%">
									<fieldset>
										<legend>New Window Options</legend>
										<table style="width:100%">
											<tr>
												<td class="label" rowspan="2" valign="top">New Window:</td>
												<td><?php combo_box('new_window', Array('0' => 'No', '1' => 'Yes'), false, $_GET['new_window'], 1, 'onChange="javascript: enable_new_window(this.form, form_element_value(this));"'); ?></td>
											</tr>
											<tr>
												<td>
													<table border="0" cellspacing="0" cellpadding="0">
														<tr>
														<?php
															$count = 0;
															foreach($new_window_bool_options as $var => $name) {
																$count++;
															?> 
																	<td width="33%">
																		<input type="checkbox" value="1" name="<?php echo $var?>" <?php echo ($_GET['new_window_options'][$var]) ? 'checked' : '';?>>
																		<?php echo $name?>
																	</td>
															<?php
																if ($count % 2 == 0) {
																	echo '</tr><tr>';
																}
															}
														?>
														</tr>
														<tr>
															<td colspan="3">
																Size : <input type="text" value="<?php echo $_GET['new_window_options']['width']?>" size="3" name="width"> (w) x <input type="text" value="<?php echo $_GET['new_window_options']['height']?>" size="3" name="height"> (h)
															</td>
														</tr>
													</table>
												</td>
											</tr>
										</table>
									</fieldset>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>

			<div style="margin-top: 5px; text-align: right;">
			<hr />
			<button type="button" name="ok" onclick="return onOK();">OK</button>
			<button type="button" name="cancel" onclick="return onCancel();">Cancel</button>
			</div>
		</form>
	</body>
</html>
