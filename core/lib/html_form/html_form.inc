<?php
/**
* HTML Form Functions
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::lib
*/


/**
* Prints a submit button
*
* @param string	$name		the form name to use for the submit button
* @param string	$value		the text that should be displayed for the submit button
* @param string	$on_click	any JS that should be executed when the submit button is pressed
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*/
function submit_button($name, $value, $on_click='', $extras='') 
{
	// Make sure that they don't use 'submit' as the form field, because it clashes
	// with the submit() fn that the form has
	if ($name == 'submit') trigger_error('Rename your Submit button, so as not use "submit"', E_USER_ERROR);
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	echo '<input type="submit" name="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" ', $extras, ' />';
}

/**
* Prints a normal form button
*
* @param string	$name		the form name to use for the submit button
* @param string	$value		the text that should be displayed for the submit button
* @param string	$on_click	any JS that should be executed when the user confirms after the warning
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*/
function normal_button($name, $value, $on_click='', $extras='') 
{
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	echo '<input type="button" name="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" ', $extras, ' />';
}

/**
* Prints a hidden field
*
* @param string	$name		the form name to use for the hidden field
* @param string	$value		the text that should stored in the hidden field
*/
function hidden_field($name, $value='') 
{
	echo '<input type="hidden" name="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" />';
}

/**
* Prints a simple single line text box
*
* @param string	$name		the form name to use for the text box
* @param string	$value		the text that should be pre-entered in the text box
* @param int	$size		the width of the text box
* @param int	$max		the maximum number of characters allowed in the text box
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*/
function text_box($name, $value, $size='', $max='', $extras='') 
{
	$size = (int) $size;
	$max  = (int) $max;
	echo	'<input type="text" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($size)   ? 'size="'.$size.'" '     : ''),
			(($max)    ? 'maxlength="'.$max.'" ' : ''),
			$extras,
			' />';
}


/**
* Prints a password box
*
* @param string	$name		the form name to use for the password box
* @param string	$value		the text that should be pre-entered in the password box
* @param int	$size		the width of the password box
* @param int	$max		the maximum number of characters allowed in the password box
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*/
function password_box($name, $value='', $size='', $max='', $extras='') 
{
	$size = (int) $size;
	$max  = (int) $max;
	echo	'<input type="password" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($size)   ? 'size="'.$size.'" '     : ''),
			(($max)    ? 'maxlength="'.$max.'" ' : ''),
			$extras,
			' />';
}


/**
* Prints a multi-line text box (text area)
*
* @param string	$name		the form name to use for the text area
* @param string	$contents	the text that should be pre-entered in the text area
* @param int	$cols		the width of the text area
* @param int	$rows		the height (in lines) of the text area
* @param int	$max		the maximum number of characters allowed in the text area
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*/
function text_area($name, $contents, $cols=0, $rows=0, $max=0, $extras='') 
{
	$cols = (int) $cols;
	$rows = (int) $rows;
	$max  = (int) $max;
	echo	'<textarea name="', htmlspecialchars($name), '" ',
			(($cols)  ? 'cols="'.$cols.'" ' : ''),
			(($rows)  ? 'rows="'.$rows.'" ' : ''),
			(($max)   ? 'onKeyPress="if (this.value.length > '.$max.') { this.value = this.value.substr(0,'.$max.'); alert(\'This field can contain no more than '.$max.' characters.\'); this.focus()}" ' : ''),
			$extras,
			'>',
			htmlspecialchars($contents),
			'</textarea>';
}

/**
* Prints a radio button
*
* @param string	$name		the form name to use for the radio button
* @param string	$value		the value that will come through when the button is selected
* @param bool	$checked	whether the button is checked by default or not
* @param string	$on_click	any JS that should be executed when the submit button is pressed
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*/
function radio_button($name, $value='1', $checked=false, $on_click='', $extras='') 
{
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	echo	'<input type="radio" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($checked) ? 'checked ' : ''),
			$extras,
			' />';
}

/**
* Prints a check box
*
* @param string	$name		the form name to use for the check box
* @param string	$value		the value that will come through when the box is selected
* @param bool	$checked	whether the button is checked by default or not
* @param string	$on_click	any JS that should be executed when the submit button is pressed
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*/
function check_box($name, $value='1', $checked=false, $on_click='', $extras='') 
{
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	echo	'<input type="checkbox" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($checked) ? 'checked ' : ''),
			$extras,
			' />';
}



/**
* Prints out a 
* Given a name, an array of keys and values, and a default value, prints a combo box
*
* @param string	$name		the form name to use for the combo box
* @param Array	$contents	an array of whose keys are values to be returned when that option is selected and whose values are the display text for the option
* @param bool	$multiple	whether you can multiple selections can be made or not
* @param mixed	$defaults	if there is only one default can be a string, or if there are multiple can be an array
* @param int	$height		the height (in lines) of the combo box
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
* @param int	$max		the maximum number of characters displayed for any option, anything over gets truncated with '...' appended
*/
function combo_box($name, $contents, $multiple, $defaults, $height=0, $extras='', $maxlength=0) 
{
	$height    = (int) $height;
	$maxlength = (int) $maxlength;

	if ($multiple) {
		$extras .= ' multiple';
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
				((in_array($key, $defaults)) ? ' selected' : ''),
				'>',
				$value,
				'</option>';
	}
	echo '</select>';
}

