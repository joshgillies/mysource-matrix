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
* $Id: html_form.inc,v 1.50.2.4 2004/03/26 16:41:50 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* HTML Form Functions
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage lib
*/


/**
* Prints a submit button
*
* @param string	$name		the form name to use for the submit button
* @param string	$value		the text that should be displayed for the submit button
* @param string	$on_click	any JS that should be executed when the submit button is pressed
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function submit_button($name, $value, $on_click='', $extras='')
{
	// Make sure that they don't use 'submit' as the form field, because it clashes
	// with the submit() fn that the form has
	if ($name == 'submit') trigger_error('Rename your Submit button, so as not use "submit"', E_USER_ERROR);
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
	echo '<input type="submit" name="', htmlspecialchars($name), '" id="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" ', $extras, ' />';

}//end submit_button()


/**
* Prints a normal form button
*
* @param string	$name		the form name to use for the submit button
* @param string	$value		the text that should be displayed for the submit button
* @param string	$on_click	any JS that should be executed when the user confirms after the warning
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function normal_button($name, $value, $on_click='', $extras='')
{
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
	echo '<input type="button" name="', htmlspecialchars($name), '" id="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" ', $extras, ' />';

}//end normal_button()


/**
* Prints a hidden field
*
* @param string	$name		the form name to use for the hidden field
* @param string	$value		the text that should stored in the hidden field
*
* @return void
* @access public
*/
function hidden_field($name, $value='')
{
	echo '<input type="hidden" name="', htmlspecialchars($name), '" id="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" />';

}//end hidden_field()


