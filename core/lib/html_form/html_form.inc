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
* $Id: html_form.inc,v 1.166.2.5 2013/06/28 01:05:38 akarelia Exp $
*
*/

/**
* HTML Form Functions
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.166.2.5 $
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
	if ($name == 'submit') {
		trigger_localised_error('SYS0019', E_USER_ERROR);
	}

	if ($on_click) {
		$extras .= ' onclick="'.htmlspecialchars($on_click).' "';
	}

	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	if (!stristr($extras, 'id=')) {
		$extras .= ' id="'.htmlspecialchars($name).'"';
	}

	echo '<input type="submit" name="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" ', $extras, ' />';

}//end submit_button()


/**
* Prints a reset button, used to clear forms
*
* @param string	$name		the form name to use for the reset button
* @param string	$value		the text that should be displayed for the reset button
* @param string	$on_click	any JS that should be executed when the reset button is pressed
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function reset_button($name, $value, $on_click='', $extras='')
{
	// Make sure that they don't use 'reset' as the form field, because it clashes
	// with the reset() fn that the form has
	if ($name == 'reset') {
		trigger_localised_error('SYS0018', E_USER_ERROR);
	}

	if ($on_click) {
		$extras .= ' onclick="'.htmlspecialchars($on_click).' "';
	}
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	if (!stristr($extras, 'id=')) {
		$extras .= ' id="'.htmlspecialchars($name).'"';
	}

	echo '<input type="reset" name="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" ', $extras, ' />';

}//end reset_button()


/**
* Prints a normal form button
*
* @param string	$name		the form name to use for the button
* @param string	$value		the text that should be displayed for the button
* @param string	$on_click	JS to execute when the button is clicked
* @param string	$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function normal_button($name, $value, $on_click='', $extras='')
{
	if ($on_click) {
		$extras .= ' onclick="'.htmlspecialchars($on_click).' "';
	}
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	echo '<input type="button" name="', htmlspecialchars($name), '" id="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" ', $extras, ' />';

}//end normal_button()


/**
* Prints a hidden field
*
* @param string	$name	the form name to use for the hidden field
* @param string	$value	the text that should stored in the hidden field
* @param string	$extras	the extra options to specify
*
* @return void
* @access public
*/
function hidden_field($name, $value='', $extras='')
{
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	if (!stristr($extras, 'id=')) {
		$extras .= ' id="'.htmlspecialchars($name).'"';
	}
	echo '<input type="hidden" name="', htmlspecialchars($name), '" value="', htmlspecialchars($value), '" ', $extras, ' />';

}//end hidden_field()


/**
* Prints a simple single line text box
*
* @param string		$name		the form name to use for the text box
* @param string		$value		the text that should be pre-entered in the text box
* @param int		$size		the width of the text box
* @param int		$max		the maximum number of characters allowed in the text box
* @param boolean	$hide_value	use the default value as indicative text only (hide it onFocus and restore onBlur
*								if needed) - useful if you want to use it as instructional text (as required by priority
*								level 3 accessability standards)
* @param string		$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function text_box($name, $value='', $size='', $max='', $hide_value=FALSE, $extras='')
{
	$size = (int) $size;
	$max  = (int) $max;
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	if (!stristr($extras, 'id=')) {
		$extras .= ' id="'.htmlspecialchars($name).'"';
	}
	if ($hide_value) {
		if (!stristr($extras, 'onfocus')) {
			$extras .= ' onfocus="if (this.value == \''.htmlspecialchars($value).'\') { this.value = \'\'; }"';
		} else {
			$insert_pos = strpos($extras, 'onfocus="') + 8;
			$extras = substr($extras, 0, $insert_pos).'if (this.value == \''.htmlspecialchars($value).'\') { this.value = \'\'; }'.substr($extras, $insert_pos);
		}
		if (!stristr($extras, 'onblur')) {
			$extras .= ' onblur="if (this.value == \'\') { this.value=\''.htmlspecialchars($value).'\'; }"';
		} else {
			$quote_pos = strpos($extras, '"', strpos($extras, 'onblur="') + 8);
			$extras = substr($extras, 0, $quote_pos).';  if (this.value == \'\') { this.value=\''.htmlspecialchars($value).'\'; }'.substr($extras, $quote_pos);
		}
	}

	echo	'<input type="text" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($size)   ? 'size="'.$size.'" '     : ''),
			(($max)    ? 'maxlength="'.$max.'" ' : ''),
			$extras,
			' />';

}//end text_box()


/**
* Prints a single line text box that only accepts integers
*
* @param string		$name			the form name to use for the text box
* @param string		$value			the text that should be pre-entered in the text box
* @param boolean	$allow_negative	whether negative values are allowed
* @param int		$size			the width of the text box
* @param int		$range_min		the minimum number of range
* @param int		$range_max		the maximum number of range
* @param int		$max_chars		the maximum number of characters allowed in the text box
* @param boolean	$hide_value		use the default value as indicative text only (hide it onFocus and restore onBlur
*									if needed) - useful if you want to use it as instructional text (as required by priority
*									level 3 accessability standards)
* @param boolean	$allow_empty	allow the text box to be empty?
* @param string		$extras			any input field attributes that should be placed in the <input ... > tag
* @param boolean		$include_js			include js file
* @param boolean		$use_js			use js validation
*
* @return void
* @access public
*/
function int_text_box($name, $value, $allow_negative=TRUE, $size='', $range_min=NULL, $range_max=NULL, $max_chars='', $hide_value=FALSE, $allow_empty=FALSE, $extras='', $include_js=TRUE, $use_js=TRUE)
{
	if($use_js && $include_js) {
	?><script type="text/javascript" src="<?php echo sq_web_path('lib')?>/html_form/html_form.js"></script><?php
	}
	$allow_negative = $allow_negative ? 'true' : 'false';

	// ranges
	if (!is_null($range_min) && !is_null($range_max)) {
		if (strlen($value) != 0) {
			if ($value >= $range_min) {
				if ($value > $range_max) $value = $range_max;
			} else {
				$value = $range_min;
			}
		}

		$ranges = Array(
					$range_min,
					$range_max,
				  );
	} else {
		$ranges = Array();
	}

	$ranges_js  = implode(', ', $ranges);
	$javascript = '';
	if($use_js) {
	    $javascript = '
		    onkeyup="validate_numeric_text_field(this, '.$allow_negative.(!empty($ranges_js) ? ', '.$ranges_js : '').');"
		    onblur="validate_numeric_range(this'.(!empty($ranges_js) ? ', '.$ranges_js : '').', '.($allow_empty ? 'true' : 'false').');"
	    ';
	    if (!$hide_value) {
		    $javascript .= '	onchange="validate_numeric_text_field(this, '.$allow_negative.(!empty($ranges_js) ? ', '.$ranges_js : '').');"';
	    }
	}
	text_box($name, $value, $size, $max_chars, $hide_value, $javascript.$extras);

}//end int_text_box()


