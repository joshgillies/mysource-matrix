<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Cron_Job_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
										'name'			=> 'Details',
										'force_unlock'	=> true,
									),
								);
	}//end constructor


	/**
	* Prints out the cron job specific interface for the a date box that we can use
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @access private
	*/
	function _paintWhenBox(&$asset, &$o, $prefix)
	{
		$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) return false;

		if ((int) $cron_mgr->attr('refresh_time') <= 0) {
			echo 'Cron Refresh Time is unknown';
			return;
		}

		$prefix .= '_when';

		$one_off = ($asset->attr('type') == 'one_off');
		$repeating = !$one_off; // for easier reading


		$cron_mgr_edit_fns = $cron_mgr->getEditFns();
		$hours_mins = $cron_mgr_edit_fns->getWhenBoxHoursMins($cron_mgr, $one_off);

		$when_str = $asset->attr('when');
		$when_arr = Cron_Job::getWhenArr($when_str);

		$now = getdate();

		$days = Array('--' => '--');
		if ($repeating) {
			for($i = 0; $i <= 6; $i++) $days[(string) $i] = Cron_Job::whenWeekDayName($i);
		}
		for ($i = 1; $i <= 31; $i++) $days[str_pad($i, 2, '0', STR_PAD_LEFT)] = str_pad($i, 2, '0', STR_PAD_LEFT);
		if ($asset->writeAccess()) {
			combo_box($prefix.'_day', $days, false, $when_arr['day']);
		} else {
			echo $days[$when_arr['day']];
		}

		echo '&nbsp;/&nbsp;';

		if ($asset->writeAccess()) {
			$months = Array('--' => '--');
			for ($i = 1; $i <= 12; $i++) $months[str_pad($i, 2, '0', STR_PAD_LEFT)] = date('M', mktime(0,0,0,$i,1,2000));
			combo_box($prefix.'_month', $months, false, $when_arr['month']);
		} else {
			echo $when_arr['month'];
		}

		if ($one_off) {
			echo '&nbsp;/&nbsp;';
			if ($asset->writeAccess()) {
				$years = Array('--' => '--');
				$end_year = max($when_arr['year'], $now['year']) + 10;
				for ($i = $now['year']; $i <= $end_year; $i++) $years[str_pad($i, 4, '0', STR_PAD_LEFT)] = str_pad($i, 4, '0', STR_PAD_LEFT);
				combo_box($prefix.'_year', $years, false, $when_arr['year']);
			} else {
				echo $when_arr['year'];
			}
		}

		echo '&nbsp;&nbsp;&nbsp;';

		if ($asset->writeAccess()) {
			if (count($hours_mins['hours']) > 1) {
				$hours = Array('--' => '--');
				foreach($hours_mins['hours'] as $i) $hours[$i] = $i;
				combo_box($prefix.'_hour', $hours, false, $when_arr['hour']);
			} else {
				echo $hours_mins['hours'][0];
				hidden_field($prefix.'_hour', $hours_mins['hours'][0]);
			}
		} else {
			echo $when_arr['hour'];
		}

		echo '&nbsp;:&nbsp;';

		if ($asset->writeAccess()) {
			if (count($hours_mins['mins']) > 1) {
				$mins = Array('--' => '--');
				foreach($hours_mins['mins'] as $i) $mins[$i] = $i;
				combo_box($prefix.'_minute', $mins, false, $when_arr['minute']);
			} else {
				echo $hours_mins['mins'][0];
				hidden_field($prefix.'_minute', $hours_mins['mins'][0]);
			}
		} else {
			echo $when_arr['minute'];
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

		if (!isset($_POST[$prefix.'_minute'])) return false;

		$one_off = ($asset->attr('type') == 'one_off');
		$repeating = !$one_off; // for easier reading

		$when_str = '';
		$type_prefix = ($one_off) ? 'OO' : '';

		foreach(Array('year', 'month', 'day', 'hour', 'minute') as $field) {
			if ($_POST[$prefix.'_'.$field] != '--') {
				switch($field) {
					case 'year' :
						$when_str .= $_POST[$prefix.'_'.$field].'-';
						break;
					case 'month' :
						$when_str .= $_POST[$prefix.'_'.$field].'-';
						if (!$type_prefix) {
							$type_prefix = 'YR';
						}
						break;
					case 'day' :
						$when_str .= $_POST[$prefix.'_'.$field].' ';
						if (!$type_prefix) {
							$type_prefix = (strlen($_POST[$prefix.'_'.$field]) == 2) ? 'MT' : 'WK';
						}
						break;
					case 'hour' :
						$when_str .= $_POST[$prefix.'_'.$field].':';
						if (!$type_prefix) {
							$type_prefix = 'DL';
						}
						break;
					case 'minute' :
						$when_str .= $_POST[$prefix.'_'.$field];
						if (!$type_prefix) {
							$type_prefix = 'HR';
						}
						break;
				}// end switch

			} else if ($type_prefix) {
				trigger_error('You cannot have a '.Cron_Job::whenTypeName($type_prefix).' '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name').' without filling in the '.$field.' field', E_USER_WARNING);
				return false;
			}// end if

		}// end foreach

		$when = $type_prefix.'='.$when_str;
#		pre_echo('When : '.$when);

		return $asset->setAttrValue('when', $when);

	}//end _processWhenBox()


}//end class
?>
