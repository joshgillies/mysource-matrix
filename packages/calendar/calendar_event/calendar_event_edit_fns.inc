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
* $Id: calendar_event_edit_fns.inc,v 1.12 2004/09/01 07:17:25 tbarrett Exp $
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
* @version $Revision: 1.12 $ - 1.0
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
	* @param string					$onchange_code	javascript code to add to the fields' onchange attributes
	*
	* @return boolean
	* @access public
	*/
	function paintStartEndChooser(&$asset, $prefix, $onkeyup_code = '', $onchange_code='') 
	{
		$occurrence_end_ts = (isset($_REQUEST['SQ_CALENDAR_DATE']) && ($asset->getEndDate())) ? strtotime($_REQUEST['SQ_CALENDAR_DATE'].' '.($asset->getEndTime() ? $asset->getEndTime() : '12:00:00 GMT')) : false;
		?>
		<script language="JavaScript" type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/calendar_event/js/date_chooser.js'?>"></script>
		<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td class="sq-backend-data" style="text-align: right">Occurs on&nbsp;&nbsp;</td>
				<td class="sq-backend-data">
					<?php $this->printDateChooser($prefix.'_start', $onkeyup_code, "updateEndDate('$prefix');  $onchange_code", isset($_REQUEST['SQ_CALENDAR_DATE']) ? strtotime($_REQUEST['SQ_CALENDAR_DATE']) : ($asset->getStartDate() ? iso8601_ts($asset->attr('start_date')) : 0)); ?>
				</td>
				<td class="sq-backend-data">
					&nbsp;&nbsp;<?php $this->printTimeChooser($prefix.'_start', "processStartTimeClick(this, '$prefix'); ", $onkeyup_code, "updateEndDate('$prefix'); $onchange_code",  ($asset->getStartTime() ? iso8601_ts($asset->attr('start_date')) : false)); ?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data" style="text-align: right">
					<input type="checkbox" value="1" <?php if ($asset->getEndDate()) echo ' checked="checked" '; ?> id="<?php echo $prefix; ?>_end_date_enabled" name="<?php echo $prefix; ?>_end_date_enabled" onclick="processEndEnabledClick(this, '<?php echo $prefix; ?>')" onkeypress="processEndEnabledClick(this, '<?php echo $prefix; ?>')" />runs until&nbsp;&nbsp;
				</td>
				<td class="sq-backend-data">
					<?php $this->printDateChooser($prefix.'_end', $onkeyup_code, "updateStartDate('$prefix');  $onchange_code", $occurrence_end_ts ? $occurrence_end_ts : ($asset->getEndDate() ? iso8601_ts($asset->attr('end_date')) : false)); ?>
				</td>
				<td class="sq-backend-data">
					&nbsp;&nbsp;<?php $this->printTimeChooser($prefix.'_end', "processEndTimeClick(this, '$prefix'); ", $onkeyup_code, "updateStartDate('$prefix'); $onchange_code",  ($asset->getEndTime() ? iso8601_ts($asset->attr('end_date')) : false)); ?>
				</td>
			</tr>
		</table>
		<?php
		return true;

	}//end paintStartEndChooser()

	
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
		foreach (Array('_start', '_end') as $component) {
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

		$start_date_value = $_REQUEST[$prefix.'_start_year'] .'-' 
			. sprintf("%02d", $_REQUEST[$prefix.'_start_month']) .'-' 
			. sprintf("%02d", $_REQUEST[$prefix.'_start_day']).' ';
		if (isset($_REQUEST[$prefix.'_start_time_enabled'])) {
			$start_date_value .= sprintf("%02d", $_REQUEST[$prefix.'_start_hours']).':'
									. sprintf("%02d", $_REQUEST[$prefix.'_start_minutes']).':--';
		} else {
			$start_date_value .= '--:--:--';
		}

		if (isset($_REQUEST[$prefix.'_end_time_enabled'])) {
			$end_date_value = $_REQUEST[$prefix.'_end_year'] .'-' 
				. sprintf("%02d", $_REQUEST[$prefix.'_end_month']) .'-' 
				. sprintf("%02d", $_REQUEST[$prefix.'_end_day']).' '
				. sprintf("%02d", $_REQUEST[$prefix.'_end_hours']).':'
				. sprintf("%02d", $_REQUEST[$prefix.'_end_minutes']).':--';
		} else {
			if (isset($_REQUEST[$prefix.'_end_date_enabled'])) {
				$end_date_value = $_REQUEST[$prefix.'_end_year'] .'-' 
					. sprintf("%02d", $_REQUEST[$prefix.'_end_month']) .'-' 
					. sprintf("%02d", $_REQUEST[$prefix.'_end_day']).' '
					. '--:--:--';
			} else {
				$end_date_value = '---------- --:--:--';
			}
		}
		$res = true;
		$res &= $asset->setAttrValue('start_date', $start_date_value);
		$res &= $asset->setAttrValue('end_date', $end_date_value);
		return $res;

	}//end processStartEndChooser

	
	/**
	* Print form fields for choosing a date
	*
	* @param string	$prefix			the prefix to use on field names
	* @param string $onchange_code	extra javascript code to put in the fields' onchange attributes
	* @param int	$timestamp		a default value, or 0 to use today's date, or FALSE to disable the field
	*
	* @return boolean
	* @access public
	*/
	function printDateChooser($prefix, $onkeyup_code, $onchange_code, $timestamp)
	{
		$disabled_text = ($timestamp === false) ? ' disabled="disabled" ' : '';
		if (!$timestamp) $timestamp = time();
		?>
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $prefix; ?>_day" id="<?php echo $prefix; ?>_day" size="2" value="<?php echo date('j', $timestamp); ?>" <?php echo $disabled_text; ?> onkeypress="return processKeyEvent(this);" onkeyup="if (dayOK(this)) { <?php echo $onkeyup_code?> }" onblur="if (validateDay(this)) { <?php echo $onchange_code; ?> }" onfocus="this.select()" />
		
		<select class="sq-form-field" name="<?php echo $prefix; ?>_month" id="<?php echo $prefix; ?>_month" <?php echo $disabled_text; ?> onblur="<?php echo $onchange_code; ?>">
		<?php
		$currentMonth = date('n', $timestamp);
		foreach(range(1, 12) as $monthNumber) {
			echo '		<option '.(($monthNumber == $currentMonth) ? ' selected="selected" ' : '').'value="'.$monthNumber.'">'.DateTime_Field::shortMonth($monthNumber)."</option>\n";
		}
		?>
		</select>
		
		<input class="sq-form-field" type="text" name="<?php echo $prefix; ?>_year" id="<?php echo $prefix; ?>_year" size="5" value="<?php echo date('Y', $timestamp); ?>" <?php echo $disabled_text; ?> onkeypress="return processKeyEvent(this);" onkeyup="if (yearOK(this)) { <?php echo $onkeyup_code; ?> }" onblur="if (validateYear(this)) { <?php echo $onchange_code; ?> }" onfocus="this.select()" />
		<?php
		return true;

	}//end printDateChooser


	/**
	* Print form fields for choosing a date, including a checkbox to enable or disable the time fields
	*
	* @param string	$prefix			the prefix to use on field names
	* @param string $onchange_code	extra javascript code to put in the fields' onchange attributes
	* @param string $onclick_code	extra javascript code to run when the checkbox is clicked
	* @param int	$timestamp		a default value, or 0 to use today's date, or FALSE to disable the field
	*
	* @return boolean
	* @access public
	*/
	function printTimeChooser($prefix, $onclick_code, $onkeyup_code, $onchange_code, $timestamp)
	{
		$disabled_text = ($timestamp === false) ? ' disabled="disabled" ' : '';
		$checked_text = ($timestamp === false) ? '' : ' checked="checked" ';
		if (!$timestamp) $timestamp = time();

		?>
		<input class="sq-form-field" type="checkbox" value="1" <?php echo $checked_text; ?> name="<?php echo $prefix; ?>_time_enabled" id="<?php echo $prefix; ?>_time_enabled" onclick="<?php echo $onclick_code; ?>; " onkeypress="<?php echo $onclick_code; ?>" />at
		
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $prefix; ?>_hours" id="<?php echo $prefix; ?>_hours" size="2" maxlength="2" value="<?php echo date('g', $timestamp); ?>" onkeypress="return processKeyEvent(this);" onkeyup="if (hoursOK(this)) { <?php echo $onkeyup_code; ?> }" onblur="if (validateHours(this)) { <?php echo $onchange_code; ?> }" <?php echo $disabled_text; ?> onfocus="this.select()" /> :
		
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $prefix; ?>_minutes" id="<?php echo $prefix; ?>_minutes" size="2" maxlength="2" value="<?php echo date('i', $timestamp); ?>" onkeypress="processKeyEvent(this); this.value=make2digits(this.value); return true;" onkeyup="if (minutesOK(this)) { <?php echo $onkeyup_code; ?> }" onchange="if (validateMinutes(this)) { <?php echo $onchange_code; ?> }" <?php echo $disabled_text; ?> onfocus="this.select()" />
		
		<select class="sq-form-field" name="<?php echo $prefix; ?>_is_pm" id="<?php echo $prefix; ?>_is_pm" onblur="<?php echo $onchange_code; ?>" <?php echo $disabled_text; ?> >
			<option value="0" <?php echo (date('a', $timestamp)=='am') ? ' selected="selected" ' : ''; ?>>am</option>
			<option value="1" <?php echo (date('a', $timestamp)=='pm') ? ' selected="selected" ' : ''; ?>>pm</option>
		</select>
		<?php
		return true;

	}//end printTimeChooser
		

}//end class

?>