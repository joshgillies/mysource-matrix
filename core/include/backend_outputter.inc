<?php 

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

// FYI notices, saying 'take notice' novice users
define('SQ_BO_MSG_INFORMATION', 0);
// just normal messages, eg update confirmations
define('SQ_BO_MSG_NOTICE',      1);
// warnings, you entered something a bit a-miss and the system fixed it up for you
define('SQ_BO_MSG_WARNING',     2);
// something is just plain wrong, fix immediately
define('SQ_BO_MSG_ERROR',       3);


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
* @package Resolve
*/
class Backend_Outputter 
{

	// any messages to be displayed to the user
	var $_messages = Array();

	// an link to redirect to when we print, any messages will be carried over to the next page
	var $_redirect = '';

	// an array of fns to be called when the page loads
	var $_on_load_calls = Array();

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
	var $_current_screen = '';

	// the contents of the page split into sections
	var $_contents = Array();

	// used to make sure that each img gets a unique name
	var $_img_button_count = 0;

	// all the preload images names on the page and their preloaded images
    // Array(name => img);
	var $_preload_imgs = Array();

	var $_files_path;

	var $_raw_opened      = false;
	var $_section_opened  = false;
	var $_section_count = 0;
	var $_field_opened  = false;
	var $_field_count = 0;

	function Backend_Outputter() 
	{

		// if there are some messages from the previous page we had better add them in
		if (!empty($_SESSION['backend_outputter_msgs'])) {
			$this->_messages = $_SESSION['backend_outputter_msgs'];
			$_SESSION['backend_outputter_msgs'] = Array();
		}//end if

		$this->_files_path = SQ_WWW_LIB_DIR.'/web';
		$this->_action = $_SERVER['PHP_SELF'];


	}

	 ////////////////////////////////////////////////////////
	// set the redirect for page
	function setRedirect($link) 
	{
		$this->_redirect = trim($link);
	}//end setRedirect()


	/**
	* add an onLoad call, something JS that needs to be executed right 
	* after the page finishes loading
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
	* allows you to add get vars to the action section of the form
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
	* returns the current action, which realistically is going to be the 
	* way to get to the current location
	*
	* @access public
	*/
	function getCurrentLocation() 
	{
		return $this->_action;
	}//end getCurrentLocation()


	 ////////////////////////////////////////////////////////
	// set the anchor that the page will submit to
	function setFormAnchor($anchor) 
	{
		$anchor = trim($anchor);
		$this->_anchor = ($anchor) ? '//'.$anchor : '';
	}//end setFormAnchor()

	/**
	* set the heading for page
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
	* set the sub heading for page
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

	 ////////////////////////////////////////////////////////////////////////////////////////
	// add a message to be displayed to the user
	function addMessage($type, $msg) 
	{
		$msg = trim($msg);
		// if the type is valid and there is a message
		if(strlen($msg)) {
			$this->_messages[] = Array('type' => $type, 'msg' => $msg);
		}
	}//end addMessage()
	
	 ////////////////////////////////////////////////////////////////////////////////////////
	// add a stylesheet file to the page
	function addCssInclude($file) 
	{
		if (in_array($file, $this->_css_includes)) return;
		$this->_css_includes[] = $file;
	}//end addCssInclude()

	 ////////////////////////////////////////////////////////////////////////////////////////
	// add a javascript file to the page
	function addJsInclude($file) 
	{
		if (in_array($file, $this->_js_includes)) return;
		$this->_js_includes[] = $file;
	}//end addJsInclude()

	 ////////////////////////////////////////////////////////////////////////////////////////
	// add an onSubmit call, something JS that
    // needs to be executed before the form submits
	function addOnSubmit($call) 
	{
		$this->_on_submit_calls[] = $call;
	}//end addOnSubmit()

	 ////////////////////////////////////////////////////////////////////////////////////////
	// add a hidden field to the output
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
	* Set's the current screen, so the combo displays right
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


	 ////////////////////////////////////////////////////////////////////////////////////////
	// adds a preload image
	function addPreloadImg($file) 
	{
		$this->_preload_imgs[] = $file;
	}//end add_img_rollover()

	/**
	* Opens a section of the page for raw output
	*
	* @access public
	*/
	function openRaw() 
	{
		if ($this->_section_opened) $this->closeSection();
		if ($this->_raw_opened) $this->closeRaw();

		$this->_section_count = count($this->_contents);

		$this->_contents[$this->_section_count] = Array('type'    => 'raw',
														'contents' => '');
		ob_start();
		$this->_raw_opened = true;

	}//end openRaw()

