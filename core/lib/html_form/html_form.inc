<?php


/**
* A submit button
*/
function submit_button($name, $value, $on_click='', $extras='') 
{
	// Make sure that they don't use 'submit' as the form field, because it clashes
	// with the submit() fn that the form has
	if ($name == 'submit') trigger_error('Rename your Submit button, so as not use "submit"', E_USER_ERROR);
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	echo '<input type="submit" name="'.htmlspecialchars($name).'" value="'.htmlspecialchars($value).'" '.$extras.' />';
}

/**
* A submit button with a warning message to confirm
*/
function warning_button($name, $value, $on_click='', $warning='', $extras='') 
{
	if (!$warning) $warning = 'Are you sure you want to ?';
	$extras .= ' onClick="javascript: if (confirm(\''.addslashes(htmlspecialchars($warning)).'\')) {'.htmlspecialchars($on_click).'; submit_form(); } "';
	echo '<input type="button" name="'.htmlspecialchars($name).'" value="'.htmlspecialchars($value).'" '.$extras.' />';
}

/**
* A normal button
*/
function normal_button($name, $value, $on_click='', $extras='') 
{
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	echo '<input type="button" name="'.htmlspecialchars($name).'" value="'.htmlspecialchars($value).'" '.$extras.' />';
}

/**
* A hidden field
*/
function hidden_field($name, $value) 
{
	echo '<input type="hidden" name="'.htmlspecialchars($name).'" value="'.htmlspecialchars($value).'" />';
}

/**
* A simple text box
*/
function text_box($name, $value, $size='', $max='', $extras='') 
{
	echo	'<input type="text" '
			.'name="'.htmlspecialchars($name).'" '
			.'value="'.htmlspecialchars($value).'" '
			.(($size)   ? 'size="'.$size.'" '     : '')
			.(($max)    ? 'maxlength="'.$max.'" ' : '')
			.$extras.' />';
}


/**
* A password box
*/
function password_box($name, $value='', $size='', $max='', $extras='') 
{
	echo	'<input type="password" '
			.'name="'.htmlspecialchars($name).'" '
			.'value="'.htmlspecialchars($value).'" '
			.(($size)   ? 'size="'.$size.'" '     : '')
			.(($max)    ? 'maxlength="'.$max.'" ' : '')
			.$extras.' />';
}


/**
* A text area
*/
function text_area($name, $contents, $cols=0, $rows=0, $max=0, $extras='') 
{
	$cols = (int) $cols;
	$rows = (int) $rows;
	$max  = (int) $max;
	echo	'<textarea  '
			.'name="'.htmlspecialchars($name).'" '
			.(($cols)  ? 'cols="'.$cols.'" ' : '')
			.(($rows)  ? 'rows="'.$rows.'" ' : '')
			.(($max)   ? 'onchange="if (value.length > '.$max.') { this.value = this.value.substr(0,'.$max.'); alert(\'This field can contain no more than '.$max.' characters.\');focus()}" ' : '')
			.$extras.'>'
			.htmlspecialchars($contents)
			.'</textarea>';
}

/**
* A simple radio box
*/
function radio_button($name, $value='1', $checked=false, $on_click='', $extras='') 
{
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	echo	'<input type="radio" '
			.'name="'.htmlspecialchars($name).'" '
			.'value="'.htmlspecialchars($value).'" '
			.(($checked) ? 'checked ' : '')
			.$extras.' />';
}

/**
* A simple check box
*/
function check_box($name, $value='1', $checked=false, $on_click='', $extras='') 
{
	if ($on_click) $extras .= ' onClick="javascript: '.htmlspecialchars($on_click).' "';
	echo	'<input type="checkbox" '
			.'name="'.htmlspecialchars($name).'" '
			.'value="'.htmlspecialchars($value).'" '
			.(($checked) ? 'checked ' : '')
			.$extras.' />';
}



