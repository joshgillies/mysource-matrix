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
* $Id: calendar_event_edit_fns.inc,v 1.19 2004/09/10 01:44:26 tbarrett Exp $
* $prefix:  $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

/**
* Page_Edit_Fns
*
* Functions used by the edit interface of the calendar_event asset
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.19 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;
		$this->static_screens['details']['lock_type'] = 'attributes';

	}//end Calendar_Event_Edit_Fns()


	/**
	* Paint a form for entering the start/end date/time for an event, including javascript validation functions
	*
	* @param object Calendar_Event	$asset			the event to print the form for
	* @param string					$prefix			the prefix to use for form field names
	* @param string					$attributes		extra attributes to print for each input field name=>val
	*
	* @return boolean
	* @access public
	*/
	function paintStartEndChooser(&$asset, $prefix, $attributes=Array()) 
	{
		
		$occurrence_end_ts = (isset($_REQUEST['SQ_CALENDAR_DATE']) && ($asset->getEndDate())) ? strtotime($_REQUEST['SQ_CALENDAR_DATE'].' '.($asset->getEndTime() ? $asset->getEndTime() : '12:00:00 GMT')) : false;
		$end_disabled_code = (($asset->id && ($asset->getEndTime() || $asset->getEndDate())) || (isset($_REQUEST['SQ_CALENDAR_DURATION']))) ? '' : 'disabled="disabled"';
		$end_disabled_array = $end_disabled_code ? Array('disabled'=>'disabled') : Array();

		?>
		<script language="JavaScript" type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/calendar_event/js/date_chooser.js'?>"></script>

		<!-- start -->
		<textarea id="textmessage"></textarea>
		<table border="0" cellpadding="0" cellspacing="0" style="margin-bottom: 1ex">
			<tr>
				<td class="sq-backend-data" style="vertical-align: bottom">&nbsp;Occurs on&nbsp;
					<?php $this->printDateChooser($prefix.'_start', isset($_REQUEST['SQ_CALENDAR_DATE']) ? strtotime($_REQUEST['SQ_CALENDAR_DATE']) : ($asset->getStartDate() ? iso8601_ts($asset->attr('start_date')) : 0), Array('onkeypress'=>'processKeyEvent(this)', 'onfocus'=>'currentField=this', 'onblur'=>'currentField=null; oldField=this; setTimeout(\'processStartDateBlur(oldField, \\\''.$prefix.'\\\')\', 10);')); ?>
				</td>
				<td class="sq-backend-data">
					&nbsp;&nbsp;<?php $this->printTimeChooser($prefix.'_start',  isset($_REQUEST['SQ_CALENDAR_TIME']) ? strtotime($_REQUEST['SQ_CALENDAR_TIME']) : ($asset->getStartTime() ? iso8601_ts($asset->attr('start_date')) : false), Array('onkeypress'=>'processKeyEvent(this)', 'onfocus'=>'currentField=this', 'onblur'=>'currentField=null; oldField=this; setTimeout(\'processStartDateBlur(oldField, \\\''.$prefix.'\\\')\', 10);'), Array('onclick'=>'if (isChecked(\''.$prefix.'_end_enabled\')) { if (this.checked) enableTimeField(\''.$prefix.'_end\'); else disableTimeField(\''.$prefix.'_end\'); } else { if (this.checked) { checkBox(\''.$prefix.'_end_time_enabled\'); } else { unCheckBox(\''.$prefix.'_end_time_enabled\'); } } ')); ?>
				</td>
			</tr>
		</table>

		<!-- end -->
		<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td class="sq-backend-data">
					<input name="<?php echo $prefix ?>_end_enabled" id="<?php echo $prefix ?>_end_enabled" type="checkbox" <?php if ($asset->getEndDate() || isset($_REQUEST['SQ_CALENDAR_DURATION'])) echo ' checked="checked" '; ?> onclick="processEndClick(this, '<?php echo $prefix; ?>');"/>&nbsp;runs&nbsp;
				</td>
				<!-- duration -->
				<td class="sq-backend-data">
					<input type="radio" value="duration" id="<?php echo $prefix; ?>_duration_enabled" name="<?php echo $prefix; ?>_end_type" <?php if (!$asset->id) echo 'checked="checked"'; ?> <?php echo $end_disabled_code; ?> /> for 
				</td>
				<td colspan="2" class="sq-backend-data">
					<?php text_box($prefix.'_duration', (isset($_REQUEST['SQ_CALENDAR_DURATION']) ? strtok($_REQUEST['SQ_CALENDAR_DURATION'], '_') : ''), 2, 0, 0, 'onclick="checkBox(\''.$prefix.'_duration_enabled\'); checkBox(\''.$prefix.'_start_time_enabled\'); enableTimeField(\''.$prefix.'_start\');"  onblur="updateDurationValues(\''.$prefix.'\')" '.$end_disabled_code);
					combo_box($prefix.'_duration_units', Array('i' => 'Minutes', 'h' => 'Hours'), false, Array('i'), 0,  'onclick="checkBox(\''.$prefix.'_duration_enabled\'); checkBox(\''.$prefix.'_start_time_enabled\'); enableTimeField(\''.$prefix.'_start\');" onblur="updateDurationValues(\''.$prefix.'\')" '.$end_disabled_code); ?>
				</td>
			</tr>
			<tr>
				<td>&nbsp</td>
				<!-- end date -->
				<td class="sq-backend-data">
					<input type="radio"  <?php if ($asset->id) echo 'checked="checked"'; ?> value="date" id="<?php echo $prefix; ?>_end_date_enabled" name="<?php echo $prefix; ?>_end_type" <?php echo $end_disabled_code; ?>  /> until &nbsp;
				</td>
				<td class="sq-backend-data">
					<?php $this->printDateChooser($prefix.'_end', $occurrence_end_ts ? $occurrence_end_ts : ($asset->getEndDate() ? iso8601_ts($asset->attr('end_date')) : false), Array('onclick'=>'checkBox(\''.$prefix.'_end_date_enabled\');', 'onkeypress'=>'processKeyEvent(this)', 'onfocus'=>'currentField=this', 'onblur'=>'currentField=null; oldField=this; setTimeout(\'processEndDateBlur(oldField, \\\''.$prefix.'\\\')\', 10);')+$end_disabled_array); ?>
				</td>
				<td class="sq-backend-data">
					&nbsp;&nbsp;<?php $this->printTimeChooser($prefix.'_end', ($asset->getEndTime() ? iso8601_ts($asset->attr('end_date')) : ($asset->getStartTime() ? 0 : false)), Array('onkeypress'=>'processKeyEvent(this)', 'onfocus'=>'currentField=this', 'onblur'=>'currentField=null; oldField=this; setTimeout(\'processEndDateBlur(oldField, \\\''.$prefix.'\\\')\', 10);')+$end_disabled_array, Array('onclick'=>'if (this.checked) { checkBox(\''.$prefix.'_end_date_enabled\'); enableTimeField(\''.$prefix.'_start\'); } else disableTimeField(\''.$prefix.'_start\');')+$end_disabled_array); ?>
				</td>
			</tr>
		</table>
		<?php
		return true;

	}//end paintStartEndChooser()

	
	function _validateDateTimeFields($prefix, $component) 
	{
		if (isset($_REQUEST[$prefix.$component.'_time_enabled'])) {
			// interpret AM/PM values and covert to 24hr time
			if ($_REQUEST[$prefix.$component.'_is_pm']) {
				if ($_REQUEST[$prefix.$component.'_hours'] != 12) {
					$_REQUEST[$prefix.$component.'_hours'] += 12;
				}
			} else {
				if ($_REQUEST[$prefix.$component.'_hours'] == 12) {
					$_REQUEST[$prefix.$component.'_hours'] = 0;
				}
			}
			$_REQUEST[$prefix.$component.'_hours'] = (int)$_REQUEST[$prefix.$component.'_hours'];
			if (($_REQUEST[$prefix.$component.'_hours'] < 0) || ($_REQUEST[$prefix.$component.'_hours'] > 23)) {
				trigger_error('You entered an invalid hour: '.$_REQUEST[$prefix.$component.'_hours'], E_USER_WARNING);
				return false;
			}
			$_REQUEST[$prefix.$component.'_minutes'] = (int)$_REQUEST[$prefix.$component.'_minutes'];
			if (($_REQUEST[$prefix.$component.'_minutes'] < 0) || ($_REQUEST[$prefix.$component.'_minutes'] > 59)) {
				trigger_error('You entered an invalid minute', E_USER_WARNING);
				return false;
			}
		}
		if (isset($_REQUEST[$prefix.$component.'_year'])) {
			$_REQUEST[$prefix.$component.'_year'] = (int)$_REQUEST[$prefix.$component.'_year'];
			if (($_REQUEST[$prefix.$component.'_year'] < 1900) || ($_REQUEST[$prefix.$component.'_year'] > 2100)) {
				trigger_error('You entered an invalid year value', E_USER_WARNING);
				return false;
			}
			$_REQUEST[$prefix.$component.'_month'] = (int)$_REQUEST[$prefix.$component.'_month'];
			if (($_REQUEST[$prefix.$component.'_month'] < 1) || ($_REQUEST[$prefix.$component.'_month'] > 12)) {
				trigger_error('You entered an invalid month value', E_USER_WARNING);
				return false;
			}
			$_REQUEST[$prefix.$component.'_day'] = (int)$_REQUEST[$prefix.$component.'_day'];
			if (($_REQUEST[$prefix.$component.'_day'] < 1) || ($_REQUEST[$prefix.$component.'_day'] > 31)) {
				trigger_error('You entered an invalid day value', E_USER_WARNING);
				return false;
			}
		}
	}

	/**
	* Process the start/end date/time entry form fields for an event
	*
	* @param object Calendar_Event		&$asset			the event to process the form for
	* @param object Backend_Outputter	&$o				the outputter class (not used here)
	* @param string						$prefix			the prefix to use for form field names
	*
	* @return boolean	Whether the update was successful
	* @access public
	*/
	function processStartEndChooser(&$asset, &$o, $prefix) 
	{
		bam($_REQUEST);

		// begin with the start time
		$this->_validateDateTimeFields($prefix, '_start');
		$start_date_value = $_REQUEST[$prefix.'_start_year'] .'-' 
			. sprintf("%02d", $_REQUEST[$prefix.'_start_month']) .'-' 
			. sprintf("%02d", $_REQUEST[$prefix.'_start_day']).' ';
		if (isset($_REQUEST[$prefix.'_start_time_enabled'])) {
			$start_date_value .= sprintf("%02d", $_REQUEST[$prefix.'_start_hours']).':'
									. sprintf("%02d", $_REQUEST[$prefix.'_start_minutes']).':--';
		} else {
			$start_date_value .= '--:--:--';
		}
		
		// the end is more complex...
		if (isset($_REQUEST[$prefix.'_end_enabled'])) {
			if (!isset($_REQUEST[$prefix.'_end_type'])) {
				trigger_error('End type not specified', E_USER_WARNING);
				return false;
			}
			if ($_REQUEST[$prefix.'_end_type'] == 'duration') {
				if (!isset($_REQUEST[$prefix.'_duration'])) {
					trigger_error('Duration value not specified', E_USER_WARNING);
					return false;
				}
				$start_ts = iso8601_ts($start_date_value);
				switch ($_REQUEST[$prefix.'_duration_units']) {
					case 'i':
						$end_ts = $start_ts + ((int)$_REQUEST[$prefix.'_duration'] * 60);
					break;
					case 'h':
						$end_ts = $start_ts + ((int)$_REQUEST[$prefix.'_duration'] * 60 * 60);
					break;
				}
				$end_date_value = ts_iso8601($end_ts);
			} else {
				$this->_validateDateTimeFields($prefix, '_end');
				if (isset($_REQUEST[$prefix.'_end_time_enabled'])) {
					$end_date_value = $_REQUEST[$prefix.'_end_year'] .'-' 
						. sprintf("%02d", $_REQUEST[$prefix.'_end_month']) .'-' 
						. sprintf("%02d", $_REQUEST[$prefix.'_end_day']).' '
						. sprintf("%02d", $_REQUEST[$prefix.'_end_hours']).':'
						. sprintf("%02d", $_REQUEST[$prefix.'_end_minutes']).':--';
				} else {
					$end_date_value = $_REQUEST[$prefix.'_end_year'] .'-' 
						. sprintf("%02d", $_REQUEST[$prefix.'_end_month']) .'-' 
						. sprintf("%02d", $_REQUEST[$prefix.'_end_day']).' '
						. '--:--:--';
				}
			}
		} else {
			$end_date_value = '---------- --:--:--';
		}

		$res = true;
		$res &= $asset->setAttrValue('start_date', $start_date_value);
		$res &= $asset->setAttrValue('end_date', $end_date_value);
		return $res;

	}//end processStartEndChooser()

	
	/**
	* Print form fields for choosing a date
	*
	* @param string	$prefix			the prefix to use on field names
	* @param int	$timestamp		a default value, or 0 to use today's date, or FALSE to disable the field
	* @param array	$attributes		array of attributes for the input tags, eg 'onclick'=>'alert("hi")'
	*
	* @return boolean
	* @access public
	*/
	function printDateChooser($prefix, $timestamp=0, $attributes=Array())
	{
		/* old attrs: 
			onkeypress="return processKeyEvent(this);" 
			onkeyup="if (dayOK(this)) { <?php echo $onkeyup_code?> }" 
			onblur="if (validateDay(this)) { <?php echo $onchange_code; ?> }" 
			onfocus="this.select()"
		*/
		$select_attributes = $attributes;
		unset($select_attributes['onkeypress']);
		$attrs_code = '';
		foreach ($attributes as $name => $value) $attrs_code .= ' '.$name.'="'.$value.'"';
		if ($timestamp === false) $attrs_code .= ' disabled="disabled"';
		if (!$timestamp) $timestamp = time();
		?>
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $prefix; ?>_day" id="<?php echo $prefix; ?>_day" size="2" value="<?php echo date('j', $timestamp); ?>" <?php echo $attrs_code ?>  />
		
		<select class="sq-form-field" name="<?php echo $prefix; ?>_month" id="<?php echo $prefix; ?>_month" <?php echo $attrs_code; ?>>
		<?php
		$currentMonth = date('n', $timestamp);
		foreach(range(1, 12) as $monthNumber) {
			echo '		<option '.(($monthNumber == $currentMonth) ? ' selected="selected" ' : '').'value="'.$monthNumber.'">'.DateTime_Field::shortMonth($monthNumber)."</option>\n";
		}
		?>
		</select>
		
		<input class="sq-form-field" type="text" name="<?php echo $prefix; ?>_year" id="<?php echo $prefix; ?>_year" size="5" value="<?php echo date('Y', $timestamp); ?>" <?php echo $attrs_code; ?> />
		<?php
		return true;

	}//end printDateChooser()


	/**
	* Print form fields for choosing a date, including a checkbox to enable or disable the time fields
	*
	* @param string	$prefix			the prefix to use on field names
	* @param int	$timestamp		a default value, or 0 to use today's date, or FALSE to disable the field
	* @paran arrat	$attributes		Attributes to put in the field tags, eg 'onclick'=>'alert()'
	*
	* @return boolean
	* @access public
	*/
	function printTimeChooser($prefix, $timestamp, $attributes=Array(), $checkbox_attributes=Array())
	{
		$attrs_code = '';
		foreach ($attributes as $name => $value) $attrs_code .= ' '.$name.'="'.$value.'"';
		if ($timestamp === false) $attrs_code .= ' disabled="disabled"';
		$checked_text = ($timestamp !== false) ? 'checked="checked"' : '';
		if (!$timestamp) $timestamp = time();

		if (!isset($checkbox_attributes['onclick'])) $checkbox_attributes['onclick'] = '';
		$checkbox_attributes['onclick'] = 'if (this.checked) { enableTimeField(\''.$prefix.'\'); } else { disableTimeField(\''.$prefix.'\'); } '.$checkbox_attributes['onclick'];
		$checkbox_attrs_code = '';
		foreach ($checkbox_attributes as $name => $value) $checkbox_attrs_code .= ' '.$name.'="'.$value.'"';


		?>
		<input class="sq-form-field" type="checkbox" <?php echo $checked_text ?> value="1" name="<?php echo $prefix; ?>_time_enabled" id="<?php echo $prefix; ?>_time_enabled" <?php echo $checkbox_attrs_code; ?>" />at
		
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $prefix; ?>_hours" id="<?php echo $prefix; ?>_hours" size="2" maxlength="2" value="<?php echo date('g', $timestamp); ?>" <?php echo $attrs_code; ?> /> :
		
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $prefix; ?>_minutes" id="<?php echo $prefix; ?>_minutes" size="2" maxlength="2" value="<?php echo date('i', $timestamp); ?>" <?php echo $attrs_code; ?> />
		
		<select class="sq-form-field" name="<?php echo $prefix; ?>_is_pm" id="<?php echo $prefix; ?>_is_pm" <?php echo $attrs_code; ?> >
			<option value="0" <?php echo (date('a', $timestamp)=='am') ? ' selected="selected" ' : ''; ?>>am</option>
			<option value="1" <?php echo (date('a', $timestamp)=='pm') ? ' selected="selected" ' : ''; ?>>pm</option>
		</select>
		<?php
		return true;

	}//end printTimeChooser
		

}//end class

?>