/**
* Prints a password box
*
* @param string	$name	the form name to use for the password box
* @param string	$value	the text that should be pre-entered in the password box
* @param int	$size	the width of the password box
* @param int	$max	the maximum number of characters allowed in the password box
* @param string	$extras	any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function password_box($name, $value='', $size='', $max='', $extras='')
{
	$size = (int) $size;
	$max  = (int) $max;
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	if (!stristr($extras, 'id=')) {
		$extras .= ' id="'.htmlspecialchars($name).'"';
	}
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
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	if (!stristr($extras, 'id=')) {
		$extras .= ' id="'.htmlspecialchars($name).'"';
	}

	$js = 'if (this.value.length > '.$max.') { this.value = this.value.substr(0,'.$max.'); alert(\'This field can contain no more than '.$max.' characters.\'); this.focus()}';

	echo 	'<textarea name="', htmlspecialchars($name), '" ',
			'cols="'.$cols.'" ',
			'rows="'.$rows.'" ',
			(($max)   ? 'onkeyup="'.$js.'" onblur="'.$js.'"' : ''),
			$extras,
			'>',
			htmlspecialchars($contents),
			'</textarea>';

}//end text_area()


/**
* Prints a radio button
*
* @param string		$name		the form name to use for the radio button
* @param string		$value		the value that will come through when the button is selected
* @param boolean	$checked	whether the button is checked by default or not
* @param string		$on_click	any JS that should be executed when the submit button is pressed
* @param string		$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function radio_button($name, $value='1', $checked=FALSE, $on_click='', $extras='')
{
	if ($on_click) {
		$extras .= ' onclick="'.htmlspecialchars($on_click).' "';
	}
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	if (!stristr($extras, 'id=')) {
		$extras .= ' id="'.htmlspecialchars($name.'_'.$value).'"';
	}
	echo	'<input type="radio" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($checked) ? 'checked="checked" ' : ''),
			$extras,
			' />';

}//end radio_button()


/**
* Prints a check box
*
* @param string		$name		the form name to use for the check box
* @param string		$value		the value that will come through when the box is selected
* @param boolean	$checked	whether the button is checked by default or not
* @param string		$on_click	any JS that should be executed when the submit button is pressed
* @param string		$extras		any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function check_box($name, $value='1', $checked=FALSE, $on_click='', $extras='')
{
	if ($on_click) {
		$extras .= ' onclick="'.htmlspecialchars($on_click).' "';
	}
	if (!stristr($extras, 'id=')) {
		$extras .= ' id="'.htmlspecialchars($name).'"';
	}
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	echo	'<input type="checkbox" ',
			'name="', htmlspecialchars($name), '" ',
			'value="', htmlspecialchars($value), '" ',
			(($checked) ? 'checked="checked" ' : ''),
			$extras,
			' />';

}//end check_box()


/**
* Given a name, an array of keys and values, and a default value, prints a dropdown box
*
* @param string		$name		the form name to use for the dropdown box
* @param array		$contents	an array of whose keys are values to be returned when that
*								option is selected and whose values are the display text for the option
* @param boolean	$multiple	whether you can multiple selections can be made or not
* @param mixed		$defaults	if there is only one default can be a string, or if there are
*								multiple can be an array
* @param int		$height		the height (in lines) of the combo box
* @param string		$extras		any input field attributes that should be placed in the <input ... > tag
* @param int		$maxlength	the maximum number of characters displayed for any option,
*								anything over gets truncated with '...' appended
*
* @return void
* @access public
*/
function combo_box($name, $contents, $multiple=FALSE, $defaults='', $height=0, $extras='', $maxlength=0)
{
	$height    = (int) $height;
	$maxlength = (int) $maxlength;
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	if (!stristr($extras, 'id=')) {
		$extras .= ' id="'.htmlspecialchars($name).'"';
	}
	if ($multiple) {
		$extras .= ' multiple="multiple"';
		$name .= '[]';
	}

	if (!is_array($contents)) $contents = Array($contents);
	if (!is_array($defaults)) $defaults = Array($defaults);

	echo	'<select name="', htmlspecialchars($name), '" ',
			(($height > 1)? 'size="'.$height.'" ' : ''),
			$extras,
			'>';

	foreach ($contents as $key => $value) {
		if ($maxlength) {
			$value = str_replace('&nbsp;', ' ', $value);  // we don't want a space to take 6 chars do we ?
			if (strlen($value) > $maxlength) {
				$value = substr($value, 0, $maxlength - 3).'...';
			}
		}
		$value = str_replace(' ', '&nbsp;', htmlspecialchars($value));
		echo	'<option value="', htmlspecialchars($key), '"',
				((in_array((string) $key, $defaults)) ? ' selected="selected"' : ''),
				'>',
				$value,
				'</option>';
	}
	echo '</select>';

}//end combo_box()


/**
* Prints a label
*
* @param string	$text	The text to go inside the label
* @param string	$for	The ID of the form element this label applies to
* @param string	$extras	Any extra HTML to put in the label tag
*
* @return void
* @access public
*/
function label($text, $for='', $extras='')
{
	if (!empty($for)) $extras .= ' for="'.$for.'"';
	if (!empty($extras)) $extras = ' '.$extras;
	echo '<label'.$extras.'>'.$text.'</label>';

}//end label()


/**
* Prints an entry box for a colour picker, requires javascript and colour_picker script
*
* @param string		$name					the form name to use for the colour box
* @param array		$default				the colour you want to display to begin with
* @param boolean	$allow_blanks			allow no colour to be selected ot not
* @param mixed		$button_text			text on the button that displays the colour picker
* @param int		$show_text_box			show the text box to allow manual entry of a colour
* @param string		$show_current_colour	show a box with the current displayed
* @param int		$show_change_colour		show a tick or cross to indicate if the colour has changed
*
* @return void
* @access public
*/
function colour_box($name, $default='000000', $allow_blanks=FALSE, $button_text='*', $show_text_box=TRUE, $show_current_colour=TRUE, $show_change_colour=TRUE)
{

	$default = strtolower($default);
	$default = preg_replace('/^#/', '', $default);

	static $initialised = FALSE;
	if (!$initialised) {
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('lib')?>/html_form/colour_picker.js"></script>
		<script type="text/javascript">var colour_change_image_dir='<?php echo sq_web_path('lib');?>/web/images/'</script>
		<?php
		$initialised = TRUE;
	}

	if ($show_current_colour) {
		?>
		<span id="colour_span_<?php echo $name; ?>" style="background-color: <?php echo $default; ?>;">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>&nbsp;
		<?php
	}

	if ($show_text_box) {
		$extras  = ' id="colour_box_'.$name.'" style="font-family: courier;"'
				  .' onchange="this.value = check_colour(this.value, '.(($allow_blanks) ? 'true' : 'false').');"';
		if ($show_current_colour) {
			 $extras .= ' show_colour_change(\''.addslashes($name).'\');';
		}
		text_box($name, $default, 7, 6, FALSE, $extras);
	} else {
		hidden_box($name, $default);
	}

	normal_button('', " $button_text ",
					  "load_colour_picker(this.form.elements['".addslashes($name)."'],'".sq_web_path('lib')."/html_form')",
					  'style="font-family: courier;"');

	if ($show_change_colour) {
		?>
		<img id="colour_change_<?php echo $name; ?>" name="colour_change_<?php echo $name; ?>" src="<?php echo sq_web_path('lib'); ?>/web/images/cross.gif" width="15" height="15" border="0" align="bottom" />
		<?php
	}

}//end colour_box()


/**
* Returns a file upload box
*
* @param string	$name	the form name to use for the file upload box
* @param string	$extras	any input field attributes that should be placed in the <input ... > tag
*
* @return void
* @access public
*/
function file_upload($name, $extras='')
{
	if (!stristr($extras, 'class=')) {
		$extras .= ' class="sq-form-field"';
	}
	echo '<input type="file" name="', $name, '" ', $extras, ' />';

}//end file_upload()


