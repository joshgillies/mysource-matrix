<?php

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

// just normal messages, eg update confirmations
define('SQ_BO_MSG_NOTICE',      1);
// warnings, you entered something a bit a-miss and the system fixed it up for you
define('SQ_BO_MSG_WARNING',     2);
// something is just plain wrong, fix immediately
define('SQ_BO_MSG_ERROR',       4);


/**
* Backend_Outputter
*
* Purpose
*
*    This class will be controlling all output for the backend of thesystem,
*    there will only ever be one instance of this object
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Backend_Outputter
{

	// any messages to be displayed to the user
	var $_messages = Array();

	// an link to redirect to when we print, any messages will be carried over to the next page
	var $_redirect = '';
	var $_redirect_target = '';

	// an array of fns to be called when the page loads
	var $_on_load_calls = Array();

	// an array assets that are going to be refreshed in the flash menu
	var $_refreshing_assets = Array();

	// the form action
	var $_action  = '';

	// the anchor the form will be submitted to
	var $_anchor  = '';

	// the page heading
	var $_heading  = '';
	var $_heading_icon = '';
	var $_sub_heading  = '';
	var $_sub_heading_icon = '';

	// stylesheet files to include
	var $_css_includes = Array();

	// javascript files to include
	var $_js_includes = Array();

	// an array of fns to be called when the form is submitted
	var $_on_submit_calls = Array();

	// hidden fields for the form Array([field name] => [value])
	var $_hidden_fields = Array();

	// edit screens
	var $_screens = Array();
	var $_static_screens = Array();
	var $_current_screen = '';

	// the contents of the page split into sections
	var $_contents = Array();

	// used to make sure that each img gets a unique name
	var $_img_button_count = 0;

	// all the preload images names on the page and their preloaded images
	// Array(name => img);
	var $_preload_imgs = Array();

	var $_files_path;

	var $_raw_opened = false;

	var $_section_stack = Array();
	var $_current_field = null;

	var $_section_count = 0;
	var $_field_count   = 0;

	var $_charset = '';


	/**
	* Constructor
	*
	* @param string	$type_code
	*
	* @access  public
	*/
	function Backend_Outputter()
	{

		// if there are some messages from the previous page we had better add them in
		if (!empty($_SESSION['backend_outputter_msgs'])) {
			$this->_messages = $_SESSION['backend_outputter_msgs'];
			$_SESSION['backend_outputter_msgs'] = Array();
		}

		$this->_files_path = sq_web_path('lib').'/web';
		$this->_action = $_SERVER['PHP_SELF'];

		// register that we want to listen to link update events
		$GLOBALS['SQ_SYSTEM']->addEventListener($this, array('CreateLink',
															 'DeleteLink',
															 'AssetUpdate',
															 'AssetTypeUpdate'
															));
	}//end constructor


	/**
	* Set the redirect for the page
	*/
	function setRedirect($link, $target='self')
	{
		$this->_redirect = trim($link);
		$this->_redirect_target = trim($target);
	}//end setRedirect()


	/**
	* Add an onLoad call, something JS that needs to be executed right after the page finishes loading
	*
	* @param string $call	JS code to execute
	*
	* @access public
	*/
	function addOnLoad($call)
	{
		$this->_on_load_calls[] = $call;
	}//end addOnLoad()


	/**
	* Allows you to add get vars to the action section of the form
	*
	* @param string $name
	* @param string $value
	*
	* @access public
	*/
	function addFormActionGetVar($name, $value)
	{
		$this->_action .= ((strstr($this->_action, '?')) ? '&' : '?').rawurlencode($name).'='.rawurlencode($value);
	}//end addFormActionGetVar()


	/**
	* Returns the current action, which realistically is going to be the way to get to the current location
	*
	* @access public
	*/
	function getCurrentLocation()
	{
		return $this->_action;
	}//end getCurrentLocation()


	/**
	* Set the anchor that the page will submit to
	*
	* @access public
	*/
	function setFormAnchor($anchor)
	{
		$anchor = trim($anchor);
		$this->_anchor = ($anchor) ? '//'.$anchor : '';
	}//end setFormAnchor()


	/**
	* Set the heading for page
	*
	* @param string $heading
	* @param string $icon
	*
	* @access public
	*/
	function setHeading($heading, $icon='')
	{
		$this->_heading = trim($heading);
		$this->_heading_icon = $icon;
	}//end setHeading()


	/**
	* Set the sub heading for page
	*
	* @param string $heading
	* @param string $icon
	*
	* @access public
	*/
	function setSubHeading($heading, $icon='')
	{
		trigger_error('Sub Heading not outputting, fix in _paintHeader()', E_USER_WARNING);
		$this->_sub_heading = trim($heading);
		$this->_sub_heading_icon = $icon;
	}//end setSubHeading()


	/**
	* Add a message to be displayed to the user
	*
	* @access public
	*/
	function addMessage($type, $msg)
	{
		$msg = trim($msg);
		// if the type is valid and there is a message
		if(strlen($msg)) {
			$this->_messages[] = Array('type' => $type, 'msg' => $msg);
		}
	}//end addMessage()


	/**
	* Add a stylesheet file to the page
	*
	* @access public
	*/
	function addCssInclude($file)
	{
		if (in_array($file, $this->_css_includes)) return;
		$this->_css_includes[] = $file;
	}//end addCssInclude()


	/**
	* Add a javascript file to the page
	*
	* @access public
	*/
	function addJsInclude($file)
	{
		if (in_array($file, $this->_js_includes)) return;
		$this->_js_includes[] = $file;
	}//end addJsInclude()


	/**
	* Add an onSubmit call, something JS that needs to be executed before the form submits
	*
	* @access public
	*/
	function addOnSubmit($call)
	{
		$this->_on_submit_calls[] = $call;
	}//end addOnSubmit()


	/**
	* Add a hidden field to the output
	*
	* @param string	$name	the name of the field
	* @param string	$value	the value to store in the field
	*
	* @return void
	* @access public
	*/
	function addHiddenField($name, $value='')
	{
		$this->_hidden_fields[$name] = $value;

	}//end addHiddenField()


	/**
	* Adds a screen options to the drop down
	*
	* @param string $url
	* @param string $name
	*
	* @access public
	*/
	function addScreen($url, $name)
	{
		if (empty($url)) return;
		if (in_array($name, $this->_screens)) {
			trigger_error('Screen Code "'.$name.'" already in use', E_USER_ERROR);
		}
		$this->_screens[$url] = $name;
	}//end addScreen()


	/**
	* Sets the current screen, so the combo displays right
	*
	* @param string $url
	*
	* @access public
	*/
	function setCurrentScreen($url)
	{
		if (empty($url)) return;
		$this->_current_screen = $url;
	}//end setCurrentScreen()


	/**
	* Adds a static screen for editing
	*
	* @param string $url
	* @param string $name
	*
	* @access public
	*/
	function addStaticScreen($url, $name)
	{
		if (empty($url)) return;
		if (in_array($name, $this->_static_screens)) {
			trigger_error('Staic Screen Code "'.$name.'" already in use', E_USER_ERROR);
		}
		$this->_static_screens[$url] = $name;
	}//end addTab()


	/**
	* Adds a preload image
	*
	* @access public
	*/
	function addPreloadImg($file)
	{
		$this->_preload_imgs[] = $file;
	}//end add_img_rollover()


	/**
	* Set the character set for the page
	*
	* @param string $charset the character set to use
	*
	* @access public
	*/
	function setCharset($charset)
	{
		$this->_charset = $charset;
	}//end setCharset()


	/**
	* Opens a section of the page for raw output
	*
	* @access public
	*/
	function openRaw()
	{
		$init_arr = Array(
						'type'		=> 'raw',
						'contents'	=> ''
					);

		$ret_val = $this->_openSection($init_arr);
		ob_start();
		$this->_raw_opened = true;
		return $ret_val;

	}//end openRaw()


	/**
	* Closes a raw output section
	*
	* @access public
	*/
	function closeRaw()
	{
		if (!$this->_raw_opened) return;
		$stack_i = count($this->_section_stack) - 1;
		$this->_section_stack[$stack_i]['contents'] = ob_get_contents();
		ob_end_clean();
		$this->_raw_opened = false;
		$this->_closeSection();
	}//end closeRaw()


	/**
	* Opens a section
	*
	* @param string		$heading
	* @param string		$icon
	*
	* @access public
	*/
	function openSection($heading='', $icon='')
	{
		$init_arr = Array(
						'area_type'	=> 'section',
						'type'		=> 'normal',
						'heading'	=> $heading,
						'icon'		=> $icon,
						'areas'		=> Array(),
					);
		return $this->_openSection($init_arr);
	}// end openSection()


	/**
	* Closes a section
	*
	* @access public
	*/
	function closeSection()
	{
		if (!is_null($this->_current_field)) $this->closeField();
		$this->_closeSection();
	}


	/**
	* Opens a section, with the passed initialise array
	*
	* @param Array		$init_arr
	*
	* @access private
	*/
	function _openSection($init_arr)
	{
		if (!is_null($this->_current_field)) $this->closeField();
		if ($this->_raw_opened) $this->closeRaw();

		$this->_section_count++;
		$init_arr['section_count'] = $this->_section_count;

		// Nothing in the stack ? we are in the top level
		if (empty($this->_section_stack)) {
			$j = count($this->_contents);
			$this->_contents[$j] = $init_arr;
			$this->_section_stack[] = &$this->_contents[$j];
		} else {
			$i = count($this->_section_stack) - 1;
			$j = count($this->_section_stack[$i]['areas']);
			$this->_section_stack[$i]['areas'][$j] = $init_arr;
			$this->_section_stack[] = &$this->_section_stack[$i]['areas'][$j];
		}

		return 'section_'.$this->_section_count;

	}// end _openSection()


	/**
	* Closes a section
	*
	* @access private
	*/
	function _closeSection()
	{
		array_pop($this->_section_stack);
	}


	/**
	* Opens a field inside a section
	*
	* @param string $name		label of the field
	* @param string $format		format 'new_line' | 'commit' | ''
	* @param string $note		a note to place with the field
	*
	* @access public
	*/
	function openField($name, $format='', $note='')
	{
		if (!is_null($this->_current_field)) $this->closeField();
		if (empty($this->_section_stack)) $this->openSection();
		if (!trim($name)) $name = '&nbsp;';

		$this->_field_count++;

		$stack_i = count($this->_section_stack) - 1;
		$i = count($this->_section_stack[$stack_i]['areas']);
		$this->_section_stack[$stack_i]['areas'][$i] = Array(
															'area_type'		=> 'field',
															'field_count'	=> $this->_field_count,
															'name'			=> $name,
															'format'		=> $format,
															'note'			=> $note,
															'contents'		=> '',
														);

		$this->_current_field = &$this->_section_stack[$stack_i]['areas'][$i];
		ob_start();

		return 'field_'.$this->_field_count;

	}// end openField()


	/**
	* Closes a field
	*
	* @access public
	*/
	function closeField()
	{
		if (is_null($this->_current_field)) return;
		$this->_current_field['contents'] = ob_get_contents();
		ob_end_clean();
		// unset the reference before setting to null, so we don't blow away the field's array
		unset($this->_current_field);
		$this->_current_field = null;

	}//end closeField()


	/**
	* Returns the path to the web dir in lib
	*
	* @param string $file  if supplied is concatinated onto the path before returning
	*
	* @return string
	* @access public
	*/
	function filesPath($file='')
	{
		if ($file && substr($file, 0, 1) != '/') $file = '/'.$file;
		return $this->_files_path.$file;
	}


	/**
	* paint the cached backend interface
	*
	* @access public
	*/
	function paint()
	{
		if ($this->_raw_opened)     $this->closeRaw();
		// close all sections
		while(!empty($this->_section_stack)) $this->closeSection();

		if ($this->_redirect) {
			// save the messages so the come through on the next page
			$_SESSION['backend_outputter_msgs'] = $this->_messages;
			$this->addOnLoad($this->_redirect_target.'.location = "'.addslashes($this->_redirect).'";');
		}

		$this->_paintHeader();

		if (!$this->_redirect) {
			for($i = 0; $i < count($this->_contents); $i++) {
				$this->_paintSection($this->_contents[$i]);
			}
		}// end if

		$this->_paintFooter();

	}//end paint();


	/**
	* Returns the outputters painted contents as a string
	*
	* @access public
	*/
	function getAsString()
	{
		ob_start();
		$this->paint();
		$str = ob_get_contents();
		ob_end_clean();
		return $str;

	}//end getAsString()


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

				function form_on_submit() {
			<?php
				foreach($this->_on_submit_calls as $call) {
				?>
					<?php echo $call;?>;
				<?php
				}//end foreach
			?>
					// basically if they get this far then we can submit
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
		foreach($this->_hidden_fields as $name => $value) {
			hidden_field($name, $value);
		}//end foreach

		if ($this->_heading || count($this->_screens) || count($this->_static_screens)) {
			?>
			<table width="100%" cellpadding="3" cellspacing="0" border="0">
				<tr>
					<td class="sq-backend-main-heading" valign="middle"><img src="<?php echo sq_web_path('lib'); ?>/web/images/blank.gif" width="5" height="1"><?php echo $this->_heading;?></td>
					<td class="sq-backend-main-heading" style="text-align: right;">
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
		<?php

			// if this ain't the last section, add a spacer
//			if ($i < count($this->_contents) - 1) {
			?>
				<table width="100%" cellpadding="0" cellspacing="0" border="0"><tr><td width="100%"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td></tr></table>
			<?php
//			}// end if

		}//end if section type == raw

	}//end _paintSection()


	/**
	* Paint the footer of the page
	*
	* @access private
	*/
	function _paintFooter()
	{
		?>
		</form>
		</body>
		</html>
		<?php

	}//end _paintFooter()

