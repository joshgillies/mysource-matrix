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
* $Id: cron_job_edit_fns.inc,v 1.19 2005/03/17 23:02:15 amiller Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Cron_Job_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Cron_Job_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens = Array(
									  'details' => Array(
														 'name' => 'View Jobs',
														 'force_unlock' => true
														),
									  'logs'    => Array(
														 'name' => 'Logs',
														 'force_unlock' => true,
														),
									 );
	}//end constructor


	/**
	* Prints out the cron job specific interface for the a date box that we can use
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	* @param string						$when    prepared when string
	* @param string						$form_extra_elements any additional elements for the form entries
	*
	* @access private
	*/
	function _paintWhenBox(&$asset, &$o, $prefix, $when='', $form_extra_elements='')
	{
		$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return false;

		if ((int) $cron_mgr->attr('refresh_time') <= 0) {
			echo 'The Cron Manager refresh time is unknown';
			return;
		}

		$write_access = $asset->writeAccess('attributes');

		$prefix .= '_when';

		$one_off = ($asset->attr('type') == 'one_off');
		$repeating = !$one_off; // for easier reading

		$cron_mgr_edit_fns = $cron_mgr->getEditFns();
		$hours_mins = $cron_mgr_edit_fns->getWhenBoxHoursMins($cron_mgr, $one_off);

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);

		$when_str = ($when == '') ? $asset->attr('when') : $when;
		$when_arr = Cron_Job::getWhenArr($when_str);
		
		$now = getdate();

		$days = Array('--' => '--');
		if ($repeating) {
			for ($i = 0; $i <= 6; $i++) $days[(string) $i] = Cron_Job::whenWeekDayName($i);
		}
		for ($i = 1; $i <= 31; $i++) $days[str_pad($i, 2, '0', STR_PAD_LEFT)] = str_pad($i, 2, '0', STR_PAD_LEFT);
		if ($write_access) {
			combo_box($prefix.'_day', $days, false, $when_arr['day'], null, $form_extra_elements);
		} else {
			echo $days[$when_arr['day']];
		}

		echo '&nbsp;/&nbsp;';

		if ($write_access) {
			$months = Array('--' => '--');
			for ($i = 1; $i <= 12; $i++) $months[str_pad($i, 2, '0', STR_PAD_LEFT)] = date('M', mktime(0,0,0,$i,1,2000));
			combo_box($prefix.'_month', $months, false, $when_arr['month'], null, $form_extra_elements);
		} else {
			echo $when_arr['month'];
		}

		if ($one_off) {
			echo '&nbsp;/&nbsp;';
			if ($write_access) {
				$years = Array('--' => '--');
				$end_year = max($when_arr['year'], $now['year']) + 10;
				for ($i = $now['year']; $i <= $end_year; $i++) $years[str_pad($i, 4, '0', STR_PAD_LEFT)] = str_pad($i, 4, '0', STR_PAD_LEFT);
				combo_box($prefix.'_year', $years, false, $when_arr['year'], null, $form_extra_elements);
			} else {
				echo $when_arr['year'];
			}
		}

		echo '&nbsp;&nbsp;&nbsp;';
		
		if ($write_access) {
			if (count($hours_mins['hours']) > 1) {
				$hours = Array('--' => '--');
				foreach ($hours_mins['hours'] as $i) $hours[$i] = $i;
				combo_box($prefix.'_hour', $hours, false, $when_arr['hour'], null, $form_extra_elements);
			} else {
				echo $hours_mins['hours'][0];
				hidden_field($prefix.'_hour', $hours_mins['hours'][0]);
			}
		} else {
			echo $when_arr['hour'];
		}

		echo '&nbsp;:&nbsp;';

		if ($write_access) {
			if (count($hours_mins['mins']) > 1) {
				$mins = Array('--' => '--');
				foreach ($hours_mins['mins'] as $i) $mins[$i] = $i;
				combo_box($prefix.'_minute', $mins, false, $when_arr['minute'], null, $form_extra_elements);
			} else {
				echo $hours_mins['mins'][0];
				hidden_field($prefix.'_minute', $hours_mins['mins'][0]);
			}
		} else {
			echo $when_arr['minute'];
		}

		if ($write_access) {
			$next_run = $cron_mgr->timeOfNextRun()
			?>
			<script language="javascript">
				var TIME_UNITS = new Array();
				<?php
				$units = Array(
							   'Y' => 'year',
							   'n' => 'month',
							   'j' => 'day',
							   'G' => 'hour',
							   'i' => 'minute',
							   );

				foreach ($units as $php_unit => $js_unit) {
					echo 'TIME_UNITS[\''.$js_unit.'\'] = \''.str_pad(date($php_unit, $next_run), 2, '0', STR_PAD_LEFT).'\';';
				}
				?>

				function setCurrentTime(prefix) {
					for (u in TIME_UNITS) {
						var id = prefix + '_' + u;
						var unit = document.getElementById(id);

						if (unit.type == 'text') {
							unit.value = TIME_UNITS[u];
						} else {
							for (var i = 0; i < unit.options.length; i++) {
								if (TIME_UNITS[u] == unit.options[i].value) {
									unit.selectedIndex = i;
								}
							}
						}
					}
				}
			</script>
			<?php
			normal_button($prefix.'_set_now', 'next run', "setCurrentTime('$prefix');");
			?>
			&nbsp;or&nbsp;in&nbsp
			<?php
			text_box($prefix.'_duration', ($when_arr['duration'] <> '' ? $when_arr['duration'] : '-'), 2, '', true, $form_extra_elements)
			?>
			&nbsp;
			<?php 
			$periods = Array(
							 'd' => 'days',
							 'w' => 'weeks',
							 'm' => 'months',
							 'y' => 'years',
							 );

			combo_box($prefix.'_period', $periods, false, ($when_arr['period'] <> '' ? $when_arr['period'] : '-'), null, $form_extra_elements);
		}

	}//end _paintWhenBox()


	/**
	* Processes the cron job specific interface for the a date box that we can use
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _processWhenBox(&$asset, &$o, $prefix)
	{
		$prefix .= '_when';

		if (!isset($_POST[$prefix.'_minute']) || !isset($_POST[$prefix.'_duration'])) return false;
		
		$fields = Array();
		foreach (Array('year', 'month', 'day', 'hour', 'minute', 'period', 'duration') as $field) {
			$fields[$field] = $_POST[$prefix.'_'.$field];
		}
		
		$one_off = ($asset->attr('type') == 'one_off');
		$repeating = !$one_off; // for easier reading

		$when_str = '';
		$type_prefix = ($one_off) ? 'OO' : '';

		
		if (isset($_POST[$prefix.'_duration']) && $_POST[$prefix.'_duration'] <> '-') {
			// user chose a duration instead of a fixed date, so lets return that
			$when = $type_prefix.'!'.$fields['period'].$fields['duration'];
			error_log($when);
			
		} else {
			// usr chose a specific date/time, so return that
			foreach (Array('year', 'month', 'day', 'hour', 'minute') as $field) {
				if ($fields[$field] != '--') {
					switch ($field) {
						case 'year' :
							$when_str .= $fields[$field].'-';
						break;
						case 'month' :
							$when_str .= $fields[$field].'-';
							if (!$type_prefix) {
								$type_prefix = 'YR';
							}
						break;
						case 'day' :
							$when_str .= $fields[$field].' ';
							if (!$type_prefix) {
								$type_prefix = (strlen($_POST[$prefix.'_'.$field]) == 2) ? 'MT' : 'WK';
							}
						break;
						case 'hour' :
							$when_str .= $fields[$field].':';
							if (!$type_prefix) {
								$type_prefix = 'DL';
							}
						break;
						case 'minute' :
							$when_str .= $fields[$field];
							if (!$type_prefix) {
								$type_prefix = 'HR';
							}
						break;
					}//end switch
	
				} else if ($type_prefix) {
					trigger_error('You cannot have a '.Cron_Job::whenTypeName($type_prefix).' '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name').' without filling in the '.$field.' field', E_USER_WARNING);
					return false;
				}//end if

			}//end foreach
			
			$when = $type_prefix.'='.$when_str;

		}//end if

		return $when;

	}//end _processWhenBox()


}//end class
?>