/**
* Returns the information for an uploaded file if the upload was successful
*
* It returns FALSE on error, blank array on no file upload and the $_FILES[$name]
* if a file was uploaded successfully
*
* @param string	$name	the form name from the file upload box
* @param boolean    $pre_uploaded   TRUE if the pre-uploaded file info is passed
*
* @return mixed boolean|array
* @access public
*/
function get_file_upload_info($name, $pre_uploaded=FALSE)
{
	// browsers of handheld iOS devices disables the file upload
	// input so the _FILES var will be totally empty but this 
	// doesn't mean there was error uploading the file. see more 
	// on bug #5574 Form file upload problem with  iOS device
	if (empty($_FILES)) return Array();
	if (empty($_FILES[$name])) return FALSE;

	$nice_name = $_FILES[$name]['name'];

	// Matrix file preference limits
	require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
	$pref_max_filesize = sizeInBytes($GLOBALS['SQ_SYSTEM']->getUserPrefs('file', $pre_uploaded ? 'SQ_FILE_PREUPLOADED_MAX_SIZE' : 'SQ_FILE_MAX_SIZE'));
	$filesize_limits = Array();
	// Relevant limits set in php.ini
	foreach(Array('upload_max_filesize', 'post_max_size', 'memory_limit') as $param) {
		$param_val = ini_get($param);
		if ($param_val) {
			$filesize_limits[] = sizeInBytes(($param_val));
		}
	}
	$max_size = min($filesize_limits);
	if ($pref_max_filesize && $pref_max_filesize < $max_size) {
		$settings_type = "Matrix";
		$max_allowed_size = $pref_max_filesize/(1024*1024);
	} else {
		$settings_type = "PHP";
		$max_allowed_size = $max_size/(1024*1024);
	}

	// if something went wrong, sorry bad luck
	switch ($_FILES[$name]['error']) {

		case UPLOAD_ERR_NO_FILE :
			return Array();
		break;

		case UPLOAD_ERR_INI_SIZE  :
			trigger_localised_error('SYS0022', E_USER_WARNING, $nice_name, ini_get('upload_max_filesize'), $settings_type, $max_allowed_size);
			return FALSE;
		break;

		case UPLOAD_ERR_FORM_SIZE :
			trigger_localised_error('SYS0021', E_USER_WARNING, $nice_name, easy_filesize($_POST['MAX_FILE_SIZE']), $settings_type, $max_allowed_size);
			return FALSE;
		break;

		case UPLOAD_ERR_PARTIAL :
			trigger_localised_error('SYS0023', E_USER_WARNING, $nice_name);
			return FALSE;
		break;

		case UPLOAD_ERR_NO_FILE :
			trigger_localised_error('SYS0049', E_USER_WARNING, $nice_name);
			return FALSE;
		break;
	
		case UPLOAD_ERR_NO_TMP_DIR :
			trigger_localised_error('SYS0050', E_USER_WARNING, $nice_name);
			return FALSE;
		break;

		case UPLOAD_ERR_CANT_WRITE :
			trigger_localised_error('SYS0051', E_USER_WARNING, $nice_name);
			return FALSE;
		break;
		
		case UPLOAD_ERR_EXTENSION :
			trigger_localised_error('SYS0052', E_USER_WARNING, $nice_name);
			return FALSE;
		break;

	}//end switch

	if (empty($_FILES[$name]['name']) || !is_uploaded_file($_FILES[$name]['tmp_name'])) {
		return FALSE;
	}

	return $_FILES[$name];

}//end get_file_upload_info()


/**
* Commits an uploaded file to its intended place
*
* It returns FALSE on error, blank string on no file upload and the filename (basename)
* if a file was uploaded
*
* @param string		$name			the form name from the file upload box
* @param string		$dest			the destination for the uploaded file, if this is a directory,
*									the file is put in that directory if it is a file, it is written
*									to that file (taking into account $overwrite)
* @param boolean	$overwrite		whether to overwrite any existing file or not
* @param int		$max_size		the maximum number of bytes that the uploaded file can be
* @param boolean	$restrict_exts	if true extension MUST BE in $extensions,
*									if false must NOT be in $extensions
* @param array		$extensions		if not empty, the only extensions that are not allowed to be uploaded
*
* @return mixed boolean|string
* @access public
*/
function commit_file_upload($name, $dest, $overwrite=FALSE, $max_size=0, $restrict_exts=TRUE, $extensions=Array())
{
	$info = get_file_upload_info($name);
	if ($info === FALSE) return FALSE;
	if (!count($info))   return '';

	$nice_name = str_replace('_', ' ', $name);

	if ($max_size > 0 && $max_size < $info['size']) {
		trigger_localised_error('SYS0020', E_USER_WARNING, $nice_name, easy_filesize($max_size));
		return FALSE;
	}

	//// if we get this far the file on the server OK ////

	// Are we worried about the extension ?
	if (count($extensions)) {
		$exts = Array();
		foreach ($extensions as $v) {
			$exts[] = strtolower($v);
		}
		$ext = strtolower(substr(strrchr($info['name'], '.'), 1));
		$in_arr = in_array($ext, $exts);

		// see $restrict_exts documentation in function header
		if (($restrict_exts && !$in_arr) || (!$restrict_exts && $in_arr)) {
			trigger_localised_error('SYS0002', E_USER_WARNING, $ext, $nice_name);
			return FALSE;
		}//end if
	}//end if

	//// Let's try and find out what our final destination file is going to be ////

	// Doe dest exist and is it a directory ?
	if (is_dir($dest)) {
		if (!is_writable($dest)) {
			trigger_localised_error('SYS0007', E_USER_WARNING, $dest);
			return FALSE;
		}
		$dest .= '/'.$info['name'];
	}

	// Now we know we have a proper filename, if it exists are we allowed to write over it
	if (file_exists($dest)) {
		if (!$overwrite) {
			trigger_localised_error('SYS0027', E_USER_WARNING, $info['name']);
			return FALSE;
		}
		if (!is_writable($dest)) {
			trigger_localised_error('SYS0008', E_USER_WARNING, $dest);
			return FALSE;
		}

	// the destination doesn't exist
	} else {
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$parent = dirname($dest);
		// if the parent isn't a directory, can we create it ?
		if (!is_dir($parent) && !create_directory($parent)) {
			trigger_localised_error('SYS0015', E_USER_WARNING, $parent);
			return FALSE;
		} else if (!is_writable($parent)) {
			trigger_localised_error('SYS0016', E_USER_WARNING, $parent);
			return FALSE;
		}
	}//end dest exists

	// Move file over
	if (!move_uploaded_file($info['tmp_name'], $dest)) {
		trigger_localised_error('SYS0025', E_USER_WARNING, $dest);
		return FALSE;
	}

	return basename($dest);

}//end commit_file_upload()


/**
* Get a string version of the given array of type codes, for passing to the asset map
*
* @param array	$type_codes	The type codes to encode
*
* @return string
* @access public
*/
function get_type_codes_string($type_codes)
{
	$all_type_codes = Array();
	foreach ($type_codes as $type_code => $flag) {
		$all_type_codes[] = $type_code;
		switch ($flag) {
			case 'A' :
				$ancestors = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);
				foreach ($ancestors as $ancestor) {
					$all_type_codes[] = $ancestor;
				}
			break;
			case 'D' :
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code);
				foreach ($descendants as $descendant) {
					$all_type_codes[] = $descendant;
				}
			break;
		}//end switch
	}//end foreach

	if (count($all_type_codes) == 1) {
		$type_codes = implode('', $all_type_codes).'|';
	} else {
		$type_codes = implode('|', $all_type_codes);
	}
	return $type_codes;

}//end get_type_codes_string()


