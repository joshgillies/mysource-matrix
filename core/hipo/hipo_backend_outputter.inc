<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: hipo_backend_outputter.inc,v 1.1 2003/10/28 23:21:54 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/include/backend_outputter.inc';


/**
* Backend_Outputter
*
* Purpose
*
*    similar to the standard backend outputter, but colour scheme modified to 
*    suite hipo jobs
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Hipo_Backend_Outputter extends Backend_Outputter
{

	/**
	* Constructor
	*
	* @param string	$type_code
	*
	* @access  public
	*/
	function Hipo_Backend_Outputter()
	{
		$this->Backend_Outputter();

	}//end constructor


	/**
	* Paint the header of the page
	*
	* @access private
	*/
	function _paintHeader()
	{
		if (!headers_sent()) {
			if($this->_charset) {
				header("Content-type: text/html; charset=$this->_charset");
			} else {
				$default_charset = 'iso-8859-1';
				header("Content-type: text/html; charset=$default_charset");
			}
		}
		?>
		<html>
		<head>
			<title><?php echo $this->_heading; ?></title>
			<?php

			$this->_paintCssInclude($this->filesPath('css/edit.css'));
			foreach($this->_css_includes as $file) $this->_paintCssInclude($file);

			$this->_paintJsInclude(sq_web_path('lib').'/html_form/html_form.js');
			$this->_paintJsInclude(sq_web_path('lib').'/js/general.js');
			$this->_paintJsInclude(sq_web_path('lib').'/js/debug.js');
			$this->_paintJsInclude(sq_web_path('lib').'/js/edit.js');
			foreach($this->_js_includes as $file) $this->_paintJsInclude($file);
		?>
			<script language="Javascript" type="text/javascript">
			<!--
				function page_on_load() {

			<?php
				if (isset($_SESSION['backend_outputter_msgs'])) {
					$this->_messages = array_merge($this->_messages, $_SESSION['backend_outputter_msgs']);
					$_SESSION['backend_outputter_msgs'] = Array();
				}
				if (count($this->_messages)) {
					require_once 'XML/Tree.php';
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					$msgs = new XML_Tree();
					$msgs_root = &$msgs->addRoot('messages');
					foreach($this->_messages as $data) {
						$msg_body = "<p>".htmlentities(nl2br($data['msg'])). "</p>";
						$msg_body .= "<font color=\"#333333\">".$GLOBALS['SQ_SYSTEM']->datetime()."<font>";
						$msgs_root->addChild('message', $msg_body, Array('type' => get_bit_names('SQ_BO_MSG_', $data['type'])));
					}
					?>
					if (top.sidenav && top.sidenav.add_messages) {
						top.sidenav.add_messages("<?php echo str_replace("\n", "\\n", addslashes($msgs->get())); ?>");
					}
					<?php
				}// end if messages

				foreach($this->_on_load_calls as $call) {
					echo "\n".$call;
				}//end foreach

			?>

				}// end page_on_load()

				var SQ_FORM_SUBMITTED = false;
				function form_on_submit() {
					if (SQ_FORM_SUBMITTED) {
						alert('The form has already been submitted, please wait ...');
						return;
					}

			<?php
				foreach($this->_on_submit_calls as $call) {
				?>
					<?php echo $call;?>;
				<?php
				}//end foreach
			?>

					// basically if they get this far then we can submit
					SQ_FORM_SUBMITTED = true;
					return true;

				}// end form_on_submit()

			<?php
				foreach($this->_preload_imgs as $file) {
				?>
					preload_image('<?php echo $file;?>');
				<?php
				}//end foreach
			?>

			// -->
			</script>
		</head>
		<body bgcolor="#FFFFFF" onLoad="javascript: page_on_load();" marginwidth="0" marginheight="0" topmargin="0" leftmargin="0">

		<form action="<?php echo $this->_action.$this->_anchor;?>" name="main_form" method="post" enctype="multipart/form-data" onSubmit="javascript: return form_on_submit();">
	<?php
		hidden_field('process_form', '1');
		foreach($this->_hidden_fields as $name => $value) {
			hidden_field($name, $value);
		}//end foreach

		if ($this->_heading || count($this->_screens) || count($this->_static_screens)) {
			?>
			<table width="100%" cellpadding="3" cellspacing="0" border="0">
				<tr>
					<td class="sq-backend-main-heading" bgcolor="#735B7D" valign="middle"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="5" height="1"/><?php echo $this->_heading;?></td>
					<td class="sq-backend-main-heading" style="text-align: right;">
					<div id="sq_screen_menu_filler" style="display: none"></div>
					<div id="sq_screen_menu">
					<?php
						if (empty($this->_screens) && empty($this->_static_screens)) {
							echo '&nbsp;';
						} else {
							if (!empty($this->_screens)) {
								$this->_screens = array_reverse($this->_screens);
								$this->_screens['#'] = '------';
								foreach (array_reverse($this->_static_screens) as $url => $code) {
									$this->_screens[$url] = $code;
								}
								$this->_screens = array_reverse($this->_screens);
							} else {
								$this->_screens = $this->_static_screens;
							}

							combo_box('screen_menu', $this->_screens, false, $this->_current_screen, 1, 'id="screen_menu" onChange="javascript: self.location = form_element_value(this);"');
							echo '&nbsp;';
							normal_button('screen_menu_go', ' Go ', 'self.location = get_form_element_value("screen_menu");', 'id="screen_menu_go"');
							echo '&nbsp;';
						}
					?>
					</div>
					</td>
				</tr>
			</table>
			<?php
		}// end if

	}//end _paintHeader()


	/**
	* Paint the passed section onto the page
	*
	* @access private
	*/
	function _paintSection($section, $depth=0)
	{
		if ($section['type'] == 'raw') {
			echo $section['contents'];
		} else {

		?>
			<a name="section_<?php echo $section['section_count']; ?>"></a>
			<table width="100%" cellpadding="0" cellspacing="1" border="0">
			<?php
			if ($section['heading']) {
				if ($depth > 0) {
					// printing a nested section - needs to look a little different
					?>
					<tr>
						<td class="sq-backend-section-subheading" colspan="3"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				} else {
					// printing a top level section
					?>
					<tr>
						<td class="sq-backend-section-heading" colspan="3"><?php echo $section['heading'];?></td>
					</tr>
					<tr>
						<td colspan="3">
							<table width="100%" cellpadding="0" cellspacing="0" border="0">
								<tr>
									<td width="100%" bgcolor="#000000"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="1" /></td>
								</tr>
							</table>
						</td>
					</tr>
					<?php
				}
			} // endif

			// print the section seperator
			?>
			<tr>
				<td colspan="3"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td>
			</tr>
			<tr>
				<td width="20"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="20" height="1" /></td>
				<td colspan="2">
					<table cellspacing="0" cellpadding="0" border="0" width="100%">
			<?php

			for($j = 0; $j < count($section['areas']); $j++) {
				switch($section['areas'][$j]['area_type']) {
					case 'section' :
						?>
							<tr>
								<td width="100%" valign="top" colspan="3">
								<?php
									$this->_paintSection($section['areas'][$j], $depth + 1);
								?>
								</td>
							</tr>
						<?php

						break;

					case 'field' :
						$field = &$section['areas'][$j];
						switch ($field['format']) {
							case 'new_line' :
								?>
									<tr>
										<td nowrap valign="top" class="sq-backend-field" colspan="3">
											<a name="field_<?php echo $field['field_count']; ?>"></a>
											<?php echo $field['name'];?>
										</td>
									</tr>
									<tr>
										<td valign="top" class="sq-backend-data" colspan="3">
											<table cellpadding="0" cellspacing="0" border="0">
												<tr>
													<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="5" height="1"/></td>
													<td width="100%" valign="top" class="sq-backend-data">
													<?php
														echo $field['contents'];
														if ($field['note']) $this->note($field['note']);
													?>
													</td>
												</tr>
											</table>
										</td>
									</tr>
								<?php
							break;

							case 'commit' :
								?>
								<tr>
									<td width="100%" valign="top" class="sq-backend-data" style="text-align: right;" colspan="3">
									<?php
										echo $field['contents'];
										if ($field['note']) $this->note($field['note']);
									?>
									</td>
								</tr>
								<?php
							break;

							case 'blank' :
								// a blank field to be used just for formatting
							break;

							default :
							?>
								<tr>
									<td nowrap valign="top" class="sq-backend-field">
										<a name="field_<?php echo $field['field_count']; ?>"></a>
										<?php echo $field['name'];?>
									</td>
									<td width="10"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
									<td width="100%" valign="top" class="sq-backend-data">
									<?php
										echo $field['contents'];
										if ($field['note']) $this->note($field['note']);
									?>
									</td>
								</tr>
								<tr>
									<td colspan="3"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td>
								</tr>
							<?php
						}//end switch

						break;

					default :
						trigger_error('Unknown Area Type : "'.$section['areas'][$j]['area_type'].'"', E_USER_ERROR);

				}// end switch area type

			}//end for

			?>
					</table>
				</td>
			</tr>
			</table>
			<table width="100%" cellpadding="0" cellspacing="0" border="0"><tr><td width="100%"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td></tr></table>
			<?php

		}//end if section type == raw

	}//end _paintSection()

}//end class

?>