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
* $Id: cron_job_edit_fns.inc,v 1.47 2013/08/14 06:55:37 ewang Exp $
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
* @version $Revision: 1.47 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

		$this->static_screens = Array(
									'details'	=> Array(
													'name'			=> translate('details'),
													'force_unlock'	=> TRUE,
												   ),
									'logs'		=> Array(
													'name'			=> translate('logs'),
													'force_unlock'	=> TRUE,
												   ),
								);

	}//end constructor


	/**
	* Prints out the cron job specific interface for the a date box that we can use
	*
	* @param Cron_Job			$asset					the owning asset
	* @param Backend_Outputter	$o						the backend outputter
	* @param string				$prefix					prefix for form elements
	* @param string				$when					prepared when string
	* @param string				$form_extra_elements	any additional elements for the form entries
	*
	* @return void
	* @access private
	*/
	public function _paintWhenBox(Cron_Job $asset, Backend_Outputter $o, $prefix, $when='', $form_extra_elements='')
	{
		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return FALSE;

		if ((int) $cron_mgr->attr('refresh_time') <= 0) {
			echo translate('cron_manager_refresh_unknown');
			return;
		}

		$write_access = $asset->writeAccess('attributes');
		$prefix .= '_when';
		$one_off = ($asset->attr('type') == 'one_off');
		$repeating = !$one_off; // for easier reading

		$when_str = ($when == '') ? $asset->attr('when') : $when;
		$when_arr = Cron_Job::getWhenArr($when_str);
		$now = getdate();

		// work out the allowed hour/minute based on the cron manager refresh time
		$cron_mgr_edit_fns = $cron_mgr->getEditFns();
		$hours_mins = $cron_mgr_edit_fns->getWhenBoxHoursMins($cron_mgr, $one_off);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);

		// prepare the drop down box for a repeating cron job with time interval (TI)
		if ($repeating) {

			$time_unit = Array(
							60		=> translate('minutes'),
							3600	=> translate('hours'),
							86400	=> translate('days'),
						 );

			$mins = Array('--' => '--');
			$hours = Array('--' => '--');

			// setup the interval in minute
			if (count($hours_mins['mins']) > 1) {
				foreach ($hours_mins['mins'] as $i) {
					$mins[$i] = $i;
				}
				unset($mins['0']);
			} else {
				unset($time_unit[60]);
			}

			// setup the interval in hour
			if (count($hours_mins['hours']) > 1) {
				foreach ($hours_mins['hours'] as $i) {
					$hours[$i] = $i;
				}
				unset($hours['0']);
			} else {
				unset($time_unit[3600]);
			}

			// Set the current values
			$current_mins = $mins['--'];
			$current_hours = $hours['--'];
			$current_days = '-';
			$current_time_unit = current($time_unit);

			if (!empty($when_arr['minute'])) {
				$current_mins = $when_arr['minute'];
				$current_time_unit = isset($time_unit['60']) ? '60' : $current_time_unit;
			} else if (!empty($when_arr['hour'])) {
				$current_hours = $when_arr['hour'];
				$current_time_unit = isset($time_unit['3600']) ? '3600' : $current_time_unit;
			} else {
				if (!empty($when_arr['day']))
				    $current_days = $when_arr['day'];
				$current_time_unit = '86400';
			}

			combo_box($prefix.'_interval_mins', $mins, FALSE, $current_mins, NULL, $form_extra_elements);
			combo_box($prefix.'_interval_hours', $hours, FALSE, $current_hours, NULL, $form_extra_elements);
			text_box($prefix.'_interval_days', $current_days, 2, '', TRUE, $form_extra_elements);
			echo '&nbsp;&nbsp;';

			// time interval unit selection, minute/hour/day (based on cron manager's refresh time)
			// javascript will show/hide selection based on the refresh time setting
			$onChange = 'onChange="switchInterval(this.value)"';
			combo_box($prefix.'_interval_unit', $time_unit, FALSE, $current_time_unit, NULL, $onChange.$form_extra_elements);
			echo '<br />';

			// javascipt to show/hide input based on the current time unit selection
			?>
			<script type="text/javascript">
				var prefix = '<?php echo $prefix; ?>';
				var value = document.getElementById(prefix + '_interval_unit').value;
				switchInterval(value);

				function switchInterval(interval) {
					var prefix = '<?php echo $prefix; ?>';
					if (interval == '60') {
						document.getElementById(prefix + '_interval_mins').style['display'] = '';
						document.getElementById(prefix + '_interval_hours').style['display'] = 'none';
						document.getElementById(prefix + '_interval_days').style['display'] = 'none';
					} else if (interval == '3600') {
						document.getElementById(prefix + '_interval_mins').style['display'] = 'none';
						document.getElementById(prefix + '_interval_hours').style['display'] = '';
						document.getElementById(prefix + '_interval_days').style['display'] = 'none';
					} else if  (interval == '86400') {
						document.getElementById(prefix + '_interval_mins').style['display'] = 'none';
						document.getElementById(prefix + '_interval_hours').style['display'] = 'none';
						document.getElementById(prefix + '_interval_days').style['display'] = '';
					}
				}
			</script>
			<?php

		} else if ($one_off) {

			// day (1 - 31)
			$days = Array('--' => '--');
			for ($i = 1; $i <= 31; $i++) {
				$days[str_pad($i, 2, '0', STR_PAD_LEFT)] = $i;
			}
			if ($write_access) {
				combo_box($prefix.'_day', $days, FALSE, $when_arr['day'], NULL, $form_extra_elements);
			} else {
				echo $days[$when_arr['day']];
			}

			// month (Jan - Dec)
			echo '&nbsp;/&nbsp;';
			if ($write_access) {
				$months = Array('--' => '--');
				for ($i = 1; $i <= 12; $i++) {
					$months[str_pad($i, 2, '0', STR_PAD_LEFT)] = date('M', mktime(0,0,0,$i,1,2000));
				}
				combo_box($prefix.'_month', $months, FALSE, $when_arr['month'], NULL, $form_extra_elements);
			} else {
				echo $when_arr['month'];
			}

			// year (this year - specified years later)
			echo '&nbsp;/&nbsp;';
			if ($write_access) {
				$years = Array('--' => '--');
				$max_years = (int) $cron_mgr->attr('max_years');
				$end_year = max($when_arr['year'], $now['year']) + ($max_years > 0 ? $max_years : 10);
				for ($i = $now['year']; $i <= $end_year; $i++) {
					$years[str_pad($i, 4, '0', STR_PAD_LEFT)] = str_pad($i, 4, '0', STR_PAD_LEFT);
				}
				combo_box($prefix.'_year', $years, FALSE, $when_arr['year'], NULL, $form_extra_elements);
			} else {
				echo $when_arr['year'];
			}

			// hours (based on the values from getWhenBoxHoursMins())
			echo '&nbsp;&nbsp;&nbsp;';
			if ($write_access) {
				if (count($hours_mins['hours']) > 1) {
					$hours = Array('--' => '--');
					foreach ($hours_mins['hours'] as $i) {
						$hours[$i] = $i;
					}
					combo_box($prefix.'_hour', $hours, FALSE, $when_arr['hour'], NULL, $form_extra_elements);
				} else {
					echo $hours_mins['hours'][0];
					hidden_field($prefix.'_hour', $hours_mins['hours'][0]);
				}
			} else {
				echo $when_arr['hour'];
			}

			// minutes (based on the values from getWhenBoxHoursMins())
			echo '&nbsp;:&nbsp;';
			if ($write_access) {
				if (count($hours_mins['mins']) > 1) {
					$mins = Array('--' => '--');
					foreach ($hours_mins['mins'] as $i) {
						$mins[$i] = $i;
					}
					combo_box($prefix.'_minute', $mins, FALSE, $when_arr['minute'], NULL, $form_extra_elements);
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
								'Y'	=> translate('year'),
								'n'	=> translate('month'),
								'j'	=> translate('day'),
								'G'	=> translate('hour'),
								'i'	=> translate('minute'),
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
				if ( $form_extra_elements != 'disabled') {
					normal_button($prefix.'_set_now', translate('next_run'), "setCurrentTime('$prefix');");
				}
				echo ' '.translate('or_in').' ';
				text_box($prefix.'_duration', ($when_arr['duration'] <> '' ? $when_arr['duration'] : '-'), 2, '', TRUE, $form_extra_elements)
				?>
				&nbsp;
				<?php
				$periods = Array(
							'i'	=> translate('minutes'),
							'h'	=> translate('hours'),
							'd'	=> translate('days'),
							'w'	=> translate('weeks'),
							'm'	=> translate('months'),
							'y'	=> translate('years'),
						   );

				combo_box($prefix.'_period', $periods, FALSE, ($when_arr['period'] <> '' ? $when_arr['period'] : '-'), NULL, $form_extra_elements);
			}//end if $write_access

		}//end if one off job

	}//end _paintWhenBox()


	/**
	* Processes the cron job specific interface for the a date box that we can use
	*
	* @param Cron_Job			$asset	the owning asset
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return mixed string|boolean
	* @access private
	*/
	public function _processWhenBox(Cron_Job $asset, Backend_Outputter $o, $prefix)
	{
		$one_off = ($asset->attr('type') == 'one_off');
		$repeating = !$one_off; // for easier reading
		$valid = FALSE;
		$fields = Array();
		$prefix .= '_when';

		// one-off job, make sure form elements have the correct values
		if ($one_off) {
			$has_day = isset($_POST[$prefix.'_day']) && ($_POST[$prefix.'_day'] != '--');
			$has_month = isset($_POST[$prefix.'_month']) && ($_POST[$prefix.'_month'] != '--');
			$has_year = isset($_POST[$prefix.'_year']) && ($_POST[$prefix.'_year'] != '--');
			$has_hour = isset($_POST[$prefix.'_hour']) && ($_POST[$prefix.'_hour'] != '--');
			$has_minute = isset($_POST[$prefix.'_minute']) && ($_POST[$prefix.'_minute'] != '--');
			$has_duration = isset($_POST[$prefix.'_duration']) && ($_POST[$prefix.'_duration'] != '-');
			$valid = (($has_day && $has_month && $has_year && $has_hour && $has_minute) || $has_duration);
			foreach (Array('year', 'month', 'day', 'hour', 'minute', 'period', 'duration') as $field) {
				if (!isset($_POST[$prefix.'_'.$field])) continue;
				$fields[$field] = $_POST[$prefix.'_'.$field];
			}

		// repeating job
		} else if ($repeating) {
			$has_interval = isset($_POST[$prefix.'_interval_unit']);
			$has_mins = isset($_POST[$prefix.'_interval_mins']) && ($_POST[$prefix.'_interval_mins'] != '--') && $_POST[$prefix.'_interval_mins'] > 0;
			$has_hours = isset($_POST[$prefix.'_interval_hours']) && ($_POST[$prefix.'_interval_hours'] != '--') && $_POST[$prefix.'_interval_hours'] > 0;
			$has_days = isset($_POST[$prefix.'_interval_days']) && ($_POST[$prefix.'_interval_days'] != '-') && $_POST[$prefix.'_interval_days'] > 0;
			$valid_min = $has_interval && $has_mins && $_POST[$prefix.'_interval_unit'] == '60';
			$valid_hour = $has_interval && $has_hours && $_POST[$prefix.'_interval_unit'] == '3600';
			$valid_day = $has_interval && $has_days && $_POST[$prefix.'_interval_unit'] == '86400';
			$valid = (($valid_min) || ($valid_hour) || ($valid_day));
		}

		if (!$valid) return FALSE;

		$when_str = '';
		$when = '';
		$type_prefix = ($one_off) ? 'OO' : '';

		if ($one_off) {

			if (isset($_POST[$prefix.'_duration']) && $_POST[$prefix.'_duration'] <> '-') {
				// user chose a duration instead of a fixed date, so lets return that
				$when = $type_prefix.'!'.$fields['period'].$fields['duration'];

				// display notice if user chose a specific date/time as well
				foreach (Array('year', 'month', 'day', 'hour', 'minute') as $field) {
				if ($fields[$field] != '--') {
					trigger_localised_error('CRON0044', E_USER_NOTICE);
					break;
				}
			}

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
								if (!$type_prefix) $type_prefix = 'YR';
							break;
							case 'day' :
								$when_str .= $fields[$field].' ';
								if (!$type_prefix) {
									$type_prefix = (strlen($_POST[$prefix.'_'.$field]) == 2) ? 'MT' : 'WK';
								}
							break;
							case 'hour' :
								$when_str .= $fields[$field].':';
								if (!$type_prefix) $type_prefix = 'DL';
							break;
							case 'minute' :
								$when_str .= $fields[$field];
								if (!$type_prefix) $type_prefix = 'HR';
							break;
						}//end switch

					} else if ($type_prefix) {
						trigger_localised_error('CRON0040', E_USER_WARNING, Cron_Job::whenTypeName($type_prefix), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name'), $field);
						return FALSE;
					}//end if

				}//end foreach

				// do some date parsing like with datetime fields so we get a valid date
				$timestamp = mktime(
								(int) substr($when_str,11,2),
								(int) substr($when_str,14,2),
								(int) substr($when_str,17,2),
								(int) substr($when_str,5,2),
								(int) substr($when_str,8,2),
								(int) substr($when_str,0,4)
							);
				$parsed_date = date('Y-m-d', $timestamp).substr($when_str, 10);

				if (strcmp($parsed_date, $when_str) != 0) {
					trigger_localised_error('SYS0311', E_USER_NOTICE, $when_str);
					$when_str = $parsed_date;
				}

				$when = $type_prefix.'='.$when_str;
			}//end else

		} else if ($repeating) {

			if (isset($_POST[$prefix.'_interval_unit'])) {
				// 60, 3600 or 86400 sec
				$interval_unit = (int) $_POST[$prefix.'_interval_unit'];
				$interval = 0;
				if ($interval_unit == 60) {
					$interval = (int) $_POST[$prefix.'_interval_mins'];
				} else if ($interval_unit == 3600) {
					$interval = (int) $_POST[$prefix.'_interval_hours'];
				} else if ($interval_unit == 86400) {
					$interval = (int) $_POST[$prefix.'_interval_days'];
				}

				$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
				$next_run = $cron_mgr->timeOfNextRun();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);
				// default next run = next run of cron manager
				// new next run = next run + (interval * interval unit)
				$when = "TI=$next_run,$interval,$interval_unit";
			}
		}//end else if

		return $when;

	}//end _processWhenBox()


}//end class
?>