/**
* Prints an interface for choosing an asset from the asset map
*
* @param string		$name		the name to use for the field
* @param int		$assetid	current value for this field
* @param array		$type_codes	Array('[type_code]' => '[flag]')
*								where flag = (A|D|I) -> Ancestors, Descendants or Only Itself
* @param string		$map_frame	a reference to the map_frame so we can get the asset map
*								if this is an empty string, then the asset_finder will look for the asset map
*								in the frame where it was loaded
* @param boolean	$new_window	is the asset finder in a different window than the asset map
* @param string		$done_fn	a function to call when the asset finder is finished
* @param array		$buttons	an array of which additional buttons to print (clear, reset)
*
* @return void
* @access public
*/
function asset_finder($name, $assetid=0, $type_codes=Array(), $map_frame='sq_sidenav', $new_window=FALSE, $done_fn='null', $buttons=Array('clear', 'reset'))
{
	$finder_frame = ($new_window || empty($map_frame)) ? 'self' : 'parent.frames["sq_main"]';
	$escaped_name = str_replace('\\', '\\\\', $name);
	$assetid	  = trim($assetid);

	if (!empty($map_frame)) {
		$map_frame = trim('parent.frames["'.$map_frame.'"]', '.');
	} else {
		$map_frame = 'self';
	}
	if ($new_window) {
		$map_frame = 'window.opener.'.$map_frame;
	}

	$label = '';
	if ($assetid) {
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', TRUE);
		if (is_null($asset)) {
			$assetid = 0;
		} else {
			// Dont reveal the asset name if the current user has no read permission
			if ($asset->readAccess()) {
				$label = $asset->short_name;
			}
		}
	}

	$safe_name = 'sq_asset_finder_'.preg_replace('/[^A-Za-z0-9\-]/', '_', $name);

	$type_codes = get_type_codes_string($type_codes);

	hidden_field($name.'[assetid]', $assetid);
	hidden_field($name.'[url]', '');
	hidden_field($name.'[linkid]', '');
	hidden_field($name.'[type_code]', '');
	// call the js function to set where the finder frame is

	text_box($safe_name.'_label', $label, 20, 0, FALSE, 'readonly="readonly" class="sq-form-asset-finder" onfocus="this.tmp = this.value;" onblur="this.value = this.tmp;"');
	// only print the buttons if in the backend (where there's an asset map frame available)
	if (SQ_IN_BACKEND || SQ_IN_LIMBO || defined('SQ_PAINTED_SIMPLE_ASSET_MAP')) {
		?><span class="sq-asset-finder-id-label">Id : #</span><?php
		text_box($safe_name.'_assetid', empty($assetid) ? '' : $assetid, 2, '', FALSE, 'style="border:1px solid #EFEFEF;width:7ex;" onchange="this.value=this.value.replace(/^\s*((?:[\S\s]*\S)?)\s*$/, \'$1\');'.str_replace('"', '\'', $finder_frame).'.set_hidden_field(\''.$name.'\' + \'[assetid]\', this.value);"');
		?> <?php
		normal_button($safe_name.'_change_btn', translate('change'), $map_frame.'.asset_finder_change_btn_press(\''.$escaped_name.'\', \''.$safe_name.'\', \''.addslashes($type_codes).'\', '.$done_fn.');');
		if (in_array('clear', $buttons)) {
			normal_button($safe_name.'_clear_btn', translate('clear'), $map_frame.'.asset_finder_clear_btn_press(\''.$escaped_name.'\', \''.$safe_name.'\');');
		}
		if (in_array('reset', $buttons)) {
			normal_button($safe_name.'_reset_btn', translate('reset'), $map_frame.'.asset_finder_reset_btn_press(\''.$escaped_name.'\', \''.$safe_name.'\', \''.$assetid.'\', \''.addslashes($label).'\');');
		}
		?><script type="text/javascript">
				if (!window.top.frames['sq_resizer'] || <?php echo $map_frame; ?>.SQ_DOCUMENT_LOADED) {
					<?php echo $map_frame; ?>.set_finder(<?php echo $finder_frame; ?>);
				} else {
					// this gets overridden when multiple asset finders exist anyway,
					// so no harm at the moment in only saving the latest set_finder call
					if (typeof currentOnload == 'undefined') {
						currentOnload = <?php echo $map_frame; ?>.onload;
					}
					<?php echo $map_frame; ?>.onload = function() { <?php echo $map_frame; ?>.set_finder(<?php echo $finder_frame; ?>); currentOnload(); }
				}
			</script><?php
	}

}//end asset_finder()


