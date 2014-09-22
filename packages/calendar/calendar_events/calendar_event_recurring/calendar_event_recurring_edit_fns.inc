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
* $Id: calendar_event_recurring_edit_fns.inc,v 1.31 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once  dirname(__FILE__).'/../../calendar_event/calendar_event_edit_fns.inc';

/**
* Page_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.31 $
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
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Prints the recurrence frequency field of the recurring event
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFrequencyField(&$asset, &$o, $prefix)
	{
		$freq = $asset->attr('frequency');
		if (!$freq) $freq = '   ';
		$stop_date_attr =& $asset->getAttribute('stop_date');
		if (($stopdate = $stop_date_attr->getTimestamp()) == -1) {
			$stopdate = FALSE;
		}

		if ($asset->writeAccess('attributes')) {
			?>
			<script type="text/javascript" src="<?php echo sq_web_path('data').'/asset_types/calendar_event_recurring/js/frequency_chooser.js'?>"></script>

			<table class="sq-calendar-date-table recurring" width="100%" style="white-space: nowrap">
				<tr>

					<!-- Overall Frequency Types -->
					<td style="width: 12ex; vertical-align: top">

						<input type="radio" <?php echo ((($freq{0} == 'D') || !$freq) ? 'checked="checked" ' : '') ?> name="<?php echo $prefix; ?>_frequency" id="<?php echo $prefix; ?>_frequency_daily" value="daily" onkeypress="this.click();" onclick="showFreqOptions('<?php echo $prefix; ?>', 'Daily'); checkBox('<?php echo $prefix?>_frequency_type_DED');" />
						<label for="<?php echo $prefix; ?>_frequency_daily">Daily</label>
						<br />

						<input type="radio" <?php echo (($freq{0} == 'W') ? 'checked="checked" ' : '') ?> name="<?php echo $prefix; ?>_frequency" id="<?php echo $prefix; ?>_frequency_weekly" value="weekly" onkeypress="this.click();" onclick="showFreqOptions('<?php echo $prefix; ?>', 'Weekly'); checkBox('<?php echo $prefix?>_frequency_type_WEW');" />
						<label for="<?php echo $prefix; ?>_frequency_weekly">Weekly</label>
						<br />

						<input type="radio" <?php echo (($freq{0} == 'M') ? 'checked="checked" ' : '') ?> name="<?php echo $prefix; ?>_frequency" id="<?php echo $prefix; ?>_frequency_monthly" value="monthly" onkeypress="this.click();" onclick="showFreqOptions('<?php echo $prefix; ?>', 'Monthly'); checkBox('<?php echo $prefix?>_frequency_type_M');" />
						<label for="<?php echo $prefix; ?>_frequency_monthly">Monthly</label>
						<br />&nbsp;<br />

					</td>

					<td style="vertical-align: top; ">

						<!-- Daily Options -->
						<div id="<?php echo $prefix; ?>DailyOptions" style="display: <?php echo (($freq{0} == 'D') ? 'block' : 'none') ?>">
							<input type="radio" <?php echo ((($freq == 'DED') || !$freq) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_DED" value="DED" /><?php echo translate('Every day'); ?>

							<?php
							// Every 'X' days textbox
							ob_start(); ?>
								<input type="text" size="2" value="<?php echo trim(substr($freq, 3)); ?>" name="<?php echo $prefix; ?>_num_days" onkeypress="this.click();" onclick="checkBox('<?php echo $prefix?>_frequency_type_DEDx');" />
								<?php $x_days_tb = ob_get_contents();
							ob_end_clean(); ?>
							<br /><input type="radio" <?php echo ((($freq != 'DED') && (substr($freq, 0, 3) == 'DED')) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_DEDx" value="DEDx" /><?php echo sprintf(translate('Every %s days'), $x_days_tb); ?>

							<br /><input type="radio" <?php echo (((substr($freq, 0, 3) == 'DWD')) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_DWD" value="DWD" /><?php echo translate('Every weekday'); ?>

							<br /><input type="radio" <?php echo (((substr($freq, 0, 3) == 'DWE')) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_DWE" value="DWE" /><?php echo translate('Every weekend day'); ?>

						</div>

						<!-- Weekly Options -->
						<div id="<?php echo $prefix; ?>WeeklyOptions" style="display: <?php echo (($freq{0} == 'W') ? 'block' : 'none') ?>">
							<?php
							// link to weekday on Week links
							ob_start(); ?>
								<a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix ?>_week_day"></a>
								<?php $week_link = ob_get_contents();
							ob_end_clean();

							// link to weekday on Every 'X' Weeks link
							ob_start(); ?>
								<a id="<?php echo $prefix ?>_week_day_4" href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();"></a>
								<?php $x_weeks_link = ob_get_contents();
							ob_end_clean();

							// Every 'X' weeks textbox
							ob_start(); ?>
								<input type="text" size="2" value="<?php echo substr($freq, 3); ?>" name="<?php echo $prefix ?>_num_weeks" onkeypress="this.click();" onclick="checkBox('<?php echo $prefix?>_frequency_type_WEWx');" />
								<?php $x_weeks_tb = ob_get_contents();
							ob_end_clean(); ?>
							<input type="radio" <?php echo ((($freq == 'WEW') || !$freq) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_WEW" value="WEW" /><?php echo sprintf(translate('Every week on %s'), $week_link); ?>

							<br /><input type="radio" <?php echo ((($freq != 'WEW') && (substr($freq,0,3) == 'WEW')) ? 'checked="checked" ' : '')?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_WEWx" value="WEWx" /><?php echo sprintf(translate('Every %1$s weeks on %2$s'), $x_weeks_tb, $x_weeks_link); ?>

						</div>

						<!-- Monthly options -->
						<table border="0" cellpadding="0" cellspacing="0" id="<?php echo $prefix; ?>MonthlyOptions" style="display: <?php echo (($freq{0} == 'M') ? 'block' : 'none') ?>"><tr>

							<td style="width: 25ex; vertical-align: top">
								<?php
								// Every 'X' months textbox
								ob_start(); ?>
									<input type="text" size="2" value="<?php echo substr($freq, 3); ?>" name="<?php echo $prefix ?>_num_months" onkeypress="this.click();" onclick="checkBox('<?php echo $prefix?>_frequency_type_Mx');" />
									<?php $x_months_tb = ob_get_contents();
								ob_end_clean();
								?>
								<input type="radio" <?php echo ((($freq{0}=='M') && (strlen($freq) == 3)) ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_M" value="M"><?php echo translate('Every month'); ?>

								<br /><input type="radio" <?php echo ((($freq{0}=='M') && (strlen($freq) != 3)) ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_frequency_type" id="<?php echo $prefix; ?>_frequency_type_Mx" value="Mx"><?php echo sprintf(translate('Every %s months'), $x_months_tb); ?>

							</td>
							<td style="width: 60ex">
								<?php
								// link to day number on Every 'X' Months on Day link
								ob_start(); ?>
									<a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_month_date_ord"></a>
									<?php $mfn_month_link = ob_get_contents();
								ob_end_clean(); ?>

								<input type="radio" <?php echo ((($freq{0} != 'M') || (substr($freq, 0, 3) == 'MFN')) ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_monthly_type" value="MFN" /><?php echo sprintf(translate('On the %s day of the month'), $mfn_month_link) ?><span id="<?php echo $prefix; ?>_month_date_warning" style="display: none"><?php echo translate('(when it exists)') ?></span><br />



								<?php
								// link to day number on Every 'X' Months on Weekday link
								ob_start(); ?>
									<a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_month_week_ord"></a>
									<?php $mfw_month_link = ob_get_contents();
								ob_end_clean();

								// link to weekday on Every 'X' Months on Weekday link
								ob_start(); ?>
									<a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_week_day_2"></a>
									<?php $mfw_day_link = ob_get_contents();
								ob_end_clean(); ?>
								  <input type="radio" <?php echo ((substr($freq, 0, 3) == 'MFW') ? 'checked="checked" ' : ''); ?>name="<?php echo $prefix; ?>_monthly_type" value="MFW" /><?php echo sprintf(translate('On the %1$s %2$s of the month'), $mfw_month_link, $mfw_day_link) ?> <span id="<?php echo $prefix; ?>_month_week_warning" style="display: none"><?php echo translate('(when it exists)') ?></span><br />



								<?php
								// link to day number on Every 'X' Months on Last Day link
								ob_start(); ?>
									<a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_reverse_month_date_ord"></a><a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();"><?php echo translate('Last') ?></a>

									<?php $mrn_month_link = ob_get_contents();
								ob_end_clean(); ?>
								<input type="radio" <?php echo ((substr($freq, 0, 3) == 'MRN') ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_monthly_type" value="MRN" /><?php echo sprintf(translate('On the %s last day of the month'), $mrn_month_link) ?> <span id="<?php echo $prefix ?>_reverse_month_date_warning" style="display: none"><?php echo translate('(when it exists)') ?></span><br />



								<?php
								// link to day number on Every 'X' Months on Last Weekday link
								ob_start(); ?>
									<a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_reverse_month_week_ord"></a><a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();"><?php echo translate('Last') ?></a>

									<?php $mrw_month_link = ob_get_contents();
								ob_end_clean();

								// link to weekday on Every 'X' Months on Weekday link
								ob_start(); ?>
									<a href="javascript: document.getElementById('<?php echo $prefix ?>_start_day').focus();" id="<?php echo $prefix; ?>_week_day_3"></a>
									<?php $mrw_day_link = ob_get_contents();
								ob_end_clean(); ?>
								<input type="radio" <?php echo ((substr($freq, 0, 3) == 'MRW') ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_monthly_type" value="MRW" /><?php echo sprintf(translate('On the %1$s last %2$s of the month'), $mrw_month_link, $mrw_day_link) ?> <span id="<?php echo $prefix; ?>_reverse_month_week_warning" style="display: none"><?php echo translate('(when it exists)') ?></span>


							</td>
						</tr></table>

					</td>
				</tr>

				<tr>
					<!-- Stop Date -->
					<td colspan="2">
					  <?php
						ob_start();
						parent::printDateChooser($prefix.'_stop', $stopdate);
						$date_chooser = ob_get_contents();
						ob_end_clean(); ?>
					  <input type="radio" name="<?php echo $prefix; ?>_stop_date_selected" value="0" <?php echo ($stopdate ? '' : 'checked="checked" '); ?> onkeypress="this.click();" onclick="disableDateField('<?php echo $prefix; ?>_stop')" /><?php echo ucfirst(translate('until further notice')); ?>

					  <br /><input type="radio" <?php echo ($stopdate ? 'checked="checked" ' : ''); ?> name="<?php echo $prefix; ?>_stop_date_selected" value="1" onkeypress="this.click();" onclick="enableDateField('<?php echo $prefix; ?>_stop')" /><?php echo ucfirst(sprintf(translate('until %1$s%'), '')).$date_chooser; ?>

					</td>
				</tr>

			</table>

			<script type="text/javascript"><!--
			//<![CDATA[
				updateValues('<?php echo $prefix; ?>');
			//]]> -->
			</script>
			<?php

			hidden_field($prefix.'_frequency_field_printed', 1);

		} else {
			echo $asset->getFriendlyFrequency();
		}

		return TRUE;

	}//end paintFrequencyField()


	/**
	* Processes the recurrence frequency field of the recurring event
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processFrequencyField(&$asset, &$o, $prefix)
	{
		// Do not process further if the Frequency Field interface is not printed to the user.
		// This can occur when a Simple Edit Layout does not include this field.
		if (!(isset($_REQUEST[$prefix.'_frequency_field_printed']) && ($_REQUEST[$prefix.'_frequency_field_printed'] == 1))) {
			return FALSE;
		}

		$res = TRUE;
		if (isset($_REQUEST[$prefix.'_stop_date_selected']) && ($_REQUEST[$prefix.'_stop_date_selected'])) {
			$_REQUEST[$prefix.'_stop_year'] = (int)$_REQUEST[$prefix.'_stop_year'];
			if (($_REQUEST[$prefix.'_stop_year'] < 1900) || ($_REQUEST[$prefix.'_stop_year'] > 2100)) {
				trigger_localised_error('CAL0039', translate('You entered an invalid year value in the \'Stop Date\' field'), E_USER_WARNING)
				return FALSE;
			}
			$_REQUEST[$prefix.'_stop_month'] = (int)$_REQUEST[$prefix.'_stop_month'];
			if (($_REQUEST[$prefix.'_stop_month'] < 1) || ($_REQUEST[$prefix.'_stop_month'] > 12)) {
				trigger_localised_error('CAL0040', translate('You entered an invalid month value in the \'Stop Date\' field'), E_USER_WARNING)
				return FALSE;
			}
			$_REQUEST[$prefix.'_stop_day'] = (int)$_REQUEST[$prefix.'_stop_day'];
			if (($_REQUEST[$prefix.'_stop_day'] < 1) || ($_REQUEST[$prefix.'_stop_day'] > 31)) {
				trigger_localised_error('CAL0041', translate('You entered an invalid day value in the \'Stop Date\' field'), E_USER_WARNING)
				return FALSE;
			}
			$res &= $asset->setAttrValue('stop_date', sprintf('%04d-%02d-%02d', $_REQUEST[$prefix.'_stop_year'],  $_REQUEST[$prefix.'_stop_month'], $_REQUEST[$prefix.'_stop_day']).' --:--:--');
		} else {
			$res &= $asset->setAttrValue('stop_date', '---------- --:--:--');
		}
		$interval = '';

		// do this so it doesn't give undefined index errors in the switch if nothing is set
		if (empty($_REQUEST[$prefix.'_frequency_type'])) {
			$_REQUEST[$prefix.'_frequency_type'] = '';
		}

		switch ($_REQUEST[$prefix.'_frequency_type']) {
			case 'DED':
			case 'DWD':
			case 'DWE':
			case 'WEW':
				$frequency = $_REQUEST[$prefix.'_frequency_type'];
			break;

			case 'DEDx':
				if ((($interval = (int)$_REQUEST[$prefix.'_num_days']) > 0)) {
					$frequency = 'DED'.$interval;
				} else {
					trigger_localised_error('CAL0042', translate('You entered an invalid number of days in the "every x days" field - it must be between 1 and 30'), E_USER_WARNING)
					return FALSE;
				}
			break;

			case 'WEWx':
				if ((($interval = (int)$_REQUEST[$prefix.'_num_weeks']) > 0)) {
					$frequency = 'WEW'.$interval;
				} else {
					trigger_localised_error('CAL0043', translate('You entered an invalid number of weeks in the "every x weeks" field - it must be at least 1'), E_USER_WARNING)
					return FALSE;
				}
			break;

			case 'M':
				if (isset($_REQUEST[$prefix.'_monthly_type']) && in_array($_REQUEST[$prefix.'_monthly_type'], Array('MFN', 'MFW', 'MRN', 'MRW'))) {
					$frequency = $_REQUEST[$prefix.'_monthly_type'];
				} else {
					trigger_localised_error('CAL0044', translate('You entered an invalid type of monthly frequency'), E_USER_WARNING)
					return FALSE;
				}
			break;

			case 'Mx':
				if (in_array($_REQUEST[$prefix.'_monthly_type'], Array('MFN', 'MFW', 'MRN', 'MRW'))) {
					if ((($interval = (int)$_REQUEST[$prefix.'_num_months']) > 0)) {
						$frequency = $_REQUEST[$prefix.'_monthly_type'].$interval;
					} else {
						trigger_localised_error('CAL0045', translate('You entered an invalid number of months in the "every x months" field\' - it must be at least 1'), E_USER_WARNING)
						return FALSE;
					}
				} else {
					trigger_localised_error('CAL0046', translate('You entered an invalid type of monthly frequency'), E_USER_WARNING)
					return FALSE;
				}
			break;

			default:
				return FALSE;
			break;
		}//end switch
		$res &= $asset->setAttrValue('frequency', $frequency);
		return $res;

	}//end processFrequencyField()


	/**
	* Paints the field to enter the start/end dates/times for the first instance
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRecurringStartEndChooser(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$this->paintStartEndChooser($asset, $prefix, Array('onchange'=>'updateValues(\''.$prefix.'\');', 'onkeyup'=>'updateValues(\''.$prefix.'\');'));
		} else {
			echo $asset->getFriendlyDatesAndTimes();
		}
		return TRUE;

	}//end paintRecurringStartEndChooser()


	/**
	* Processes the field containing the start/end dates/times for the first instance
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