/**
* Prints a simple single line text box
*
* @param string		$name			the form name to use for the text box
* @param string		$value			the text that should be pre-entered in the text box
* @param int		$size			the width of the text box
* @param int		$max			the maximum number of characters allowed in the text box
* @param boolean	$hide_value		use the default value as indicative text only (hide it onFocus and restore onBlur
*									if needed) - useful if you want to use it as instructional text (as required by priority
*									level 3 accessability standards)
* @param string		$extras			any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function text_box($name, $value, $size='', $max='', $hide_value=false, $extras='')
{
	$size = (int) $size;
	$max  = (int) $max;
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
	echo	'<input type="text" ',
			'id="', htmlspecialchars($name), '" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($size)   ? 'size="'.$size.'" '     : ''),
			(($max)    ? 'maxlength="'.$max.'" ' : ''),
			(($hide_value) ? 'onfocus="Javascript: if (this.value == \''.htmlspecialchars($value).'\') { this.value = \'\'; }" onblur="Javascript: if (this.value == \'\') { this.value=\''.htmlspecialchars($value).'\'; }" ' : ''),
			$extras,
			' />';

}//end text_box()


/**
* Prints a password box
*
* @param string	$name		the form name to use for the password box
* @param string	$value		the text that should be pre-entered in the password box
* @param int	$size		the width of the password box
* @param int	$max		the maximum number of characters allowed in the password box
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function password_box($name, $value='', $size='', $max='', $extras='')
{
	$size = (int) $size;
	$max  = (int) $max;
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
	echo	'<input type="password" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($size)   ? 'size="'.$size.'" '     : ''),
			(($max)    ? 'maxlength="'.$max.'" ' : ''),
			$extras,
			' />';

}//end password_box()


/**
* Prints a multi-line text box (text area)
*
* @param string	$name		the form name to use for the text area
* @param string	$contents	the text that should be pre-entered in the text area
* @param int	$cols		the width of the text area
* @param int	$rows		the height (in lines) of the text area
* @param int	$max		the maximum number of characters allowed in the text area
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function text_area($name, $contents, $cols=0, $rows=0, $max=0, $extras='')
{
	$cols = (int) $cols;
	$rows = (int) $rows;
	$max  = (int) $max;
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
	echo	'<textarea name="', htmlspecialchars($name), '" ',
			(($cols)  ? 'cols="'.$cols.'" ' : ''),
			(($rows)  ? 'rows="'.$rows.'" ' : ''),
			(($max)   ? 'onKeyPress="if (this.value.length > '.$max.') { this.value = this.value.substr(0,'.$max.'); alert(\'This field can contain no more than '.$max.' characters.\'); this.focus()}" ' : ''),
			$extras,
			'>',
			htmlspecialchars($contents),
			'</textarea>';

}//end text_area()


/**
* Prints a radio button
*
* @param string	$name		the form name to use for the radio button
* @param string	$value		the value that will come through when the button is selected
* @param bool	$checked	whether the button is checked by default or not
* @param string	$on_click	any JS that should be executed when the submit button is pressed
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function radio_button($name, $value='1', $checked=false, $on_click='', $extras='')
{
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
	echo	'<input type="radio" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($checked) ? 'checked ' : ''),
			$extras,
			' />';

}//end radio_button()


/**
* Prints a check box
*
* @param string	$name		the form name to use for the check box
* @param string	$value		the value that will come through when the box is selected
* @param bool	$checked	whether the button is checked by default or not
* @param string	$on_click	any JS that should be executed when the submit button is pressed
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function check_box($name, $value='1', $checked=false, $on_click='', $extras='')
{
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
	echo	'<input type="checkbox" ',
			'name="', htmlspecialchars($name), '" ',
			'id="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($checked) ? 'checked ' : ''),
			$extras,
			' />';

}//end check_box()


/**
* Prints out a
* Given a name, an array of keys and values, and a default value, prints a combo box
*
* @param string	$name		the form name to use for the combo box
* @param Array	$contents	an array of whose keys are values to be returned when that 
*							option is selected and whose values are the display text for the option
* @param bool	$multiple	whether you can multiple selections can be made or not
* @param mixed	$defaults	if there is only one default can be a string, or if there are 
*							multiple can be an array
* @param int	$height		the height (in lines) of the combo box
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
* @param int	$maxlength	the maximum number of characters displayed for any option, 
*							anything over gets truncated with '...' appended
*
* @return void
* @access public
*/
function combo_box($name, $contents, $multiple, $defaults, $height=0, $extras='', $maxlength=0)
{
	$height    = (int) $height;
	$maxlength = (int) $maxlength;
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';

	if ($multiple) {
		$extras .= ' multiple';
		$name .= '[]';
	}

	if (!is_array($contents)) $contents = Array($contents);
	if (!is_array($defaults)) $defaults = Array($defaults);

	echo	'<select name="', htmlspecialchars($name), '" ',
			(($height > 1)? 'size="'.$height.'" ' : ''),
			$extras,
			'>';

	foreach($contents as $key => $value) {
		if ($maxlength) {
			$value = str_replace('&nbsp;', ' ', $value);  // we don't want a space to take 6 chars do we ?
			if (strlen($value) > $maxlength) $value = substr($value, 0, $maxlength - 3).'...';
		}
		$value = str_replace(' ', '&nbsp;', htmlspecialchars($value));
		echo	'<option value="', htmlspecialchars($key), '"',
				((in_array((string) $key, $defaults)) ? ' selected' : ''),
				'>',
				$value,
				'</option>';
	}
	echo '</select>';

}// end combo_box()


/**
* Prints an entry box for a colour picker, requires javascript and colour_picker script
*
* @param string	$name					the form name to use for the colour box
* @param Array	$default				the colour you want to display to begin with
* @param bool	$allow_blanks			allow no colour to be selected ot not
* @param mixed	$button_text			text on the button that displays the colour picker
* @param int	$show_text_box			show the text box to allow manual entry of a colour
* @param string	$show_current_colour	show a box with the current displayed
* @param int	$show_change_colour		show a tick or cross to indicate if the colour has changed
*
* @return void
* @access public
*/
function colour_box($name, $default='000000', $allow_blanks=false, $button_text='*', $show_text_box=true, $show_current_colour=true, $show_change_colour=true) {

	$default = strtolower($default);
	$default = preg_replace('/^#/', '', $default);

	static $initialised = false;
	if (!$initialised) {
		?>
		<script language="JavaScript" src="<?php echo sq_web_path('lib')?>/html_form/colour_picker.js"></script>
		<script language="JavaScript" type="text/javascript">var colour_change_image_dir='<?php echo sq_web_path('lib');?>/web/images/'</script>
		<?php
		$initialised = true;
	}

	if ($show_current_colour) {
		?>
		<span id="colour_span_<?php echo $name; ?>" style="background-color: <?php echo $default; ?>;">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;
		<?php
	}

	if ($show_text_box) {
		$extras  = ' id="colour_box_'.$name.'" style="font-family: courier;"'
				 . ' onChange="javascript: this.value = check_colour(this.value, '.(($allow_blanks) ? 'true' : 'false').');"';

		if ($show_current_colour) $extras .= ' show_colour_change(\''.addslashes($name).'\');';
		text_box($name, $default, 7, 6, false, $extras);
	} else {
		hidden_box($name, $default);
	}

	normal_button('', " $button_text ",
					  "load_colour_picker(this.form.elements['".addslashes($name)."'],'".sq_web_path('lib')."/html_form')",
					  'style="font-family: courier;"');

	if ($show_change_colour) {
		?>
		<img id="colour_change_<?php echo $name; ?>" name="colour_change_<?php echo $name; ?>" src="<?php echo sq_web_path('lib'); ?>/web/images/cross.gif" width="15" height="15" border="0" align="bottom">
		<?php
	}

}// end colour_box()


