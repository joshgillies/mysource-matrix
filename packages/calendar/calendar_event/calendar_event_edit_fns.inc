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
* $Id: calendar_event_edit_fns.inc,v 1.48 2012/08/30 00:57:28 ewang Exp $
*
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
* @version $Revision: 1.48 $
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
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint a form for entering the start/end date/time for an event, including javascript validation functions
	*
	* @param object	&$asset		the event to print the form for
	* @param string	$prefix		the prefix to use for form field names
	* @param string	$attributes	extra attributes to print for each input field name=>val
	*
	* @return boolean
	* @access public
	*/
	function paintStartEndChooser(&$asset, $prefix, $attributes=Array())
	{
		$occurrence_end_ts = (isset($_REQUEST['SQ_CALENDAR_DATE']) && (!$asset->getEndDate())) ? strtotime($_REQUEST['SQ_CALENDAR_DATE'].' '.($asset->getEndTime() ? $asset->getEndTime() : '12:00:00 GMT')) : FALSE;
		$end_disabled_code = (($asset->getEndTime() || $asset->getEndDate()) || (isset($_REQUEST['SQ_CALENDAR_DURATION']))) ? '' : 'disabled="disabled"';
		$end_disabled_array = $end_disabled_code ? Array('disabled'=>'disabled') : Array();
		$duration = '';
		$duration_units = 'i';
		if ($d = ($asset->getDuration() / 60)) $duration = $d;
		if (isset($_REQUEST['SQ_CALENDAR_DURATION'])) {
			list($duration, $duration_units) = explode('_', $_REQUEST['SQ_CALENDAR_DURATION']);
		}
		if ($duration > 60) {
			$duration = $duration / 60;
			$duration_units = 'h';
			if ($duration > 24) {
				$duration = $duration / 24;
				$duration_units = 'd';
			}
		}

		?>
		<script type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/calendar_event/js/date_chooser.js'?>"></script>

		<!-- start -->
		<table class="sq-calendar-date-table start">
			<tr>
				<td class="sq-backend-data" style="white-space: nowrap">&nbsp;<?php echo translate('Occurs on') ?>&nbsp;

					<?php $this->printDateChooser($prefix.'_start', (isset($_REQUEST['SQ_CALENDAR_DATE']) && !$asset->id) ? strtotime($_REQUEST['SQ_CALENDAR_DATE']) : ($asset->getStartDate() ? iso8601_ts($asset->attr('start_date')) : 0), $attributes+Array('onkeypress'=>'processKeyEvent(this)', 'onfocus'=>'currentField=this', 'onblur'=>'currentField=null; oldField=this; setTimeout(\'processStartDateBlur(oldField, \\\''.$prefix.'\\\')\', 10);')); ?>
				</td>
				<td class="sq-backend-data" style="white-space: nowrap">
					&nbsp;&nbsp;<?php $this->printTimeChooser($prefix.'_start',  isset($_REQUEST['SQ_CALENDAR_TIME']) ? strtotime($_REQUEST['SQ_CALENDAR_TIME']) : ($asset->getStartTime() ? iso8601_ts($asset->attr('start_date')) : FALSE), $attributes+Array('onkeypress'=>'processKeyEvent(this)', 'onfocus'=>'currentField=this', 'onblur'=>'currentField=null; oldField=this; setTimeout(\'processStartDateBlur(oldField, \\\''.$prefix.'\\\')\', 10);'), Array('onclick'=>'if (isChecked(\''.$prefix.'_end_enabled\')) { if (this.checked) enableTimeField(\''.$prefix.'_end\'); else disableTimeField(\''.$prefix.'_end\'); } else { if (this.checked) { checkBox(\''.$prefix.'_end_time_enabled\'); } else { uncheckBox(\''.$prefix.'_end_time_enabled\'); } } ')); ?>
				</td>
			</tr>
		</table>

		<!-- end -->
		<table class="sq-calendar-date-table end">
			<tr>
				<td class="sq-backend-data" style="white-space: nowrap">
					<?php $checked = $asset->getEndDate() || isset($_REQUEST['SQ_CALENDAR_DURATION']); ?>
					<input name="<?php echo $prefix ?>_end_enabled" id="<?php echo $prefix ?>_end_enabled" type="checkbox" <?php if ($checked) echo ' checked="checked" '; ?> onclick="processEndClick(this, '<?php echo $prefix; ?>');"/>&nbsp;runs&nbsp;
				</td>
				<!-- duration -->
				<td class="sq-backend-data" style="white-space: nowrap">
					<input type="radio" value="duration" id="<?php echo $prefix; ?>_duration_enabled" name="<?php echo $prefix; ?>_end_type" <?php if (!$asset->id) echo 'checked="checked"'; ?> <?php echo $end_disabled_code; ?> /> <?php echo strtolower(translate('For')); ?>

				</td>
				<td colspan="2" class="sq-backend-data" style="white-space: nowrap">
					<?php text_box($prefix.'_duration', $duration, 2, 0, 0, 'onclick="checkBox(\''.$prefix.'_duration_enabled\'); checkBox(\''.$prefix.'_start_time_enabled\'); enableTimeField(\''.$prefix.'_start\');"  onblur="updateDurationValuesByPrefix(\''.$prefix.'\')" '.$end_disabled_code);
					combo_box($prefix.'_duration_units', Array('i' => translate('minutes'), 'h' => translate('hours'), 'd' => translate('days')), FALSE, $duration_units, 0,  'onclick="checkBox(\''.$prefix.'_duration_enabled\'); checkBox(\''.$prefix.'_start_time_enabled\'); enableTimeField(\''.$prefix.'_start\');" onblur="updateDurationValuesByPrefix(\''.$prefix.'\')" '.$end_disabled_code); ?>



				</td>
			</tr>
			<tr>
				<td>&nbsp;</td>
				<!-- end date -->
				<td class="sq-backend-data" style="white-space: nowrap">
					<input type="radio"  <?php if ($asset->id) echo 'checked="checked"'; ?> value="date" id="<?php echo $prefix; ?>_end_date_enabled" name="<?php echo $prefix; ?>_end_type" <?php echo $end_disabled_code; ?>  /> <?php echo strtolower(translate('Until')); ?> &nbsp;

				</td>
				<td class="sq-backend-data" style="white-space: nowrap">
					<?php $this->printDateChooser($prefix.'_end', $occurrence_end_ts ? $occurrence_end_ts : ($asset->getEndDate() ? iso8601_ts($asset->attr('end_date')) : FALSE), $attributes+Array('onclick'=>'checkBox(\''.$prefix.'_end_date_enabled\');', 'onkeypress'=>'processKeyEvent(this)', 'onfocus'=>'currentField=this', 'onblur'=>'currentField=null; oldField=this; setTimeout(\'processEndDateBlur(oldField, \\\''.$prefix.'\\\')\', 10);')+$end_disabled_array); ?>
				</td>
				<td class="sq-backend-data" style="white-space: nowrap">
					&nbsp;&nbsp;<?php $this->printTimeChooser($prefix.'_end', ($asset->getEndTime() ? iso8601_ts($asset->attr('end_date')) : ($asset->getStartTime() ? 0 : FALSE)), $attributes+Array('onclick'=>'checkBox(\''.$prefix.'_end_date_enabled\');', 'onkeypress'=>'processKeyEvent(this)', 'onfocus'=>'currentField=this', 'onblur'=>'currentField=null; oldField=this; setTimeout(\'processEndDateBlur(oldField, \\\''.$prefix.'\\\')\', 10);')+$end_disabled_array, Array('onclick'=>'if (this.checked) { checkBox(\''.$prefix.'_end_date_enabled\'); enableTimeField(\''.$prefix.'_start\'); } else disableTimeField(\''.$prefix.'_start\');')+$end_disabled_array); ?>
				</td>
			</tr>
		</table>

		<?php
		hidden_field($prefix.'_start_end_chooser_printed', 1);
		return TRUE;

	}//end paintStartEndChooser()


	/**
	* Check all the date/time components are present and correct, and convert to 24hr format
	*
	* @param string	$prefix		The form field prefix
	* @param string	$component	Which component of the start/end we are checking
	*
	* @return boolean
	* @access private
	*/
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
				trigger_localised_error('CAL0001', sprintf(translate('You entered an invalid hour: %s'), $_REQUEST[$prefix.$component.'_hours']), E_USER_WARNING);
				return FALSE;
			}
			$_REQUEST[$prefix.$component.'_minutes'] = (int)$_REQUEST[$prefix.$component.'_minutes'];
			if (($_REQUEST[$prefix.$component.'_minutes'] < 0) || ($_REQUEST[$prefix.$component.'_minutes'] > 59)) {
				trigger_localised_error('CAL0002', sprintf(translate('You entered an invalid minute: %s'), $_REQUEST[$prefix.$component.'_minutes']), E_USER_WARNING);
				return FALSE;
			}
		}
		if (isset($_REQUEST[$prefix.$component.'_year'])) {
			$_REQUEST[$prefix.$component.'_year'] = (int)$_REQUEST[$prefix.$component.'_year'];
			if (($_REQUEST[$prefix.$component.'_year'] < 1900) || ($_REQUEST[$prefix.$component.'_year'] > 2100)) {
				trigger_localised_error('CAL0003', sprintf(translate('You entered an invalid year value: %s'), $_REQUEST[$prefix.$component.'_year']), E_USER_WARNING);
				return FALSE;
			}
			$_REQUEST[$prefix.$component.'_month'] = (int)$_REQUEST[$prefix.$component.'_month'];
			if (($_REQUEST[$prefix.$component.'_month'] < 1) || ($_REQUEST[$prefix.$component.'_month'] > 12)) {
				trigger_localised_error('CAL0004', sprintf(translate('You entered an invalid month value: %s'), $_REQUEST[$prefix.$component.'_month']), E_USER_WARNING);
				return FALSE;
			}
			$_REQUEST[$prefix.$component.'_day'] = (int)$_REQUEST[$prefix.$component.'_day'];
			if (($_REQUEST[$prefix.$component.'_day'] < 1) || ($_REQUEST[$prefix.$component.'_day'] > 31)) {
				trigger_localised_error('CAL0005', sprintf(translate('You entered an invalid day value: %s'), $_REQUEST[$prefix.$component.'_day']), E_USER_WARNING);
				return FALSE;
			}
		}
		return TRUE;

	}//end _validateDateTimeFields()


	/**
	* Process the start/end date/time entry form fields for an event
	*
	* It returns TRUE if the update was successful
	*
	* @param object	&$asset	the event to process the form for
	* @param object	&$o		the outputter class (not used here)
	* @param string	$prefix	the prefix to use for form field names
	*
	* @return boolean
	* @access public
	*/
	function processStartEndChooser(&$asset, &$o, $prefix)
	{
		if (isset($_REQUEST[$prefix.'_start_end_chooser_printed'])) {
			$values = $this->getStartEndChooserResult($prefix);
			if (FALSE === $values) return FALSE;
			$res = TRUE;
			$res &= $asset->setAttrValue('start_date', $values['start']);
			$res &= $asset->setAttrValue('end_date', $values['end']);
		} else {
			$res = FALSE;
		}

		return $res;

	}//end processStartEndChooser()


	/**
	* Get the result of a start/end chooser fieldset
	*
	* It returns FALSE on failure, array('start'=>iso, 'end'=>iso) on success
	*
	* @param string	$prefix	the prefix to use for form field names
	*
	* @return mixed boolean|array
	* @access public
	*/
	function getStartEndChooserResult($prefix)
	{
		// begin with the start time
		if (!$this->_validateDateTimeFields($prefix, '_start')) {
			return FALSE;
		}
		$start_date_value = $_REQUEST[$prefix.'_start_year'].'-'.sprintf("%02d", $_REQUEST[$prefix.'_start_month']).'-'.sprintf("%02d", $_REQUEST[$prefix.'_start_day']).' ';
		if (isset($_REQUEST[$prefix.'_start_time_enabled'])) {
			$start_date_value .= sprintf("%02d", $_REQUEST[$prefix.'_start_hours']).':'
									.sprintf("%02d", $_REQUEST[$prefix.'_start_minutes']).':00';
		} else {
			$start_date_value .= '--:--:--';
		}

		// the end is more complex...
		if (isset($_REQUEST[$prefix.'_end_enabled'])) {
			if (!isset($_REQUEST[$prefix.'_end_type'])) {
				trigger_localised_error('CAL0006', translate('End type not specified'), E_USER_WARNING);
				return FALSE;
			}
			if ($_REQUEST[$prefix.'_end_type'] == 'duration') {
				if (!isset($_REQUEST[$prefix.'_duration'])) {
					trigger_localised_error('CAL0007', translate('Duration value not specified'), E_USER_WARNING);
					return FALSE;
				}
				$start_ts = iso8601_ts($start_date_value);
				switch ($_REQUEST[$prefix.'_duration_units']) {
					case 'i':
						$end_ts = $start_ts + ((int)$_REQUEST[$prefix.'_duration'] * 60);
					break;
					case 'h':
						$end_ts = $start_ts + ((int)$_REQUEST[$prefix.'_duration'] * 60 * 60);
					break;
					case 'd':
						if (isset($_REQUEST[$prefix.'_start_time_enabled'])) {
							$end_ts = $start_ts + ((int)$_REQUEST[$prefix.'_duration'] * 60 * 60 * 24);
						} else {
							// if no start time, the duration is the number of days covered
							$end_ts = $start_ts + (((int)$_REQUEST[$prefix.'_duration'] - 1) * 60 * 60 * 24);
						}
					break;
				}
				$end_date_value = ts_iso8601($end_ts);
				if (!isset($_REQUEST[$prefix.'_start_time_enabled'])) {
					$end_date_value = substr($end_date_value, 0, 10).' --:--:--';
				}
			} else {
				if (!$this->_validateDateTimeFields($prefix, '_end')) {
					return FALSE;
				}
				if (isset($_REQUEST[$prefix.'_end_time_enabled']) && isset($_REQUEST[$prefix.'_start_time_enabled'])) {
					$end_date_value = $_REQUEST[$prefix.'_end_year'].'-'
						.sprintf("%02d", $_REQUEST[$prefix.'_end_month']).'-'
						.sprintf("%02d", $_REQUEST[$prefix.'_end_day']).' '
						.sprintf("%02d", $_REQUEST[$prefix.'_end_hours']).':'
						.sprintf("%02d", $_REQUEST[$prefix.'_end_minutes']).':00';
				} else {
					$end_date_value = $_REQUEST[$prefix.'_end_year'].'-'
						.sprintf("%02d", $_REQUEST[$prefix.'_end_month']).'-'
						.sprintf("%02d", $_REQUEST[$prefix.'_end_day']).' '
						.'--:--:--';
				}
			}
		} else {
			$end_date_value = '---------- --:--:--';
		}
		return Array('start' => $start_date_value, 'end' => $end_date_value);

	}//end getStartEndChooserResult()


	/**
	* Print form fields for choosing a date
	*
	* @param string	$prefix		the prefix to use on field names
	* @param int	$timestamp	a default value, or 0 to use today's date, or FALSE to disable the field
	* @param array	$attributes	array of attributes for the input tags, eg 'onclick'=>'alert("hi")'
	*
	* @return boolean
	* @access public
	*/
	function printDateChooser($prefix, $timestamp=0, $attributes=Array())
	{
		$select_attributes = $attributes;
		unset($select_attributes['onkeypress']);
		$attrs_code = '';
		foreach ($attributes as $name => $value) {
			$attrs_code .= ' '.$name.'="'.$value.'"';
		}
		if ($timestamp === FALSE) {
			if(!isset($attributes['disabled'])){
			$attrs_code .= ' disabled="disabled"';
			}
		}
		if (!$timestamp) $timestamp = time();
		?>
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $prefix; ?>_day" id="<?php echo $prefix; ?>_day" size="2" value="<?php echo date('j', $timestamp); ?>" <?php echo $attrs_code ?>  />

		<select class="sq-form-field" name="<?php echo $prefix; ?>_month" id="<?php echo $prefix; ?>_month" <?php echo $attrs_code; ?>>
		<?php
		$currentMonth = date('n', $timestamp);
		foreach (range(1, 12) as $monthNumber) {
			echo '		<option '.(($monthNumber == $currentMonth) ? ' selected="selected" ' : '').'value="'.$monthNumber.'">'.DateTime_Field::shortMonth($monthNumber)."</option>\n";
		}
		?>
		</select>

		<input class="sq-form-field" type="text" name="<?php echo $prefix; ?>_year" id="<?php echo $prefix; ?>_year" size="5" value="<?php echo date('Y', $timestamp); ?>" <?php echo $attrs_code; ?> />
		<?php
		return TRUE;

	}//end printDateChooser()


	/**
	* Print form fields for choosing a date, including a checkbox to enable or disable the time fields
	*
	* @param string	$prefix					the prefix to use on field names
	* @param int	$timestamp				a default value, or 0 to use today's date, or FALSE to disable the field
	* @param array	$attributes				Attributes to put in the field tags, eg 'onclick'=>'alert()'
	* @param array	$checkbox_attributes	Attributes to put in the field tags, eg 'onclick'=>'alert()' for checkbox
	*
	* @return boolean
	* @access public
	*/
	function printTimeChooser($prefix, $timestamp, $attributes=Array(), $checkbox_attributes=Array())
	{
		$attrs_code = '';
		foreach ($attributes as $name => $value) {
			$attrs_code .= ' '.$name.'="'.$value.'"';
		}
		if ($timestamp === FALSE) {
			if(!isset($attributes['disabled'])){
			$attrs_code .= ' disabled="disabled"';
			}
		}
		$checked_text = ($timestamp !== FALSE) ? 'checked="checked"' : '';
		if (!$timestamp) $timestamp = time();

		if (!isset($checkbox_attributes['onclick'])) {
			$checkbox_attributes['onclick'] = '';
		}
		$checkbox_attributes['onclick'] = 'if (this.checked) { enableTimeField(\''.$prefix.'\'); } else { disableTimeField(\''.$prefix.'\'); } '.$checkbox_attributes['onclick'];
		$checkbox_attrs_code = '';
		foreach ($checkbox_attributes as $name => $value) {
			$checkbox_attrs_code .= ' '.$name.'="'.$value.'"';
		}
		?>
		<input class="sq-form-field" type="checkbox" <?php echo $checked_text ?> value="1" name="<?php echo $prefix; ?>_time_enabled" id="<?php echo $prefix; ?>_time_enabled" <?php echo $checkbox_attrs_code; ?> />at

		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $prefix; ?>_hours" id="<?php echo $prefix; ?>_hours" size="2" maxlength="2" value="<?php echo date('g', $timestamp); ?>" <?php echo $attrs_code; ?> /> :

		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $prefix; ?>_minutes" id="<?php echo $prefix; ?>_minutes" size="2" maxlength="2" value="<?php echo date('i', $timestamp); ?>" <?php echo $attrs_code; ?> />

		<select class="sq-form-field" name="<?php echo $prefix; ?>_is_pm" id="<?php echo $prefix; ?>_is_pm" <?php echo $attrs_code; ?> >
			<option value="0" <?php echo (date('a', $timestamp)=='am') ? ' selected="selected" ' : ''; ?>>am</option>
			<option value="1" <?php echo (date('a', $timestamp)=='pm') ? ' selected="selected" ' : ''; ?>>pm</option>
		</select>
		<?php
		return TRUE;

	}//end printTimeChooser()


}//end class

?>
