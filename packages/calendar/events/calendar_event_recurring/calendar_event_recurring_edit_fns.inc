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
* $Id: calendar_event_recurring_edit_fns.inc,v 1.5 2004/07/27 02:30:09 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/


require_once  dirname(__FILE__).'/../../event/calendar_event_edit_fns.inc';

/**
* Page_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.5 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Recurring_Edit_Fns extends Calendar_Event_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Recurring_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end Calendar_Event_Recurring_Edit_Fns()


	/**
	* Prints the recurrence frequency field of the recurring event
	*
	* @param object Calendar_Asset_Recurring	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFrequencyField(&$asset, &$o, $prefix)
	{
		$freq = $asset->attr('frequency');
		if (!$freq) $freq = '   ';
		$stop_date_attr = $asset->getAttribute('stop_date');
		if (($stopdate = $stop_date_attr->getTimestamp()) == -1) $stopdate = FALSE;
		
		if ($asset->writeAccess('attributes')) {
			?>
			<script>
				var weekDays = Array('Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');
				var months = Array('January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December');
				var monthLengths = Array(0,31,28,31,30,31,30,31,31,30,31,30,31);
				var suffixes = Array('th','st','nd','rd','th','th','th','th','th','th');
				function setSpanContents(eltId, text) {
					elt = document.getElementById(eltId);
					while (elt.childNodes.length > 0)
						elt.removeChild(elt.firstChild);
					elt.appendChild(document.createTextNode(text));
				}
				function ordinalSuffix(num) {
					if ((10 < num) && (num < 20))
						return num + 'th';
					else
						return num + suffixes[num % 10];
				}
				// prints nothing if num = 1
				// otherwise appends a space
				function reverseOrdinalSuffix(num) {
					if (num == 1)
						return '';
					else if ((10 < num) && (num < 20))
						return num + 'th ';
					else
						return num + suffixes[num % 10] + ' ';
				}
				function daysInFebruary(year){
					// February has 29 days in any year evenly divisible by four,
					// EXCEPT for centennial years which are not also divisible by 400.
					return (((year % 4 == 0) && ( (!(year % 100 == 0)) || (year % 400 == 0))) ? 29 : 28 );
				}

				function getMonthLength(month, year) {
					if (month == 2)
						return daysInFebruary(year);
					else
						return monthLengths[month];
				}
				function updateValues(prefix) {
					d = new Date(document.getElementById(prefix + '_start_year').value, document.getElementById(prefix + '_start_month').value - 1, document.getElementById(prefix + '_start_day').value);

					document.getElementById(prefix + '_month_date_warning').style.display = 'none';
					document.getElementById(prefix + '_month_week_warning').style.display = 'none';
					document.getElementById(prefix + '_reverse_month_date_warning').style.display = 'none';
					document.getElementById(prefix + '_reverse_month_week_warning').style.display = 'none';

					setSpanContents(prefix + '_week_day', weekDays[d.getDay()]);
					setSpanContents(prefix + '_week_day_2', weekDays[d.getDay()]);
					setSpanContents(prefix + '_week_day_3', weekDays[d.getDay()]);
					setSpanContents(prefix + '_week_day_4', weekDays[d.getDay()]);

					setSpanContents(prefix + '_month_date_ord', ordinalSuffix(d.getDate()));
					if (d.getDate() > 28)
						document.getElementById(prefix + '_month_date_warning').style.display = 'inline';
					var weekNumber = parseInt(((d.getDate()-1)/7)+1);
					setSpanContents(prefix + '_month_week_ord', ordinalSuffix(weekNumber));

					if (weekNumber > 4)
						document.getElementById(prefix + '_month_week_warning').style.display = 'inline';
					var reverseDate = getMonthLength(d.getMonth()+1, d.getFullYear()) - d.getDate()+1;
					setSpanContents(prefix + '_reverse_month_date_ord', reverseOrdinalSuffix(reverseDate));
					if (reverseDate > 28)
						document.getElementById(prefix + '_reverse_month_date_warning').style.display = 'inline';

					reverseWeekNumber = parseInt((getMonthLength(d.getMonth()+1, d.getFullYear()) - d.getDate()) / 7)+1;
					setSpanContents(prefix + '_reverse_month_week_ord', reverseOrdinalSuffix(reverseWeekNumber));
					if (reverseWeekNumber > 4)
						document.getElementById(prefix + '_reverse_month_week_warning').style.display = 'inline';

				}			

				var freqs = Array('Daily', 'Weekly', 'Monthly');
						
				function showFreqOptions(prefix, freqName) {
					updateValues(prefix);
					for (i=0; i<freqs.length; i++) {
						if ((elt = document.getElementById(prefix+freqs[i]+'Options')) != null) {
							if (freqs[i] == freqName)
								elt.style.display = 'block';
							else
								elt.style.display = 'none';
						} else {
							alert("Javascript error: Could not find frequency: " + freqs[i]);
						}
					}
				}
			</script>
		<table border="0" cellpadding="0" cellspacing="0" width="100%">
			<tr style="height: 11ex">
				<td style="width: 12ex; vertical-align: top">
					<input type="radio" <?php echo ((($freq{0} == 'D') || !$freq)? 'checked="checked" ' : '') ?> name="<?php echo $prefix; ?>_frequency" value="daily" onFocus="showFreqOptions('<?php echo $prefix; ?>', 'Daily'); checkBox('<?php echo $prefix?>_frequency_type_DED');" />Daily<br />
					<input type="radio" <?php echo (($freq{0} == 'W') ? 'checked="checked" ' : '') ?>name="<?php echo $prefix; ?>_frequency" value="weekly" onFocus="showFreqOptions('<?php echo $prefix; ?>', 'Weekly'); checkBox('<?php echo $prefix?>_frequency_type_WEW');" />Weekly<br />
					<input type="radio" <?php echo (($freq{0} == 'M') ? 'checked="checked" ' : '') ?> name="<?php echo $prefix; ?>_frequency" value="monthly" onFocus="showFreqOptions('<?php echo $prefix; ?>', 'Monthly'); checkBox('<?php echo $prefix?>_frequency_type_M');" />Monthly<br />&nbsp;<br />
				</td>

				<td style="vertical-align: top; ">
				  
				  <div id="<?php echo $prefix; ?>DailyOptions" style="display: <?php echo (($freq{0} == 'D') ? 'block' : 'none') ?>">
					<input type="radio" <?php echo ((($freq == 'DED') || !$freq) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_DED" value="DED" />Every Day
					<br /><input type="radio" <?php echo ((($freq != 'DED') && (substr($freq, 0, 3) == 'DED')) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_DEDx" value="DEDx" />Every <input type="text" size="2" value="<?php echo trim(substr($freq, 3)); ?>" name="<?php echo $prefix; ?>_num_days" onfocus="checkBox('<?php echo $prefix?>_frequency_type_DEDx');" /> Days
					<br /><input type="radio" <?php echo (((substr($freq, 0, 3) == 'DWD')) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" value="DWD" />Every WeekDay
				  </div>

				  <div id="<?php echo $prefix; ?>WeeklyOptions" style="display: <?php echo (($freq{0} == 'W') ? 'block' : 'none') ?>">
					<input type="radio" <?php echo ((($freq == 'WEW') || !$freq) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_WEW" value="WEW" />Every week on <a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix ?>_week_day"></a> 
					<br /><input type="radio" <?php echo ((($freq != 'WEW') && (substr($freq,0,3) == 'WEW')) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_WEWx" value="WEWx" />Every <input type="text" size="2" value="<?php echo substr($freq, 3); ?>" name="<?php echo $prefix ?>_num_weeks" onfocus="checkBox('<?php echo $prefix?>_frequency_type_WEWx');" /> weeks on <a id="<?php echo $prefix ?>_week_day_4" href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();"></a> 
				  </div>

				  <table border="0" cellpadding="0" cellspacing="0" id="<?php echo $prefix; ?>MonthlyOptions" style="display: <?php echo (($freq{0} == 'M') ? 'block' : 'none') ?>"><tr>
					<td style="width: 25ex; vertical-align: top">
					  <input type="radio" <?php echo ((($freq{0}=='M') && (strlen($freq) == 3)) ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_M" value="M">Every Month
					  <br /><input type="radio" <?php echo ((($freq{0}=='M') && (strlen($freq) != 3)) ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_Mx" value="Mx">Every <input type="text" size="2" value="<?php echo substr($freq, 3); ?>" name="<?php echo $prefix ?>_num_months" onfocus="checkBox('<?php echo $prefix?>_frequency_type_Mx');" /> months
					</td>
					<td style="width: 60ex">
					  <input type="radio" <?php echo ((!$freq || (substr($freq, 0, 3) == 'MFN')) ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_monthly_type" value="MFN" />On the <a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_month_date_ord"></a>&nbsp;day of the month <span id="<?php echo $prefix; ?>_month_date_warning" style="display: none">(when it exists)</span><br />
					  
					  <input type="radio" <?php echo ((substr($freq, 0, 3) == 'MFW') ? 'checked="checked" ' : ''); ?>name="<?php echo $prefix; ?>_monthly_type" value="MFW" />On the <a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_month_week_ord"></a>&nbsp;<a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_week_day_2"></a>&nbsp;of the month <span id="<?php echo $prefix; ?>_month_week_warning" style="display: none">(when it exists)</span><br />
					  
					  <input type="radio" <?php echo ((substr($freq, 0, 3) == 'MRN') ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_monthly_type" value="MRN" />On the <a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_reverse_month_date_ord"></a><a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();">last</a> day of the month <span id="<?php echo $prefix ?>_reverse_month_date_warning" style="display: none">(when it exists)</span><br />
					  
					  <input type="radio" <?php echo ((substr($freq, 0, 3) == 'MRW') ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_monthly_type" value="MRW" />On the <a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_reverse_month_week_ord"></a><a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();">last</a> <a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_week_day_3"></a>&nbsp;of the month <span id="<?php echo $prefix; ?>_reverse_month_week_warning" style="display: none">(when it exists)</span>
					</td>
				  </tr></table>
				</td>
			  </tr>
			  <tr>
				<td colspan="2">
				  <input type="radio" name="<?php echo $prefix; ?>_stop_date_selected" value="0" <?php echo ($stopdate ? '' : 'checked="checked" '); ?> onFocus="disableDateField('<?php echo $prefix; ?>_stop')" />Until further notice
				  <br /><input type="radio" <?php echo ($stopdate ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_stop_date_selected" value="1" onFocus="enableDateField('<?php echo $prefix; ?>_stop')"" />Until <?php parent::printDateChooser($prefix.'_stop', '', $stopdate); ?>
				</td>
			  </tr>
			</table>
			<script language="JavaScript">
			<!--
				updateValues('<?php echo $prefix; ?>');
			//-->
			</script>
			<?php
		} else {
			echo $asset->getFriendlyFrequency();
		}
					  
	}//end printFrequencyField()


	/**
	* Processes the recurrence frequency field of the recurring event
	*
	* @param object Calendar_Asset_Recurring	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processFrequencyField(&$asset, &$o, $prefix)
	{
		$res = true;
		if (isset($_REQUEST[$prefix . '_stop_date_selected']) && ($_REQUEST[$prefix . '_stop_date_selected'])) {
			$_REQUEST[$prefix . '_stop_year'] = (int)$prefix . '_stop_year';
			if (($_REQUEST[$prefix . '_stop_year'] < 1900) || ($_REQUEST[$prefix . '_stop_year'] > 2100)) {
				trigger_error('You entered an invalid year value in the stop date field', E_USER_WARNING);
				return false;
			}
			$_REQUEST[$prefix . '_stop_month'] = (int)$prefix . '_stop_month';
			if (($_REQUEST[$prefix . '_stop_month'] < 1) || ($_REQUEST[$prefix . '_stop_month'] > 12)) {
				trigger_error('You entered an invalid month value in the stop date field', E_USER_WARNING);
				return false;
			}
			$_REQUEST[$prefix . '_stop_day'] = (int)$prefix . '_stop_day';
			if (($_REQUEST[$prefix . '_stop_day'] < 1) || ($_REQUEST[$prefix . '_stop_day'] > 31)) {
				trigger_error('You entered an invalid day value in the stop date field', E_USER_WARNING);
				return false;
			}
			$res &= $asset->setAttrValue('stop_date', $_REQUEST[$prefix . '_stop_year'] .'-' 
					. sprintf("%02d", $_REQUEST[$prefix . '_stop_month']) .'-' 
					. sprintf("%02d", $_REQUEST[$prefix . '_stop_day']) . ' --:--:--');
		}
		$interval = '';
		switch ($_REQUEST[$prefix . '_frequency_type']) {
			case 'DED':
			case 'DWD':
			case 'WEW':
				$frequency = $_REQUEST[$prefix . '_frequency_type'];
			break;

			case 'DEDx':
				if ((($interval = (int)$_REQUEST[$prefix . '_num_days']) > 0)) {
					$frequency = 'DED' . $interval;
				} else {
					trigger_error('You entered an invalid number of days in the "every x days" field.  It must be between 1 and 30.', E_USER_WARNING);
					return false;
				}
			break;

			case 'WEWx':
				if ((($interval = (int)$_REQUEST[$prefix . '_num_weeks']) > 0)) {
					$frequency = 'WEW' . $interval;
				} else {
					trigger_error('You entered an invalid number of weeks in the "every x weeks" field.', E_USER_WARNING);
					return false;
				}
			break;

			case 'M':
				if (isset($_REQUEST[$prefix . '_monthly_type']) && in_array($_REQUEST[$prefix . '_monthly_type'], Array('MFN', 'MFW', 'MRN', 'MRW'))) {
					$frequency = $_REQUEST[$prefix . '_monthly_type'];
				} else {
					trigger_error('You entered an invalid type of monthly frequency', E_USER_WARNING);
					return false;
				}
			break;

			case 'Mx':
				if (in_array($_REQUEST[$prefix . '_monthly_type'], Array('MFN', 'MFW', 'MRN', 'MRW'))) {
					if ((($interval = (int)$_REQUEST[$prefix . '_num_months']) > 0)) {
						$frequency = $_REQUEST[$prefix . '_monthly_type'] . $interval;
					} else {
						trigger_error('You entered an invalid number of months in the "every x months" field', E_USER_WARNING);
						return false;
					}
				} else {
					trigger_error('You entered an invalid type of monthly frequency', E_USER_WARNING);
					return false;
				}	
			break;

			default:
				trigger_error('You entered an invalid frequency type', E_USER_WARNING);
				return false;
			break;
		}
		$res &= $asset->setAttrValue('frequency', $frequency);
		return $res;

	}//end processFrequencyField()


	/**
	* Paints the field to enter the start/end dates/times for the first instance
	*
	* @param object Calendar_Asset_Recurring	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRecurringStartEndChooser(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes'))
			$this->paintStartEndChooser($asset, $prefix, "updateValues('$prefix');");
		else
			echo $asset->getFriendlyDatesAndTimes();

	}//end paintRecurringStartEndChooser()


	/**
	* Processes the field containing the start/end dates/times for the first instance
	*
	* @param object Calendar_Asset_Recurring	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter			&$o		the outputter class
	* @param string								$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRecurringStartEndChooser(&$asset, &$o, $prefix)
	{
		$this->processStartEndChooser($asset, $o, $prefix);

	}//end processRecurringStartEndChooser()


}//end class

?>