/**
*  Takes timestamp and draws a set of selection boxes using it as default
*/
function date_box($name, $default_timestamp=null, $show_time=true, $extras='')
{
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';

	trigger_error('*cough*, date_box has not been fully converted - "Calendar::" references and maybe we should be using the datetime_field ?', E_USER_WARNING);
	return '';

	if(!$default_timestamp) $default_timestamp = Calendar::get_next_time_period();

	// if we aren't showing the time set the timestamp to start of day
	if (!$show_time) $default_timestamp = Calendar::get_day_start($default_timestamp);

	// calc the start and end years
	$default = getdate($default_timestamp);

	$start_year = ($default['year'] < 2002) ? $default['year'] : 2002;
	$end_year   = max($default['year'], $start_year) + 10;


	$extras .= ' onChange="javascript: check_date(\''.$name.'\', '.(($show_time) ? 'true':'false').');"';
	hidden_field($name, $default_timestamp);

	$days = array();
	for ($i = 1; $i <= 31; $i++) $days[$i] = $i;
	combo_box('day_'.$name, $days, $default['mday'], $extras);

	$months = array();
	for ($i = 1; $i <= 12; $i++) $months[$i] = date('M', mktime(0,0,0,$i,1,2000));
	combo_box('month_'.$name, $months, $default['mon'], $extras);

	$years = array();
	for ($i = $start_year; $i <= $end_year; $i++) $years[$i] = $i;
	combo_box('year_'.$name, $years, $default['year'], $extras);

	if($show_time) {

		echo '&nbsp;&nbsp;';
		time_box('hour_'.$name, $default['hours'], 'min_'.$name, $default['minutes'], $extras);

	}//end if

}//end date_box()


/**
* Takes number of hours and minutes and creates a pair of combo boxes
*/
function time_box($hours_name, $hours, $mins_name, $mins, $extras='')
{
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
	trigger_error('*cough*, time_box has not been fully converted - maybe we should be using the datetime_field ?', E_USER_WARNING);
	return '';

	$hours_list = array('0' => '12am', '1' => ' 1am', '2' => ' 2am', '3' => ' 3am', '4' => ' 4am', '5' => ' 5am', '6' => ' 6am', '7' => ' 7am', '8' => ' 8am', '9' => ' 9am', '10' => '10am', '11' => '11am',
				  '12' => '12pm', '13' => ' 1pm', '14' => ' 2pm', '15' => ' 3pm', '16' => ' 4pm', '17' => ' 5pm', '18' => ' 6pm', '19' => ' 7pm', '20' => ' 8pm', '21' => ' 9pm', '22' => '10pm', '23' => '11pm');
	combo_box($hours_name, $hours_list, $hours, $extras);

	$mins_list = array();
	for($i = 0; $i < 60; $i++) {
		$mins_list[$i] = sprintf(':%02d', $i);
	}
	combo_box($mins_name, $mins_list, $mins, $extras);

}//end time_box()