/**
* Prints an interface for choosing several assets from the asset map
*
* @param string		$name		the base name to use for the form fields
* @param array		$assetids	current values for this field
* @param array		$type_codes	Array('[type_code]' => '[flag]')
*								where flag = (A|D|I) -> Ancestors, Descendants or Only Itself
* @param string		$map_frame	Name of the frame containing the asset map
*								if this is an empty string, then the asset_finder will look for the asset map
*								in the frame where it was loaded
* @param boolean	$new_window	is the asset finder in a different window than the asset map
* @param string		$done_fn	a javascript function to call when the asset finder is finished
* @param array		$buttons	an array of which additional buttons to print (clear and/or reset)
*
* @return void
* @access public
*/
function multiple_asset_finder($name, $assetids=Array(), $type_codes=Array(), $map_frame='sq_sidenav', $new_window=FALSE, $done_fn='null', $buttons=Array('clear'))
{
	?><script type="text/javascript" src="<?php echo sq_web_path('lib')?>/html_form/html_form.js"></script><?php

	echo '<div>';
	foreach (array_values($assetids) as $index => $assetid) {
		asset_finder($name.'['.$index.']', $assetid, $type_codes, $map_frame, $new_window, $done_fn);
		echo '<br />';
	}

	$index = count($assetids);
	asset_finder($name.'['.$index.']', 0, $type_codes, $map_frame, $new_window, $done_fn, Array('clear'));
	$index++;

	if (!empty($map_frame)) {
		$map_frame_exp = trim('parent.frames["'.$map_frame.'"]', '.');
	} else {
		$map_frame_exp = 'top';
	}

	$safe_name = 'sq_asset_finder_'.preg_replace('/[^A-Za-z0-9]/', '_', $name);
	$type_codes_string = get_type_codes_string($type_codes);
	echo '<input type="button" name="'.htmlspecialchars($safe_name.'_more_btn').'" id="'.htmlspecialchars($safe_name.'_more_btn').'" value="'.translate('more').'..." onclick="addNewAssetFinder(this, \''.$name.'\', \''.$safe_name.'\', \''.addslashes($type_codes_string).'\', \''.htmlentities($map_frame_exp, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'\', '.$done_fn.', '.(in_array('clear', $buttons) ? 'true' : 'false').')" />';
	echo '<br />';

	echo '</div>';

}//end multiple_asset_finder()


/**
* Prints an interface for entering a security key generated inside an image
* Call validate_security_key() to check if the value entered was correct
*
* @param int		$key_len					the lenght of the random string to display in the image
* @param int		$box_len					the width of the text box used to enter the security key shown
* @param float		$zoom						each character will be randomly zoomed between x1 and this figure
* @param boolean	$img_on_newline				whether the img should appear under the box or not
* @param string		$bg							the background colour of the image (eg FFFFFF)
* @param string		$text						the colour of the text in the image (eg 000000)
* @param string		$border						the colour of the border image (eg 000000)
*												for no border, set to the same colour as the background
* @param boolean	$use_colours				whether to use the supplied CAPTCHA background, text and border colours
* @param boolean	$use_font					whether to use a user-specified TrueType Font for the CAPTCHA text
* @param string		$font						the TrueType Font to use for the CAPTCHA text
* @param int		$font_size					the font point size to use
* @param int		$min_angle					the minimum rotation angle for characters
* @param int		$max_angle					the maximum rotation angle for characters
* @param int		$x_start					the X ordinate location from which to start rendering the CAPTCHA
* @param int		$min_dist					the minimum pixel distance between characters
* @param int		$max_dist					the maximum pixel distance between characters
* @param int		$ttf_width					the pixel width of the TrueType Font image
* @param int		$ttf_height					the pixel height of the TrueType Font image
* @param boolean	$use_arc					whether to overlay arcs across the CAPTCHA text
* @param string		$arc_colour					the RGB colour of the arc elements
* @param boolean	$include_uppercase			whether to include uppercase characters in the CAPTCHA text
* @param boolean	$include_numbers			whether to include numbers in the CAPTCHA text
* @param boolean	$provide_accessible_captcha	whether to display a link and interface to enter an email address for an accessible CAPTCHA
* @param boolean	$accessible_format	                   print accessible format 
* @param string	$title	text label used to prompt user to enter CAPTCHA 
* @param string	$error	error message to display if wrong CAPTCHA is entered
*
* @return void
* @access public
*/
function security_key($key_len, $box_len, $zoom=1, $img_on_newline=FALSE, $bg='FFFFFF', $text='000000', $border='000000', $use_colours=FALSE, $use_font=FALSE, $font='', $font_size=20, $min_angle=10, $max_angle=20, $x_start=20, $min_dist=20, $max_dist=20, $ttf_width=200, $ttf_height=36, $use_arc=FALSE, $arc_colour='D20A0A', $include_uppercase=FALSE, $include_numbers=FALSE, $provide_accessible_captcha = FALSE, $accessible_format = FALSE, $title='', $error="")
{

	// generate a random security key string
	require_once SQ_FUDGE_PATH.'/general/general.inc';
	$password = generate_security_key($key_len, $include_uppercase, $include_numbers);

	// work out the width and height of the image
	$img_width  = $key_len * 10.5;
	$img_height = 20;

	// we need to store these vars in the session so we can get the
	// system to print the security image for us
	$security_vars = Array(
						'key_len'			=> $key_len,
						'key'				=> $password,
						'width'				=> $img_width,
						'height'			=> $img_height,
						'use_colours'		=> $use_colours,
						'bg_colour'			=> $bg,
						'text_colour'		=> $text,
						'border_colour'		=> $border,
						'zoom'				=> $zoom,
						'use_font'			=> $use_font,
						'font'				=> $font,
						'font_size'			=> $font_size,
						'min_angle'			=> $min_angle,
						'max_angle'			=> $max_angle,
						'x_start'			=> $x_start,
						'min_dist'			=> $min_dist,
						'max_dist'			=> $max_dist,
						'ttf_width'			=> $ttf_width,
						'ttf_height'		=> $ttf_height,
						'use_arc'			=> $use_arc,
						'arc_colour'		=> $arc_colour,
						'include_uppercase'	=> $include_uppercase,
						'include_numbers'	=> $include_numbers,
						'provide_accessible_captcha'	=> $provide_accessible_captcha,
					 );
	$_SESSION['SQ_SYSTEM_SECURITY_KEY'] = $security_vars;

	// generate the url to get the security image to print
	$img_url = $_SERVER['PHP_SELF'].'?SQ_ACTION=send_security_key';
	if ($accessible_format) {
	    echo '<span id="sq_normal_captcha">';
	    echo '<img id="sq_security_key" src="'.$img_url.'" alt="Security key" border="0" /><br/>';
	    if(empty($title)) $title = 'Type the characters you see in the picture above: ';
	    label($title, 'SQ_SYSTEM_SECURITY_KEY_VALUE');
	    echo '<br/>';
	    echo text_box('SQ_SYSTEM_SECURITY_KEY_VALUE', '', $box_len);
	    if(!empty($error))
		echo '<br /><span id="SQ_SYSTEM_SECURITY_KEY_VALUE_ERROR" class="sq-form-error sq-captcha-error">'.$error.'</span>';
	    echo '</span>';
	    if ($provide_accessible_captcha) {
		print_accessible_captcha_interface();
	    }
	    $replacements_link = '<a href="#" onclick="var img = document.getElementById(\'sq_security_key\'); img.src = \''.$img_url.'&amp;regen_key=1&amp;t=\' + new Date().getTime(); return false;">'.translate('security_key_link').'</a>';
	    echo '<span id="sq_regen_captcha"><br /><i>'. translate('security_key_question', $replacements_link).'</i></span>'; 
				
	}
	else {
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
		}//end if
		?>
				<td valign="top"><img id="sq_security_key" src="<?php echo $img_url; ?>" alt="Security key" border="0" />
		<?php

		if ($provide_accessible_captcha) {
			print_accessible_captcha_interface();
		}
		?>
		</td>
		</tr>
		<tr>
		<?php
			ob_start();
				?><a href="#" onclick="var img = document.getElementById('sq_security_key'); img.src = '<?php echo $img_url; ?>&amp;regen_key=1&amp;t=' + new Date().getTime(); return false;"><?php echo translate('security_key_link'); ?></a><?php
				$replacements_link = ob_get_contents();
			ob_end_clean()
		?>
			<td class="sq-backend-data" id="sq_regen_captcha" colspan="<?php echo (($img_on_newline) ? '1' : '2'); ?>"><i><?php echo translate('security_key_question', $replacements_link); ?></i></td>
		</tr>
	</table>
	<?php
	}

}//end security_key()


/**
* Prints an security key generated inside an image and sets the security key session vars
*
* @param int		$key_len		the lenght of the random string to display in the image
* @param float		$zoom			each character will be randomly zoomed between x1 and this figure
* @param string		$bg				the background colour of the image (eg FFFFFF)
* @param string		$text			the colour of the text in the image (eg 000000)
* @param string		$border			the colour of the border image (eg 000000)
*									for no border, set to the same colour as the background
* @param boolean	$use_colours	whether to use the supplied CAPTCHA background, text and border colours
* @param boolean	$use_font		whether to use a user-specified TrueType Font for the CAPTCHA text
* @param string		$font			the TrueType Font to use for the CAPTCHA text
* @param int		$font_size		the font point size to use
* @param int		$min_angle		the minimum rotation angle for characters
* @param int		$max_angle		the maximum rotation angle for characters
* @param int		$x_start		the X ordinate location from which to start rendering the CAPTCHA
* @param int		$min_dist		the minimum pixel distance between characters
* @param int		$max_dist		the maximum pixel distance between characters
* @param int		$ttf_width		the pixel width of the TrueType Font image
* @param int		$ttf_height		the pixel height of the TrueType Font image
* @param boolean	$use_arc		whether to overlay arcs across the CAPTCHA text
* @param string		$arc_colour		the RGB colour of the arc elements
* @param boolean	$include_uppercase	whether to include uppercase characters in the CAPTCHA text
* @param boolean	$include_numbers	whether to include numbers in the CAPTCHA text
*
* @return void
* @access public
*/
function security_key_image_box($key_len, $zoom=1, $bg='FFFFFF', $text='000000', $border='000000', $use_colours=FALSE, $use_font=FALSE, $font='', $font_size=20, $min_angle=10, $max_angle=20, $x_start=20, $min_dist=20, $max_dist=20, $ttf_width=200, $ttf_height=36, $use_arc=false, $arc_colour='D20A0A', $include_uppercase=FALSE, $include_numbers=FALSE, $provide_accessible_captcha = FALSE)
{
	// generate a random security key string
	require_once SQ_FUDGE_PATH.'/general/general.inc';
	$password = generate_security_key($key_len, $include_uppercase, $include_numbers);

	// work out the width and height of the image
	$img_width  = $key_len * 10.5;
	$img_height = 20;

	// we need to store these vars in the session so we can get the
	// system to print the security image for us
	$security_vars = Array(
						'key_len'		=> $key_len,
						'key'			=> $password,
						'width'			=> $img_width,
						'height'		=> $img_height,
						'use_colours'	=> $use_colours,
						'bg_colour'		=> $bg,
						'text_colour'	=> $text,
						'border_colour'	=> $border,
						'zoom'			=> $zoom,
						'use_font'		=> $use_font,
						'font'			=> $font,
						'font_size'		=> $font_size,
						'min_angle'		=> $min_angle,
						'max_angle'		=> $max_angle,
						'x_start'		=> $x_start,
						'min_dist'		=> $min_dist,
						'max_dist'		=> $max_dist,
						'ttf_width'		=> $ttf_width,
						'ttf_height'	=> $ttf_height,
						'use_arc'		=> $use_arc,
						'arc_colour'	=> $arc_colour,
						'include_uppercase'	=> $include_uppercase,
						'include_numbers'	=> $include_numbers,
						'provide_accessible_captcha'	=> $provide_accessible_captcha,
					 );
	$_SESSION['SQ_SYSTEM_SECURITY_KEY'] = $security_vars;

	// generate the url to get the security image to print
	$img_url = $_SERVER['PHP_SELF'].'?SQ_ACTION=send_security_key';
	?>
		<img id="sq_security_key" src="<?php echo $img_url; ?>" border="0" alt="Security key" />
	<?php

	if ($provide_accessible_captcha) {
		print_accessible_captcha_interface();
	}

}//end security_key_image_box()