	/**
	* Closes a raw output section
	*
	* @access public
	*/
	function closeRaw() 
	{
		if (!$this->_raw_opened) return;
		$this->_contents[$this->_section_count]['contents'] = ob_get_contents();
		ob_end_clean();
		$this->_raw_opened = false;
		
	}//end closeRaw()


	/**
	* Opens a section
	*
	* @param string $heading
	* @param string $icon
	* @param string $bg_colour
	*
	* @access public
	*/
	function openSection($heading='', $icon='')
	{
		if ($this->_raw_opened) $this->closeRaw();
		if ($this->_section_opened) $this->closeSection();
		if (!trim($heading)) $heading = '&nbsp;';
		$this->_section_count = count($this->_contents);

		$this->_contents[$this->_section_count] = Array('type'    => 'normal',
														'heading' => $heading,
														'icon'    => $icon,
														'fields'  => Array());
		$this->_section_opened = true;
		return 'section_'.$this->_section_count;
	}

	/**
	* Closes a section
	*
	* @access public
	*/
	function closeSection() 
	{
		if ($this->_field_opened) $this->closeField();
		$this->_section_opened = false;
	}

	/**
	* Opens a field inside a section
	*
	* @param string $heading
	* @param string $icon
	*
	* @access public
	*/
	function openField($name, $format='', $note='') 
	{
		if ($this->_field_opened)    $this->closeField();
		if (!$this->_section_opened) $this->openSection();
		if (!trim($name)) $name = '&nbsp;';
		$this->_field_count = count($this->_contents[$this->_section_count]['fields']);


		$this->_contents[$this->_section_count]['fields'][$this->_field_count] = Array('name' => $name,
																						'format' => $format,
																						'note' => $note,
																						'contents' => '');

		ob_start();
		$this->_field_opened = true;
	}

	/**
	* Closes a field
	*
	* @access public
	*/
	function closeField() 
	{
		if (!$this->_field_opened) return;
		$this->_contents[$this->_section_count]['fields'][$this->_field_count]['contents'] = ob_get_contents();
		ob_end_clean();
		$this->_field_opened = false;
	}

	/**
	* returns the path to the web dir in lib
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
		if ($this->_section_opened) $this->closeSection();

		if ($this->_redirect) {
			// save the messages so the come through on the next page
			$_SESSION['backend_outputter_msgs'] = $this->_messages;
			$this->addOnLoad('_self.location = "'.$this->_redirect.'";');
		}

		$this->_paintHeader();

		if (!$this->_redirect) {
			$this->_paintContents();
		}// end if

		$this->_paintFooter();
		
	}//end paint();

	/**
	* returns the outputters painted contents as a string
	*
	* @access public
	*/
	function getAsString() 
	{
		if ($this->_raw_opened)     $this->closeRaw();
		if ($this->_section_opened) $this->closeSection();

		ob_start();
		$this->paint();
		$str = ob_get_contents();
		ob_end_clean();
		return $str;
		
	}//end getAsString()