/**
* Returns a file upload box
* @param string	$name		the form name to use for the file upload box
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function file_upload($name, $extras='')
{
	if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
	echo '<input type="file" name="', $name, '" ', $extras, '>';

}//end file_upload()


/**
* Returns the information for an uploaded file if the upload was successful
*
* @param string	$name		the form name from the file upload box
*
* @return FALSE on error, blank array on no file upload and the $_FILES[$name] if a file was uploaded successfully
*
* @return void
* @access public
*/
function get_file_upload_info($name)
{
	if (empty($_FILES[$name])) return false;

	$nice_name = str_replace('_', ' ', $name);

	// if something went wrong, sorry bad luck
	switch($_FILES[$name]['error']) {

		case UPLOAD_ERR_NO_FILE :
			return Array();
			break;

		case UPLOAD_ERR_INI_SIZE  :
			trigger_error('The uploaded file "'.$nice_name.'" excedded the size specified by the <i>upload_max_size</i> directive in php.ini ('.ini_get('upload_max_size').')', E_USER_WARNING);
			return false;
			break;

		case UPLOAD_ERR_FORM_SIZE :
			trigger_error('The uploaded file "'.$nice_name.'" excedded the MAX_FILE_SIZE directive in form ('.$_POST['MAX_FILE_SIZE'].' bytes)', E_USER_WARNING);
			return false;
			break;

		case UPLOAD_ERR_PARTIAL :
			trigger_error('The uploaded file "'.$nice_name.'" was only partially uploaded.', E_USER_WARNING);
			return false;
			break;

	}//end switch

	if (empty($_FILES[$name]['name']) || !is_uploaded_file($_FILES[$name]['tmp_name'])) return false;

	return $_FILES[$name];

}//end get_file_upload_info()


/**
* Commits an uploaded file to its intended place
*
* @param string	$name			the form name from the file upload box
* @param string	$dest			the destination for the uploaded file, if this is a directory,
*								the file is put in that directory if it is a file, it is written
*								to that file (taking into account $overwrite)
* @param bool	$overwrite		whether to overwrite any existing file or not
* @param int	$max_size		the maximum number of bytes that the uploaded file can be
* @param bool	$restrict_exts	if true extension MUST BE in $extensions,
*								if false must NOT be in $extensions
* @param Array	$extensions		if not empty, the only extensions that are not allowed to be uploaded
*
* @return FALSE on error, blank string on no file upload and the filename (basename) if a file was uploaded
*
* @return void
* @access public
*/
function commit_file_upload($name, $dest, $overwrite=false, $max_size=0, $restrict_exts=true, $extensions=Array())
{
	$info = get_file_upload_info($name);
	if     ($info === false) return false;
	elseif (!count($info))   return '';

	$nice_name = str_replace('_', ' ', $name);

	if ($max_size > 0 && $max_size < $info['size']) {
		trigger_error('The uploaded file "'.$nice_name.'" excedded the '.$max_size.' bytes', E_USER_WARNING);
		return false;
	}

	//// if we get this far the file on the server OK ////

	// Are we worried about the extension ?
	if(count($extensions)) {
		$exts = Array();
		foreach($extensions as $v) $exts[] = strtolower($v);
		$ext = strtolower(substr(strrchr($info['name'],"."), 1));
		$in_arr = in_array($ext, $exts);

		// see $restrict_exts documentation in function header
		if (($restrict_exts && !$in_arr) || (!$restrict_exts && $in_arr)) {
			trigger_error($ext.' files are not allowed for "'.$nice_name.'"', E_USER_WARNING);
			return false;
		}// end if
	}// end if

	//// Let's try and find out what our final destination file is going to be ////

	// Doe dest exist and is it a directory ?
	if (is_dir($dest)) {
		if (!is_writable($dest)) {
			trigger_error('Destination Directory "'.$dest.'" is not writable', E_USER_WARNING);
			return false;
		}
		$dest .= '/'.$info['name'];
	}

	// Now we know we have a proper filename, if it exists are we allowed to write over it
	if (file_exists($dest)) {
		if (!$overwrite) {
			trigger_error('Unable to upload file "'.$info['name'].'", filename already in use', E_USER_WARNING);
			return false;
		}
		if (!is_writable($dest)) {
			trigger_error('Destination File "'.$dest.'" is not writable', E_USER_WARNING);
			return false;
		}

	// the destination doesn't exist
	} else {
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$parent = dirname($dest);
		// if the parent isn't a directory, can we create it ?
		if (!is_dir($parent) && !create_directory($parent)) {
			trigger_error('Parent Directory "'.$parent.'" does not exist', E_USER_WARNING);
			return false;
		} else if (!is_writable($parent)) {
			trigger_error('Parent Directory "'.$parent.'" is not writable', E_USER_WARNING);
			return false;
		}
	}// end dest exists

	/**
	******* Have left this out for now - BCR *******

	// if this filename already exists and we aren't to overwrite, rename it
	if(file_exists($dest . '/' . $name) && !$overwrite) {
		while(file_exists($dest . '/' . $name)) {
			$pos = strrpos($name, '.');
			if(!$pos) $pos = strlen($name);
			$name = increment_name(substr($name,0,$pos)).(substr($name,$pos));
			$thefile_name = $name;
		}
		$this->addMessage(SQ_MSG_NOTICE, 'File name already exists. File renamed to '.$name);
	}
	*/

	// Move file over
	if (!move_uploaded_file($info['tmp_name'], $dest)) {
		trigger_error('Unable to create file: '.$dest, E_USER_WARNING);
		return false;
	}

	return basename($dest);

}//end commit_file_upload()