/**
* Prints the hidden SPAN and JS magic for the Accessible CAPTCHA field
*
* @return void
* @access public
*/
function print_accessible_captcha_interface()
{
    ?>
    <script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script>
    <script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/captcha.js"></script>
	<noscript>
		Please enable JavaScript to access the Accessible CAPTCHA interface.
	</noscript>
    <?php

        // The following flag is used to verify that the AJAX request originated from Matrix - it does not indicate a CAPTCHA "pass"
        $_SESSION['SQ_ACCESSIBLE_CAPTCHA_GENERATED'] = 1;
    ?>
<span style="display: none; visibility: hidden;" id="sq_accessible_captcha">
<?php
    echo '<label for="SQ_SYSTEM_SECURITY_KEY_EMAIL">Please enter your email address: </label>';
    echo '<br />';
    echo text_box('SQ_SYSTEM_SECURITY_KEY_EMAIL', '', 30, '', FALSE, 'onkeypress="enable_submission_button();"');
?>
	<input type="button" id="sq_submit_accessible_captcha" value="Submit address" onclick="submit_email_captcha('<?php echo sq_web_path('lib'); ?>');" />
</span>
<span id="sq_accessible_validation_link">&nbsp;<a href="javascript:show_accessible_captcha();">Use accessible validation</a></span>
<span id="sq_accessible_captcha_message">
    <span id="sq_accessible_captcha_error" style="display: none; visibility: hidden;"><br />Invalid email address</span>
    <span id="sq_accessible_captcha_instruction" style="display: none; visibility: hidden;"><br />A validation email has been sent to the address specified.<br />Please click the link in this email to validate and continue your form submission.</span>
</span>
    <?php

}//end print_accessible_captcha_interface()


/**
* Prints a link for loading a new security key image with javascript
*
* @param int	$box_len	size of the input field
*
* @return void
* @access public
*/
function security_key_field($box_len)
{
	echo text_box('SQ_SYSTEM_SECURITY_KEY_VALUE', '', $box_len);

}//end security_key_field()


/**
* Prints a link for loading a new security key image with javascript
*
* @param string	$link_text	text to be used for the link
*
* @return void
* @access public
*/
function security_key_image_link($link_text)
{
	$img_url = $_SERVER['PHP_SELF'].'?SQ_ACTION=send_security_key';
	ob_start();
		?><span id="sq_regen_captcha"><a href="#" onclick="var img = document.getElementById('sq_security_key'); img.src = '<?php echo $img_url; ?>&regen_key=1&t=' + new Date().getTime(); return false;"><?php echo $link_text; ?></a></span><?php
		$replacement_link = ob_get_contents();
	ob_end_clean();

	echo $replacement_link;

}//end security_key_image_link()


/**
* Returns true if the security key entered using the interface in security_key() is correct
*
* @return boolean
* @access public
*/
function validate_security_key()
{
	// If we are using an Accessible CAPTCHA, have received a verification link and clicked it,
	// and there's a grin from the man upstairs, then I think we're golden
	if ((isset($_SESSION['SQ_ACCESSIBLE_CAPTCHA_PASSED'])) && ($_SESSION['SQ_ACCESSIBLE_CAPTCHA_PASSED'] === 1)) {
		// The actual key comparison is done upon the secondary page request at core/lib/accessible_captcha.php
		unset($_SESSION['SQ_ACCESSIBLE_CAPTCHA_PASSED']);

		return TRUE;
	}

	if (!isset($_POST['SQ_SYSTEM_SECURITY_KEY_VALUE'])) {
		return FALSE;
	}
	if (!isset($_SESSION['SQ_SYSTEM_SECURITY_KEY']['key'])) {
		return FALSE;
	}
	$entered_value = $_POST['SQ_SYSTEM_SECURITY_KEY_VALUE'];
	$real_value = $_SESSION['SQ_SYSTEM_SECURITY_KEY']['key'];
	unset($_SESSION['SQ_SYSTEM_SECURITY_KEY']);
	return ($entered_value === $real_value);

}//end validate_security_key()


/**
* Takes a variable and outputs as many hidden fields as necessary to recreate it
*
* @param array	$var	the array of variables to insert into the hidden fields
* @param string	$name	the form name to use for the field
*
* @return void
* @access public
*/
function var_to_hidden_field($var, $name='')
{
	if (is_array($var) || is_object($var)) {
		foreach ($var as $k => $v) {
			var_to_hidden_field($v, (($name) ? $name.'['.$k.']' : $k));
		}
	} else {
		if ($name) hidden_field($name, $var);
	}

}//end var_to_hidden_field()


/**
* Print an interface widget to choose asset type(s)
*
* If $show_inherit_checkbox=false, then $_REQUEST[$name] will be type code, or array of type codes.
* Otherwise it will be an Array('type_code' => (Array of) type code(s), 'inherit' => (Array of) 1(s) or 0(s))
*
* @param string		$name					Name for the field
* @param boolean	$multiple				Whether we want to be able to choose several types
* @param array		$defaults				The default values
* @param boolean	$instantiable			Do we want instantiable or non-instantiable assets? (null means both)
* @param boolean	$system_accessible		Do we want to include assets that only have system access?
* @param boolean	$show_inherit_checkbox	Do we want to show a checkbox for inheriting from a type?
* @param array		$specified_types		Limit the listing to specified types and their decendants
*
* @return void
* @access public
*/
function asset_type_chooser($name, $multiple=FALSE, $defaults=Array(), $instantiable=NULL, $system_accessible=FALSE, $show_inherit_checkbox=FALSE, $specified_types=Array())
{
	// if we specified types for the listing, build a list with these
	if (!empty($specified_types)) {
		foreach ($specified_types as $specified_type) {
			$new_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($specified_type, TRUE);

			foreach ($new_types as $new_type) {
				$types[$new_type] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_type, 'name');
			}
		}
	} else {
		$types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes($instantiable, !$system_accessible);

		foreach ($types as $code => $details) {
			$types[$code] = $details['name'];
		}
	}

	asort($types);
	$types = Array('' => '') + $types;
	if ($multiple) {
		if ($show_inherit_checkbox) {
			if (!empty($defaults)) {
				foreach ($defaults['type_code'] as $i => $type_code) {
					if (empty($type_code)) continue;
					combo_box($name.'[type_code][]', $types, FALSE, $type_code);
					hidden_field($name.'[inherit][]', $defaults['inherit'][$i]);
					check_box('', 1, $defaults['inherit'][$i], 'toggleLastHiddenField(this)');
					label(strtolower(translate('inherit')), '');
					normal_button('', translate('clear'), 'resetLastSelect(this)');
					echo '<br />';
				}
			}
			combo_box($name.'[type_code][]', $types, FALSE, '');
			hidden_field($name.'[inherit][]', 0);
			check_box('', 1, FALSE, 'toggleLastHiddenField(this)');
			label(strtolower(translate('inherit')), '');
			normal_button('', translate('clear'), 'resetLastSelect(this); clearLastCheckbox(this);');
			normal_button('', translate('more').'...', 'prependTypeSelector(this); prependInheritSelector(this); prependClearButton(this, true)');
		} else {
			foreach ($defaults as $default) {
				if (!empty($default)) {
					combo_box($name.'[]', $types, FALSE, $default);
					normal_button('', translate('clear'), 'resetLastSelect(this)');
					echo '<br />';
				}
			}
			combo_box($name.'[]', $types, FALSE, '');
			normal_button('', translate('clear'), 'resetLastSelect(this)');
			normal_button('', translate('more').'...', 'prependTypeSelector(this); prependClearButton(this, false)');
		}
		echo '<br />';
	} else {
		if ($show_inherit_checkbox) {
			combo_box($name.'[type_code]', $types, FALSE, array_get_index($defaults, 'type_code', ''));
			hidden_field($name.'[inherit]', array_get_index($defaults, 'inherit', '0'));
			check_box('', 1, array_get_index($defaults, 'inherit', '0'), 'toggleLastHiddenField(this)');
			label(strtolower(translate('inherit')), '');
		} else {
			combo_box($name, $types, FALSE, $defaults);
		}
	}

}//end asset_type_chooser()