/**
* Given a name, an array of keys and values, and a default value, prints a combo box
*/
function combo_box($name, $contents, $default, $extras='', $maxlength=0, $height=0) {
	$height    = (int) $height;
	$maxlength = (int) $maxlength;
	if (!is_array($contents)) $contents = Array($contents);
	echo '<select name="'.htmlspecialchars($name).'" '.(($height > 1)? 'size="'.$height.'" ' : '').$extras.'>';
	reset($contents);
	while(list($key, $value) = each($contents)) {
		if ($maxlength) {
			$value = str_replace('&nbsp;', ' ', $value);  // we don't want a space to take 6 chars do we ?
			if (strlen($value) > $maxlength) $value = substr($value, 0, $maxlength - 3).'...';
		}
		$value = str_replace(' ', '&nbsp;', htmlspecialchars($value));
		$selected = ((string) $key === (string) $default) ? ' selected' : '';
		echo '<option value="'.htmlspecialchars($key).'"'.$selected.'>'.htmlspecialchars($value).'</option>';
	}
	echo '</select>';
}

/**
* Given a name, an array of keys and values, and an array of default values, 
* prints a multiple select combo box 
*/
function multiple_combo_box($name, $contents, $defaults, $extras, $maxlength, $height) {
	// Make sure we are only dealing with an array
	if (!is_array($defaults)) $defaults = array($defaults);

	// if the name doesn't have the square brackets add them as they are needed for multiple selects
	if (!ereg('\[\]$', $name)) $name .= '[]';
	echo '<select name="'.htmlspecialchars($name).'" multiple '.(($height > 1)? 'size="'.$height.'" ' : '').$extras.'>';
	if(is_array($contents)) {
		reset($contents);
		while(list($key, $value) = each($contents)) {
			if ($maxlength) {
				$value = str_replace('&nbsp;', ' ', $value);  // we don't want a space to take 6 chars do we ?
				if (strlen($value) > $maxlength) $value = substr($value, 0, $maxlength - 3).'...';
			}
			$value = str_replace(' ', '&nbsp;',  htmlspecialchars($value));
			$selected = (in_array($key, $defaults)) ? ' selected' : '';
			echo '<option value="'.htmlspecialchars($key).'" '.$selected.'>'.$value.'</option>';
		}
	}
	echo '</select>';
}

/**
*  Takes timestamp and  draws a set of selection boxes using it as default
*/
function date_box($name, $default_timestamp=0, $show_time=true, $extras='') 
{

	trigger_error('*cough*, date_box has not been fully converted ("Calendar::" references)', E_USER_WARNING);
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
*/
function file_upload($name, $extras) 
{
	echo '<input type="file" name="'.$name.'" '.$extras.'>';
}//end file_upload()

/**
* Commits an uploaded file to its intended place
*/
function commit_file_upload($file, $dest, $overwrite=false) 
{

	trigger_error('*cough*, commit_file_upload has not been converted', E_USER_WARNING);
	return '';

	$nice_name = str_replace('_', ' ', $file);
	$msg = '';

	// if something went wrong, sorry bad luck
	switch($_FILES[$file]['error']) {

		case UPLOAD_ERR_INI_SIZE  :
		case UPLOAD_ERR_FORM_SIZE :
			$this->addMessage(SQ_MSG_ERROR, 'The uploaded file "'.$nice_name.'" was only partially uploaded.');
			return '';
		break;

		case UPLOAD_ERR_PARTIAL :
			$this->addMessage(SQ_MSG_ERROR, 'The uploaded file "'.$nice_name.'" was only partially uploaded.');
			return '';
		break;

		case UPLOAD_ERR_NO_FILE :
			return '';
		break;

	}//end switch


	// if we get this far the file is OK

	// create some easy vars
	$file_vars = array('tmp_name', 'name', 'size', 'type');
	for($var = reset($file_vars); $var; $var = next($file_vars)) {
		$$var = $_FILES[$file][$var];
	}

	if (!$name || !is_uploaded_file($tmp_name)) return "";

	// Create the destination directory, if necessary
	$dir_tree = explode('/', $dest);
	$dir_tree_string = $dir_tree[0];
	for($i = 1; $i <= count($dir_tree); $i++) {
		if ($dir_tree_string && !file_exists($dir_tree_string)) {
			if (!mkdir($dir_tree_string, 0770)) {
				$this->addMessage(SQ_MSG_ERROR, 'Could not create directory: '.$dir_tree_string);
				return '';
			}
		}
		$dir_tree_string .= '/' . $dir_tree[$i];

	}//end for

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

	// Move file over
	if (!copy($tmp_name, $dest.'/'.$name)) {
		$this->addMessage(SQ_MSG_ERROR, 'Unable to create file: '.$dest.'/'.$name);
		return '';
	}
	
	return $name;

}//end commit_file_upload()

?>