/**
* Prints an interface for getting an assetid from the flash menu
*
* @param string	$name			the form name to use for the field
* @param string	$assetid		current value for this field
* @param array	$type_codes		Array('[type_code]' => '[flag]') 
*								where flag = (A|D|I) -> Ancestors, Descendants or Only Itself
* @param string	$map_frame		a reference to the map_frame so we can get the asset map
								if this is an empty string, then the asset_finder will look for the asset map
								in the frame where it was loaded
* @param string	$done_fn		a function to call when the asset finder is finished
* @param array	$buttons		an array of which additional buttons to print (clear, reset)
*
* @return void
* @access public
*/
function asset_finder($name, $assetid='', $type_codes=Array(), $map_frame='sq_sidenav', $done_fn='null', $buttons=Array('clear', 'reset'))
{
	$finder_frame = (empty($map_frame)) ? 'parent.top' : 'parent.frames["sq_main"]';
	
	if (!empty($map_frame)) {
		$map_frame = trim('parent.frames["'.$map_frame.'"]', '.');
	} else {
		$map_frame = 'top';
	}
	
	$all_type_codes = Array();
	$am = &$GLOBALS['SQ_SYSTEM']->am;

	foreach($type_codes as $type_code => $flag) {
		$all_type_codes[] = $type_code;
		switch($flag) {
			case 'A' : 
				$ancestors = $am->getTypeAncestors($type_code);
				foreach($ancestors as $ancestor) {
					$all_type_codes[] = $ancestor;
				}
				break;
			case 'D' : 
				$descendants = $am->getTypeDescendants($type_code);
				foreach($descendants as $descendant) {
					$all_type_codes[] = $descendant;
				}
				break;
		}// end switch
	}// end foreach

	$label = '';
	if ($assetid) {
		$asset = &$am->getAsset($assetid);
		if (is_null($asset)) $assetid = 0;
		else $label = $asset->short_name.' (Id : #'.$asset->id.')';
	}

	$safe_name = 'sq_asset_finder_'.preg_replace('/[^A-Za-z0-9]/', '_', $name);

	if (count($all_type_codes) == 1) {
		$type_codes = implode('', $all_type_codes).'|';
	} else {
		$type_codes = implode('|', $all_type_codes);
	}

	hidden_field($name.'[assetid]', $assetid);
	hidden_field($name.'[url]', '');
	// call the js function to set where the finder frame is
	?>
		<script language="javascript" type="text/javascript">
			<?php echo $map_frame; ?>.set_finder(<?php echo $finder_frame; ?>);
		</script>
	<?php

	text_box($safe_name.'_label', $label, 30, 0, false, 'class="sq-form-asset-finder" onFocus="javascript: this.tmp = this.value;" onBlur="javascript: this.value = this.tmp;"');
	normal_button($safe_name.'_change_btn', 'Change', $map_frame.'.asset_finder_change_btn_press(\''.$name.'\', \''.$safe_name.'\', \''.addslashes($type_codes).'\', '.$done_fn.');');
	
	if (in_array('clear', $buttons)) {
		normal_button($safe_name.'_clear_btn', 'Clear', $map_frame.'.asset_finder_clear_btn_press(\''.$name.'\', \''.$safe_name.'\');');
	}
	if (in_array('reset', $buttons)) {
		normal_button($safe_name.'_reset_btn', 'Reset', $map_frame.'.asset_finder_reset_btn_press(\''.$name.'\', \''.$safe_name.'\', \''.$assetid.'\', \''.addslashes($label).'\');');
	}

}//end asset_finder()