//	 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	// just paints a row for the output table in paint() above
//	function _paintMessages($type, $msgs)
//	{
//		if (!count($msgs)) return;
//
//		$class = '';
//		$html  = '<table border="0" cellspacing="0" cellpadding="0" width="80%">';
//
//		// the notice, warning and error messages will be listed
//		if ($type == SQ_MSG_INFORMATION) {
//
//			$html .= '<tr><td class="'.$class.'" valign="top">&nbsp;</td></tr>';
//			$html .= '<tr><td class="'.$class.'" align="center" valign="top"><p>'.implode('<p>', $msgs).'</td></tr>';
//			$html .= '<tr><td class="'.$class.'" valign="top">&nbsp;</td></tr>';
//			$class = 'msg-information';
//
//		} else {
//
//			$class = '';
//			switch($type) {
//				case SQ_MSG_NOTICE:		$class = 'msg-notice';		break;
//				case SQ_MSG_WARNING:	$class = 'msg-warning';		break;
//				default:				$class = 'msg-error';
//			}
//			foreach($msgs as $s) {
//				$html .= '<tr><td class="'.$class.'" valign="top"><li>&nbsp;</td><td class="'.$class.'" valign="top" width="100%">'.$s.'</td></tr>';
//			}
//
//		}//end if
//
//		$html .= '</table>';
//		$this->_paintRow($html, $class);
//
//	}//end _paintMessages()


	/**
	* Outputs a stylesheet include line for the passed file
	*
	* @access public
	*/
	function _paintCssInclude($file)
	{
		?>
			<link rel="stylesheet" type="text/css" href="<?php echo $file;?>" />
		<?php

	}//end _paintCssInclude()


	/**
	* Outputs a javascript include line for the passed file
	*
	* @access public
	*/
	function _paintJsInclude($file)
	{
		?>
			<script type="text/javascript" language="javascript" src="<?php echo $file;?>"></script>
		<?php

	}//end _paintJsInclude()


	/**
	* Outputs a note in the proper css format
	*
	* @access public
	*/
	function note($str)
	{
		echo '<div class="sq-backend-smallprint">'.$str.'</div>';

	}//end note()


	/**
	* Creates a standardised commit button
	*
	* @access public
	*/
	function commitButton($value='Commit', $release_lock=1)
	{
		$release_lock = (int)$release_lock;
		$this->openSection(' ');
			$this->openField('', 'commit');
				hidden_field('sq_release_lock', 0);
				hidden_field('sq_release_lock_on_submit', $release_lock);
				hidden_field('sq_submit_pressed', 0);
				normal_button('sq-commit-button', $value, 'sqSubmitEditForm()');
			$this->closeField();
		$this->closeSection();

	}//end commitButton()


	/**
	* Event handler to refresh the asset map if a new link has been created
	*
	* @return void
	* @access public
	*/
	function onCreateLink($event_data=Array())
	{
		if (!in_array($event_data['assetid'], $this->_refreshing_assets)) {
			$this->addOnLoad('if (top.sidenav && top.sidenav.reload_asset) top.sidenav.reload_asset("'.$event_data['assetid'].'");');
			$this->_refreshing_assets[] = $event_data['assetid'];
		}

	}//end onCreateLink()


	/**
	* Event handler to refresh the asset map if a link has been deleted
	*
	* @return void
	* @access public
	*/
	function onDeleteLink($event_data=Array())
	{
		if (!in_array($event_data['assetid'], $this->_refreshing_assets)) {
			$this->addOnLoad('if (top.sidenav && top.sidenav.reload_asset) top.sidenav.reload_asset("'.$event_data['assetid'].'");');
			$this->_refreshing_assets[] = $event_data['assetid'];
		}

	}//end onDeleteLink()


	/**
	* Event handler to refresh the asset map if the short name of an asset has changed
	*
	* @return void
	* @access public
	*/
	function onAssetUpdate($event_data=Array())
	{
		if (!in_array($event_data['assetid'], $this->_refreshing_assets)) {
			if ($event_data['old_short_name'] != $event_data['new_short_name']) {
				$this->addOnLoad('if (top.sidenav && top.sidenav.reload_asset) top.sidenav.reload_asset("'.$event_data['assetid'].'");');
				$this->_refreshing_assets[] = $event_data['assetid'];
			}
		}

	}//end onAssetUpdate()


	/**
	* Event handler to refresh the asset map if an asset has been morphed to a new type
	*
	* @return void
	* @access public
	*/
	function onAssetTypeUpdate($event_data=Array())
	{
		if (!in_array($event_data['assetid'], $this->_refreshing_assets)) {
			if ($event_data['old_type'] != $event_data['new_type']) {
				$this->addOnLoad('if (top.sidenav && top.sidenav.reload_asset) top.sidenav.reload_asset("'.$event_data['assetid'].'");');
				$this->_refreshing_assets[] = $event_data['assetid'];
			}
		}

	}//end onAssetTypeUpdate()

}//end class

?>