/**
* Print an interface widget to enter a URL
*
* @param string	$name		Name for the field
* @param string	$value		Default value for the URL
* @param array	$protocols	Supported protocols. Ie, Array('http', 'mailto', 'ftp')
* @param int	$width		the width of the input box
*
* @return void
* @access public
*/
function url_tester($name, $value='', $protocols=Array('http'), $width=50)
{
	?><script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script><?php
	?><script type="text/javascript">

		function getFullUrl(id_name) {
			var elementName = id_name + "_protocol";
			var protocol = document.getElementById(elementName).value;
			var url = document.getElementById(id_name + "_url").value;
			var fullUrl = protocol + url;

			return fullUrl;
		}

		function <?php echo $name; ?>changeStatusText(responseState)
		{
			switch (responseState) {
				case "0":
					responseText = js_translate('non_existent_page');
				break;
				case "1":
					responseText = js_translate('valid_page');
				break;
				default:
					responseText = js_translate('unknown_state');
			}

			document.getElementById('<?php echo $name.'_status_text'; ?>').innerHTML = responseText;

		}//end changeStatusText()

		function checkUrlExists(id_name)
		{
			document.getElementById(id_name + "_status_text").innerHTML = js_translate('testing');
			var testUrl = "<?php echo sq_web_path('lib').'/web/connectivity.php'; ?>" + "?connect_url=" + getFullUrl(id_name);
			JsHttpConnector.submitRequest(testUrl, eval(id_name + "changeStatusText"));

		}//end checkUrlExists()

		function protocolChanged(id_name)
		{
			var protocol = document.getElementById(id_name + "_protocol").value;
			if (protocol == "http://") {
				document.getElementById(id_name + "_button_auto").disabled = "";
			} else {
				document.getElementById(id_name + "_button_auto").disabled = "disabled";
			}
			document.getElementById(id_name + "_status_text").innerHTML = "";

		}//end protocolChanged()

		function testUrl(id_name)
		{
			window.open(getFullUrl(id_name));

		}
	</script><?php

	$default_prot = '';
	$dd_values = array_flip($protocols);
	foreach ($dd_values as $key => $prot) {
		$replacement = '';
		if ($key == 'mailto') {
			$replacement = $key.':';
			$dd_values[$replacement] = $replacement;
		} else {
			$replacement = $key.'://';
			$dd_values[$replacement] = $replacement;
		}
		unset($dd_values[$key]);

		// because we are storing the protocol + URL as the value, we
		// need to separate the two
		$protocol = substr($value, 0, strlen($replacement));
		if ($protocol == $replacement) {
			$default_prot = $protocol;
			$value = substr($value, strlen($replacement));
		}
	}

	combo_box($name.'_protocol', $dd_values, FALSE, $default_prot, 0, 'onChange="protocolChanged(\''.$name.'\')";');
	echo '&nbsp;';

	text_box($name.'_url', $value, $width);
	echo '&nbsp;';

	normal_button($name.'_button_auto', 'Auto-Test', 'javascript: checkUrlExists(\''.$name.'\');');
	normal_button($name.'button_manual', 'Open URL', 'javascript: testUrl(\''.$name.'\');');

	?><div id="<?php echo $name; ?>_status_text"></div><?php

}//end url_tester()


/**
* Returns the full URL of the url_tester form entity
*
* It returns Empty string if the POST var isn't set, or the full URL on success (eg. protocol://url)
*
* @param string	$name	the form name of the url_tester
*
* @return string
* @access public
*/
function get_url_tester_url($name)
{
	$url = '';
	if ( (!empty($_REQUEST[$name.'_protocol'])) && (!empty($_REQUEST[$name.'_url'])) ) {
		$url = $_REQUEST[$name.'_protocol'].$_REQUEST[$name.'_url'];
	}
	return trim($url);

}//end get_url_tester_url()


/**
* Draws a structured dropdown list for selecting assets
*
* @param string		$name			the name of the menu
* @param int		$root_id		the ID of the asset to start from
* @param array		$type_codes		An array of type codes to restrict
*									options to.
* @param array		$selected		Pre-selected value
* @param int		$width			The width of the drop down
* @param int		$height			The height of the drop down
* @param int		$max_depth		Maximum depth to display nodes to
* @param boolean	$allow_multiple	Allow Multiple Entries
* @param string		$seperator		String to display between entries
* @param boolean	$include_root	Whether or not to list the root node
* @param string		$root_text		Text to display for root node
* @param boolean	$check_boxes	Display as check boxes (false = dropdown)
* @param array		$statuses		statuses to filter from list
* @param boolean	$structured		draw the structured tree
* @param array		$link_types		Allowed link types(Empty for all types)
* @param array		$allow_empty	Add an empty entry at the start
* @param int		$permission		Minimum required permission on an asset in order for it tobe displayed
*
* @return array
* @access protected
*/
function structured_drop_down($name, $root_id, $type_codes, $selected=Array(), $width=0, $height=0, $max_depth=0, $allow_multiple=FALSE, $seperator='-', $include_root=FALSE, $root_text='', $check_boxes=FALSE, $statuses=Array(), $structured=TRUE, $link_types=Array(), $allow_empty=FALSE, $permission=NULL, $skip_assetids=Array())
{
	$all_child_links = $GLOBALS['SQ_SYSTEM']->am->getAllChildLinks($root_id, 0, $permission, TRUE, 'gc_', $max_depth);

	$child_links = $all_child_links;
	$search_tree = Array();

	$asset_info_ids = Array();

	$invalid = Array();
	$invalid_type = Array();

	// weed out any unwanted entries
	foreach ($all_child_links as $treeid => $data) {
		// build an array of assetids as we go
		$asset_info_ids[] = $data['assetid'];

		if (!in_array($data['type_code'], $type_codes) && !empty($type_codes)) {
			$invalid_type[] = $child_links[$treeid]['assetid']; // we need this
			unset($child_links[$treeid]);
			// need to take this bit off as it is storing the treeid of the asset which has invalid type_code match
			// by doing so we are not looking under this tree id. But what if i have a Folder under a Standard Page
			// and i have restricted type to "Folder" ???
			// below code will result id not looking under the standard page all together since it is a type_code mismatch
			// $invalid[] = $treeid;
			continue;
		}

		if ($child_links[$treeid]['is_dependant']) {
			$invalid_type[] = $child_links[$treeid]['assetid']; // we need this
			unset($child_links[$treeid]);
			$invalid[] = $treeid;
			continue;
		}

		if ($structured) {
			// Bug #2914 - Returning folders that are listed under assets which should not be printed
			// This had an if statement here to test for structured=TRUE which did not filter all out properly
			// Bug Fix #3534. This need to be put back from the previous bug fix. Just the logic was wrong
			foreach ($invalid as $banned) {
				if (strlen($banned) > strlen($treeid)) continue;
				if (substr($treeid, 0, strlen($banned)) == $banned) {
					unset($child_links[$treeid]);
					continue 1;
				}
			}
		}

		if (!empty($link_types) && !in_array($data['link_type'], $link_types)) {
			unset($child_links[$treeid]);
			$invalid[] = $treeid;
			continue;
		}
		
		if (!empty($skip_assetids) && in_array($data['assetid'], $skip_assetids)) {
			// Issue #6193 Exclude asset/root node settings ignored in Asset Listing
			// allow the assets to be skipped if they are in skip list. we are doing
			// this to make sure the root nodes we are trying to exclude do not app-
			// ear in the dropdown in the frontend as there must be a reason it is skipped
			unset($child_links[$treeid]);
			$invalid[] = $treeid;
			continue;
		}
	}//end foreach

	// check if we have statuses we want to filter out, and also check user has atleast read permission
	if (!empty($statuses) && count($statuses) < 9) {
		foreach ($child_links as $treeid => $data) {
			$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($data['assetid']));
			if (!in_array($asset_info[$data['assetid']]['status'], $statuses)) {
				unset($child_links[$treeid]);
			}
		}
	}

	$options = Array();
	if ($allow_empty) $options[''] = '';
	$level = 1;
	if ($include_root) {
		if (!empty($root_text)) {
			$options[$root_id] = $root_text;
		} else {
			$options[$root_id] = 'All';
		}
		$level = 0;
	}
	// Insert each node into the tree
	$search_tree = Array();
	foreach ($child_links as $treeid => $child) {
		id_list_to_tree($treeid, $search_tree, '', $child['sort_order']);
	}
	sort_tree($search_tree);
	draw_tree($search_tree, $all_child_links, $options, '', $level, $seperator, $max_depth, $structured);

	// some how while in draw_tree function, matrix is putting back the assets that were removed in the
	// very first foreach() method call in the function. so we have to get rid of them
	foreach ($options as $key_id => $name_value) {
		if (in_array($key_id,$invalid_type)) {
			unset($options[$key_id]);
		}
	}

	if ($check_boxes) {
		foreach ($options as $id => $output) {
			if ($allow_multiple) {
				check_box($name.'['.$id.']', $id, in_array($id, $selected));
			} else {
				radio_button($name, $id, in_array($id, $selected));
			}
			label($output, $name.'['.$id.']');
			echo '<br />';
		}
	} else {
		combo_box($name, $options, $allow_multiple, $selected, $height, ($width != '0') ? 'style="width:'.$width.'px;"' : '');
	}

}//end structured_drop_down()