/**
*  Takes timestamp and draws a set of selection boxes using it as default
*/
function date_box($name, $default_timestamp=null, $show_time=true, $extras='') 
{

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
	$this->hidden_field($name, $default_timestamp);

	$days = array();
	for ($i = 1; $i <= 31; $i++) $days[$i] = $i;
	$this->combo_box('day_'.$name, $days, $default['mday'], $extras);

	$months = array();
	for ($i = 1; $i <= 12; $i++) $months[$i] = date('M', mktime(0,0,0,$i,1,2000));
	$this->combo_box('month_'.$name, $months, $default['mon'], $extras);

	$years = array();
	for ($i = $start_year; $i <= $end_year; $i++) $years[$i] = $i;
	$this->combo_box('year_'.$name, $years, $default['year'], $extras);

	if($show_time) {

		echo '&nbsp;&nbsp;';
		$this->time_box('hour_'.$name, $default['hours'], 'min_'.$name, $default['minutes'], $extras);

	}//end if

}//end date_box()

/**
* Takes number of hours and minutes and creates a pair of combo boxes
*/
function time_box($hours_name, $hours, $mins_name, $mins, $extras='') 
{
	trigger_error('*cough*, time_box has not been fully converted - maybe we should be using the datetime_field ?', E_USER_WARNING);
	return '';

	$hours_list = array('0' => '12am', '1' => ' 1am', '2' => ' 2am', '3' => ' 3am', '4' => ' 4am', '5' => ' 5am', '6' => ' 6am', '7' => ' 7am', '8' => ' 8am', '9' => ' 9am', '10' => '10am', '11' => '11am',
				  '12' => '12pm', '13' => ' 1pm', '14' => ' 2pm', '15' => ' 3pm', '16' => ' 4pm', '17' => ' 5pm', '18' => ' 6pm', '19' => ' 7pm', '20' => ' 8pm', '21' => ' 9pm', '22' => '10pm', '23' => '11pm');
	$this->combo_box($hours_name, $hours_list, $hours, $extras);

	$mins_list = array();
	for($i = 0; $i < 60; $i++) {
		$mins_list[$m] = sprintf(':%02d', $i);
	}
	$this->combo_box($mins_name, $mins_list, $mins, $extras);

}//end time_box()


/**
* Returns a file upload box
* @param string	$name		the form name to use for the file upload box
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*/
function file_upload($name, $extras='') 
{
	echo '<input type="file" name="', $name, '" ', $extras, '>';
}//end file_upload()

/**
* Commits an uploaded file to its intended place
*
* @param string	$name			the form name from the file upload box
* @param string	$dest			the destination for the uploaded file, if this is a directory, the file is put in that directory if it is a file, it is written to that file (taking into account $overwrite)
* @param bool	$overwrite		whether to overwrite any existing file or not
* @param mixed	$max_size		the maximum number of bytes that the uploaded file can be
* @param bool	$restrict_exts	if true extension MUST BE in $extensions, if false must NOT be in $extensions
* @param Array	$extensions		if not empty, the only extensions that are not allowed to be uploaded
*
* @return FALSE on error, blank string on no file upload and the filename (basename) if a file was uploaded
*/
function commit_file_upload($name, $dest, $overwrite=false, $max_size='', $dest_filename='', $restrict_ext=true, $extensions=Array()) 
{
	if (empty($_FILES[$name]) || empty($_FILES[$name]['name']) || !is_uploaded_file($_FILES[$name]['tmp_name'])) {
		trigger_error('"'.$name.'" is not a valid file upload', E_USER_WARNING);
		return false;
	}

	$nice_name = str_replace('_', ' ', $name);

	// if something went wrong, sorry bad luck
	switch($_FILES[$name]['error']) {

		case UPLOAD_ERR_NO_FILE :
			return '';
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

		default : 
			if ($max_size && $max_size < $_FILES[$name]['size']) {
				trigger_error('The uploaded file "'.$nice_name.'" excedded the '.$max_size.' bytes', E_USER_WARNING);
				return false;
			}

	}//end switch

	//// if we get this far the file on the server OK ////

	// Are we worried about the extension ?
	if(count($extensions)) {
		$exts = Array();
		foreach($extensions as $v) $exts[] = strtolower($v);
		$ext = strtolower(substr(strrchr($_FILES[$name]['name'],"."), 1));
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
		$dest .= '/'.$_FILES[$name]['name'];
	}

	// Now we know we have a proper filename, if it exists are we allowed to write over it
	if (file_exists($dest)) {
		if (!$overwrite) {
			trigger_error('Unable to upload file "'.$_FILES[$name]['name'].'", filename already in use', E_USER_WARNING);
			return false;
		}
		if (!is_writable($dest)) {
			trigger_error('Destination File "'.$dest.'" is not writable', E_USER_WARNING);
			return false;
		}

	// the destination doesn't exist, does it's parent ?
	} else {
		$parent = dirname($dest);
		if (!is_dir($parent) || !is_writable($parent)) {
			trigger_error('Parent Directory "'.$parent.'" does not exist or is not writable', E_USER_WARNING);
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
	if (move_uploaded_file($_FILES[$name]['tmp_name'], $dest)) {
		trigger_error('Unable to create file: '.$dest, E_USER_WARNING);
		return false;
	}
	
	return basename($dest);

}//end commit_file_upload()

?>