/**
* Prints an interface for entering a security key genereted inside an image
* Call validate_security_key() to check if the value entered was correct
*
* @param int		$key_len		the lenght of the random string to display in the image
* @param int		$box_len		the width of the text box used to enter the security key shown
* @param float		$zoom			each character will be randomly zoomed between x1 and this figure
* @param boolean	$img_on_newline	whether the img should appear under the box or not
* @param string		$bg				the background colour of the image (eg FFFFFF)
* @param string		$text			the colour of the text in the image (eg 000000)
* @param string		$border			the colour of the border image (eg 000000)
*									for no border, set to the same colour as the background
*
* @return void
* @access public
*/
function security_key($key_len, $box_len, $zoom=1, $img_on_newline=false, $bg='FFFFFF', $text='000000', $border='000000')
{
	// generate a random security key string
	require_once SQ_FUDGE_PATH.'/general/general.inc';
	$password = generate_security_key($key_len);

	// work out the width and height of the image
	$img_width  = $key_len * 10.5;
	$img_height = 20;

	// we need to store these vars in the session so we can get the
	// system to print the security image for us
	$security_vars = Array(	'key'           => $password,
							'width'         => $img_width,
							'height'        => $img_height,
							'bg_colour'     => $bg,
							'text_colour'   => $text,
							'border_colour' => $border,
							'zoom'          => $zoom
						  );
	$_SESSION['SQ_SYSTEM_SECURITY_KEY'] = $security_vars;

	// generate the url to get the security image to print
	$img_url = $_SERVER['PHP_SELF'].'?SQ_ACTION=send_security_key';
	?>
	<table border="0">
		<tr>
			<td valign="top"><?php echo text_box('SQ_SYSTEM_SECURITY_KEY_VALUE', '', $box_len); ?></td>
	<?php 
	if ($img_on_newline) {
	?>
		</tr>
		<tr>
	<?php
	}// end if
	?>
			<td valign="top"><img id="sq_security_key" src="<?php echo $img_url; ?>" border="0"></td>
		</tr>
		<tr>
			<td class="sq-backend-data" colspan="<?php echo (($img_on_newline) ? '1' : '2'); ?>"><i>Can't read the security key? <a href="#" onClick="Javascript: var img = document.getElementById('sq_security_key'); img.src = '<?php echo $img_url; ?>&regen_key=1'; return false;">Click here to get a new key</a>.</i></td>
		</tr>
	</table>
	<?php

}//end security_key


/**
* Returns true if the security key entered using the interface in security_key() is correct
*
* @return boolean
* @access public
*/
function validate_security_key()
{
	if (!isset($_POST['SQ_SYSTEM_SECURITY_KEY_VALUE'])) return false;
	if (!isset($_SESSION['SQ_SYSTEM_SECURITY_KEY']['key'])) return false;
	$entered_value = $_POST['SQ_SYSTEM_SECURITY_KEY_VALUE'];
	$real_value = $_SESSION['SQ_SYSTEM_SECURITY_KEY']['key'];
	unset($_SESSION['SQ_SYSTEM_SECURITY_KEY']);
	return ($entered_value === $real_value);

}//end validate_security_key()


/**
* Takes a variable and outputs as many hidden fields as possible to recreate it
*
* @param string	$name			the form name to use for the field
* @param int	$assetid		current value for this field
* @param array	$type_codes		Array('[type_code]' => '[flag]')
*								where flag = (A|D|I) -> Ancestors, Descendants or Only Itself
*
* @return void
* @access public
*/
function var_to_hidden_field($var, $name='')
{
	if(is_array($var) || is_object($var)) {
		foreach($var as $k => $v) {
			var_to_hidden_field($v, (($name) ? $name.'['.$k.']': $k));
		}
	} else {
		if($name) hidden_field($name, $var);
	}

}// end var_to_hidden_field()


?>