/**
* Takes a tree ID and inserts it into a tree
*
* $tree is in the following format:
* <pre>
* Array(
*	'0001'	=> Array(
* 				'sort_order' => 0,
* 			   ),
*	'0002'	=> Array(
* 				'sort_order' => 1,
*				'00020001'	=> Array(
* 								'sort_order' => 0,
* 							   ),
*			   ),
* )
* </pre>
*
* @param string	$treeid			the ID of the tree to format
* @param array	&$tree			the tree to insert the formatted ID into
* @param string	$chunk_prefix	optional, used internally
* @param int 	$sort_order		sort order for current branch of tree
*
* @return void
* @access private
*/
function id_list_to_tree($treeid, &$tree, $chunk_prefix='', $sort_order)
{
	if (strlen($treeid) < SQ_CONF_ASSET_TREE_SIZE) return FALSE;

	$first_chunk = substr($treeid, 0, SQ_CONF_ASSET_TREE_SIZE);

	if (strlen($treeid) > SQ_CONF_ASSET_TREE_SIZE) {
		$remaining_chunk = substr($treeid, SQ_CONF_ASSET_TREE_SIZE, strlen($treeid));
		if (!isset($tree[$chunk_prefix.$first_chunk]) || !is_array($tree[$chunk_prefix.$first_chunk])) {
			$tree[$chunk_prefix.$first_chunk] = Array();
			$tree[$chunk_prefix.$first_chunk]['sort_order'] = 0;
		}
		id_list_to_tree($remaining_chunk, $tree[$chunk_prefix.$first_chunk], $chunk_prefix.$first_chunk, $sort_order);
	} else {
		$tree[$chunk_prefix.$first_chunk]['sort_order'] = $sort_order;
	}

}//end id_list_to_tree()


/**
* Takes a tree array and an output array, and fills the output array with an ascii tree,
* with each line of the tree as an element in the array
*
* @param array		$tree				the tree to format
* @param array		&$all_child_links	assetInfo() for use when printing
* @param array		&$output_tree		the array to store the output array
* @param string		$branch				optional, used internally
* @param array		$level				Level in the tree to print as the base
* @param string		$seperator			String to display between
* @param int		$max_depth			Maximum depth to print
* @param boolean	$use_seperator		Facilitates the drawing of the tree with no structure. Allows flat
*										Dropdowns to be created, while still using levels etc.
*
* @return void
* @access private
*/
function draw_tree($tree, &$all_child_links, &$output_tree, $branch='', $level=0, $seperator='-', $max_depth=0, $use_seperator=TRUE)
{
	if (empty($tree)) return FALSE;
	foreach ($tree as $branch => $branch_data) {
		// skip if current node is not treeid element
		if ($branch == 'sort_order') continue;
		$prefix = '';

		$lvl = $all_child_links[$branch]['lvl'] - $level;
		if ($max_depth != 0 && $lvl >= $max_depth) {
			return;
		}
		if ($use_seperator == TRUE) {
			for ($i = 0; $i < $lvl; $i++) {
				$prefix .= ' '.$seperator.' ';
			}
		}
		$output_tree[$all_child_links[$branch]['assetid']] = $prefix.$all_child_links[$branch]['short_name'];
		if (is_array($branch_data) && !empty($branch_data)) {
			draw_tree($branch_data, $all_child_links, $output_tree, $branch, $level, $seperator, $max_depth, $use_seperator);
		}
	}

}//end draw_tree()


/**
* Paints a dropdown of keywords and inserts a selected one into text
*
* NOTE: Keywords must contain the enclosing characters like % signs
*
* @param array	$keywords	Keywords to insert in 'keyword'=>'value' format
* @param string	$target_id	Id of element to insert keyword into
* @param string	$self_id	Id to assign to the keyword selector dropdown
* @param string	$prompt		Text prompt asking users to select a keyword
*
* @return void
* @access public
*/
function keyword_selector($keywords, $target_id, $self_id=NULL, $prompt=NULL)
{
	static $internal_id = 0;
	if (is_null($self_id)) {
		$internal_id++;
		$self_id = 'sq_keyword_selector_'.$internal_id;
	}

	if (empty($prompt)) {
		$prompt = '-- '.translate('insert_keyword').' --';
	}

	?><script type="text/javascript" src="<?php echo sq_web_path('lib')?>/html_form/html_form.js"></script>
	<select id="<?php echo $self_id; ?>" onchange="insert_text(this.value, '<?php echo $target_id; ?>', this.selectedIndex=0)">
		<option value=""><?php echo $prompt; ?></option>
	<?php
	foreach ($keywords as $kw => $name) {
		?>
		<option value="<?php echo htmlentities($kw, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?>"><?php echo htmlentities($name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET); ?></option>
		<?php
	}
	?>
	</select>
	<?php

}//end keyword_selector()


/**
* Sorts tree array including nested arrays as well
*
* @param array	&$tree	Reference to array that needs to be sorted
*
* @return void
* @access public
*/
function sort_tree(&$tree)
{
	foreach ($tree as &$treeid) {
		// checks if nested array has more entries than just sort_order key
		if (count($treeid) > 1) {
			sort_tree($treeid);
		}
	}

	uasort($tree, 'sort_compare');

}//end sort_tree()


/**
* Comparison function for use in uasort()
* 
* @param array	$left
* @param array 	$right
*
* @return int
* @access public
*/
function sort_compare($left, $right)
{	
	if (!is_array($left) || !is_array($right)) {
		return 0;
	}
	if ($left['sort_order'] > $right['sort_order']) {
		return 1;
	} else {
		return -1;
	}

}//end sort_compare()


?>