	/**
	* paint the header of the page
	*
	* @access private
	*/
	function _paintHeader() 
	{

	?> 
		<html>
		<head>
			<title><?php echo $this->_heading;?></title>
			<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
		<?php
			$this->_paintCssInclude($this->filesPath('css/edit.css'));
			foreach($this->_css_includes as $file) {
				$this->_paintCssInclude($file);
			}//end foreach

			$this->_paintJsInclude(SQ_WWW_LIB_DIR.'/html_form/html_form.js');
			$this->_paintJsInclude(SQ_WWW_LIB_DIR.'/js/general.js');
			$this->_paintJsInclude(SQ_WWW_LIB_DIR.'/js/debug.js');
			//$this->_paintJsInclude($this->filesPath('/js/general.js'));
			//$this->_paintJsInclude($this->filesPath('/js/edit.js'));
			//$this->_paintJsInclude(SQ_WWW_LIB_DIR.'/js/general.js');
			foreach($this->_js_includes as $file) {
				$this->_paintJsInclude($file);
			}//end foreach
		?>

			<script language="Javascript" type="text/javascript">
			<!--

				function page_on_load() {
			<?php
				foreach($this->_on_load_calls as $call) {
				?>
					<?php echo $call;?>;
				<?php
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
		<body bgcolor="//ffffff" onLoad="javascript: page_on_load();" marginwidth="0" marginheight="0" topmargin="0" leftmargin="0">
		<form action="<?php echo $this->_action.$this->_anchor;?>" name="main_form" method="post" enctype="multipart/form-data" onSubmit="javascript: return form_on_submit();">
	<?php
		foreach($this->_hidden_fields as $name => $value) {
			$this->hidden_field($name, $value);
		}//end foreach
//	>
//
//		<table width="100%" height="100%" border="0" cellpadding="0" cellspacing="0">
//			<tr> 
//				<td valign="top" align="center">
//					<table border="0" cellpadding="1" cellspacing="0" width="95%">
//	<
//		if ($this->_show_crumming && $this->_crumming) {
//			$this->_paintRow(implode(' &gt; ', $this->_crumming), 'crumming', 'left');
//		}
//
//		foreach($this->_messages as $type => $msgs) {
//			$this->_paintMessages($type, $msgs);
//		}//end foreach
//		
//		if ($this->_heading) {
//			$this->_paintRow($this->_heading, 'page-heading');
//		}
//
		if (count($this->_messages)) {
			pre_echo("MESSAGES : ");
			pre_echo($this->_messages);
		}

		if ($this->_heading || count($this->_screens)) {
			?>
			<table width="100%" cellpadding="3" cellspacing="0" border="0">
				<tr>
					<td class="sq-backend-heading">
					<?php if ($this->_heading_icon) { ?>
						<img src="<?php echo $this->filesPath('/images/icons/'.$this->_heading_icon.'.gif');?>" width="20" height="20" border="0" align="left" alt="<?php echo $this->_heading;?>" />
					<?php } ?>
						<?php echo $this->_heading;?>
					</td>
					<td class="sq-backend-heading" style="text-align: right;">
						<?php $this->combo_box('', $this->_screens, $this->_current_screen, 'onChange="javascript: self.location = element_value(this);"', 0, 1); ?>
					</td>
				</tr>
			</table>
			<?php 
		}// end if
		
	}//end _paintHeader()

	/**
	* paint the contents of the page
	*
	* @access private
	*/
	function _paintContents() 
	{

		for($i = 0; $i < count($this->_contents); $i++) {

			if ($this->_contents[$i]['type'] == 'raw') {
				echo $this->_contents[$i]['contents'];
			} else {
		
			?>
				<a name="section_<?php echo $i;?>"></a>
				<table width="100%" cellpadding="3" cellspacing="0" border="0">
					<tr>
						<td class="sq-backend-sectionheading" colspan="2">
							<?php if ($this->_contents[$i]['icon']) { ?>
								<img src="<?php echo $this->filesPath('images/icons/'.$this->_contents[$i]['icon'].'.gif');?>" width="20" height="20" border="0" alt="<?php echo $this->_contents[$i]['heading'];?>" align="left" />
							<?php } ?>
							<span ><?php echo $this->_contents[$i]['heading'];?></span>
						</td>
					</tr>
			<?php
				for($j = 0; $j < count($this->_contents[$i]['fields']); $j++) {
					$field = &$this->_contents[$i]['fields'][$j];
					switch ($field['format']) {
						case 'new_line' :
							?>
								<tr>
									<td nowrap valign="top" class="sq-backend-field" colspan="2">
										<a name="field_<?php echo $i.'_'.$j;?>"></a>
										<?php echo $field['name'];?>
									</td>
								</tr>
								<tr>
									<td valign="top" class="sq-backend-data" colspan="2">
										<table cellpadding="5" cellspacing="0" border="0">
											<tr>
												<td width="5"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="5" height="1"/></td>
												<td width="100%">
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

						default :
						?>
							<tr>
								<td nowrap valign="top" class="sq-backend-field">
									<a name="field_<?php echo $i.'_'.$j;?>"></a>
									<?php echo $field['name'];?>
								</td>
								<td width="100%" valign="top" class="sq-backend-data">
								<?php
									echo $field['contents'];
									if ($field['note']) $this->note($field['note']);
								?>
								</td>
							</tr>
						<?php
					}//end switch

				}//end for

				// if this ain't the last section, add a spacer
				if ($i < count($this->_contents) - 1) {
				?> 
					<tr><td class="sq-backend-field" colspan="2">&nbsp;</td></tr>
				<?php
				}// end if
			?>	
				</table>
				<table width="100%" cellpadding="0" cellspacing="0" border="0"><tr><td width="100%" background="<?php echo $this->filesPath('images/section_dots.gif');?>"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="1" /></td></tr></table>
			<?php

			}//end if

		}//end for

	}//end _paintContents()

	/**
	* paint the footer of the page
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

	 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// outputs a stylesheet include line for the passed file
	function _paintCssInclude($file) 
	{
		?> 
			<link rel="stylesheet" type="text/css" href="<?php echo $file;?>" />
		<?php
	}//end _paintCssInclude()

	 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// outputs a javascript include line for the passed file
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
	function commitButton($value='Commit') {
		$this->openField('', 'commit');
		$this->submit_button('commit', $value);
	}


	/**
	* All these function are wrappers for the main fns just adding any backend specific calls
	*
	* @see html_form.inc
	* @access public
	*/
	function submit_button($name, $value, $on_click='', $extras='') {
		submit_button($name, $value, $on_click, 'class="sq-backend-data" '.$extras);
	}
	function warning_button($name, $value, $on_click='', $warning='', $extras='') {
		warning_button($name, $value, $on_click, $warning, 'class="sq-backend-data" '.$extras);
	}
	function normal_button($name, $value, $on_click='', $extras='') {
		normal_button($name, $value, $on_click, 'class="sq-backend-data" '.$extras);
	}
	function hidden_field($name, $value) {
		hidden_field($name, $value);
	}
	function text_box($name, $value, $size='', $max='', $extras='') {
		text_box($name, $value, $size, $max, 'class="sq-backend-data" '.$extras);
	}
	function password_box($name, $value='', $size='', $max='', $extras='') {
		password_box($name, $value, $size, $max, 'class="sq-backend-data" '.$extras);
	}
	function text_area($name, $contents, $cols=0, $rows=0, $max=0, $extras='') {
		text_area($name, $contents, $cols, $rows, $max, 'class="sq-backend-data" '.$extras);
	}
	function radio_button($name, $value='1', $checked=false, $on_click='', $extras='') {
		radio_button($name, $value, $checked, $on_click, 'class="sq-backend-data" '.$extras);
	}
	function check_box($name, $value='1', $checked=false, $on_click='', $extras='') {
		check_box($name, $value, $checked, $on_click, 'class="sq-backend-data" '.$extras);
	}
	function combo_box($name, $contents, $default, $extras='', $maxlength=0, $height=0) {
		combo_box($name, $contents, $default, 'class="sq-backend-data" '.$extras, $maxlength, $height);
	}
	function multiple_combo_box($name, $contents, $defaults, $extras, $maxlength, $height) {
		multiple_combo_box($name, $contents, $defaults, 'class="sq-backend-data" '.$extras, $maxlength, $height);
	}
	function date_box($name, $default_timestamp=0, $show_time=true, $extras='') {
		date_box($name, $default_timestamp, $show_time, 'class="sq-backend-data" '.$extras);
	}
	function time_box($hours_name, $hours, $mins_name, $mins, $extras='') {
		time_box($hours_name, $hours, $mins_name, $mins, 'class="sq-backend-data" '.$extras);
	}
	function file_upload($name, $extras='') {
		file_upload($name, 'class="sq-backend-data" '.$extras);
	}
	function commit_file_upload($file, $dest, $overwrite=false) {
		commit_file_upload($file, $dest, $overwrite);
	}

}//end class


?>
