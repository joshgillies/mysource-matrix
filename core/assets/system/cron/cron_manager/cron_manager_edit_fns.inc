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
* $Id: cron_manager_edit_fns.inc,v 1.31 2006/01/25 00:14:55 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Cron_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.31 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Cron_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens = Array(
									'details'		=> Array(
														'name'			=> translate('view_jobs'),
														'force_unlock'	=> true,
													   ),
									'permissions'	=> Array(
														'name'			=> translate('permissions'),
														'force_unlock'	=> true,
													   ),
									'logs'			=> Array(
														'name'			=> translate('logs'),
														'force_unlock'	=> true,
													   ),
								);

	}//end constructor


	/**
	* Prints the jobs interface
	*
	* @param object Cron_Manager		&$cron_mgr	the owning asset
	* @param string						$one_off	whether we are after one_off options or repeating
	*
	* @return Array()
	* @access public
	*/
	function getWhenBoxHoursMins(&$cron_mgr, $one_off)
	{
		$repeating = !$one_off; // for easier reading

		$options = Array(
					'mins'	=> Array(),
					'hours'	=> Array(),
				   );

		$one_hour = 60 * 60;
		$one_day  = 24 * $one_hour;

		$refresh_secs  = (int) $cron_mgr->attr('refresh_time');
		$refresh_mins  = floor($refresh_secs  / 60);
		$refresh_hours = floor($refresh_mins  / 60);

		$epoch_date    = getdate($cron_mgr->attr('epoch'));

		// how often in an hour do we refresh
		if ($refresh_mins == 0){
			$num_refreshes = 60;
		} else if ($refresh_mins >= 60){
			$num_refreshes = 1;
		} else {
			$num_refreshes = floor(60 / $refresh_mins);
		}

		if ($one_off) {
			for ($i = $epoch_date['minutes']; $i < $epoch_date['minutes'] + 60; $i+= (60 / $num_refreshes)) {
				$num = ($i >= 60) ? $i - 60 : $i;
				$options['mins'][] = str_pad($num, 2, '0', STR_PAD_LEFT);
			}
		} else if ($repeating) {
			// handle the minute option differently for a repeating (TI) job
			// showing minute options in the selection box as multiple of refresh time
			for ($i = 0; $i < 60; $i += (60 / $num_refreshes)) {
				$options['mins'][] = $i;
			}
		}
		sort($options['mins']);

		// how often in an day do we refresh
		if ($refresh_hours == 0) {
			$num_refreshes = 24;
		} else if ($refresh_hours >= 24) {
			$num_refreshes = 1;
		} else {
			$num_refreshes = floor(24 / $refresh_hours);
		}

		if ($one_off) {
			for ($i = $epoch_date['hours']; $i < $epoch_date['hours'] + 24; $i+= (24 / $num_refreshes)) {
				$num = ($i >= 24) ? $i - 24 : $i;
				$options['hours'][] = str_pad($num, 2, '0', STR_PAD_LEFT);
			}
		} else if ($repeating) {
			// handle the hour option differently for a repeating (TI) job
			// showing hour options in the selection box as multiple of refresh time
			for ($i = 0; $i < 24; $i += (24 / $num_refreshes)) {
				$options['hours'][] = $i;
			}
		}

		sort($options['hours']);

		return $options;

	}//end getWhenBoxHoursMins()


	/**
	* Prints the jobs interface
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintJobs(&$asset, &$o, $prefix)
	{

		$jobs = $asset->getJobs('cron_job', false);
		if (empty($jobs)) {
			$o->openField('');
				$o->note(translate('cron_no_jobs_registered'));
		} else {

			$whens = Array(
						'OO'	=> Array(),
						'ET'	=> Array(),
						'HR'	=> Array(),
						'DL'	=> Array(),
						'WK'	=> Array(),
						'MT'	=> Array(),
						'YR'	=> Array(),
					 );

			for ($i = 0, $total = count($jobs); $i < $total; $i++) {
				$w = $jobs[$i]->attr('when');
				$whens[substr($w, 0, 2)][$i] = $jobs[$i]->attr('when');
			}

			for (reset($whens); NULL !== ($type = key($whens)); next($whens)) {
				if (empty($whens[$type])) continue;
				asort($whens[$type], SORT_STRING);
				$o->openField(Cron_Job::whenTypeName($type), 'new_line');
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							When
						</td>
						<td class="sq-backend-table-header">
							<?php echo translate('details'); ?>
						</td>
					<?php
						if ($asset->writeAccess('links')) {
						?>
						<td class="sq-backend-table-header" style="text-align: center;">
							<?php echo translate('delete_question'); ?>
						</td>
						<?php
						}//end if
					?>
					</tr>
				<?php
				foreach ($whens[$type] as $i => $when) {
				?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo $jobs[$i]->readableWhen(false); ?>
						</td>
						<td class="sq-backend-table-cell">
							<table border="0" cellspacing="0" cellpadding="0">
								<tr>
									<td class="sq-backend-table-cell" style="font-weight: bold; text-align: right;">
										<?php echo translate('name'); ?> :&nbsp;
									</td>
									<td class="sq-backend-table-cell">
										<?php echo get_asset_tag_line($jobs[$i]->id); ?>
									</td>
								</tr>
								<tr>
									<td class="sq-backend-table-cell" style="font-weight: bold; text-align: right; border-bottom: none">
										<?php echo translate('run_as'); ?> :&nbsp;
									</td>
									<td class="sq-backend-table-cell" style="border-bottom: none">
										<?php
											$user = &$jobs[$i]->getRunningUser();
											if ($user) {
												echo get_asset_tag_line($user->id);
											} else {
												?><em><?php echo translate('system'); ?></em><?php
											}
										?>
									</td>
								</tr>
							</table>
						</td>
					<?php
						if ($asset->writeAccess('links')) {
						?>
						<td class="sq-backend-table-cell" style="text-align: center;">
						<?php
							if ($jobs[$i]->canDelete()) {
								check_box($prefix.'_remove_jobs[]', $jobs[$i]->id);
							} else {
								echo '&nbsp;';
							}//end if
						?>
						</td>
						<?php
						}//end if
					?>
					</tr>
				<?php
				}//end foreach
			}//end for

			?>
			</table>
			<?php

		}//end if

		return true;

	}//end paintJobs()


	/**
	* Processes the jobs interface
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processJobs(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_remove_jobs'])) {
			return false;
		}
		if (!$asset->writeAccess('links')) return false;

		$jobs = $asset->getJobs('cron_job', false);
		if (empty($jobs)) return false;

		for ($i = 0, $total = count($jobs); $i < $total; $i++) {
			if (in_array($jobs[$i]->id, $_POST[$prefix.'_remove_jobs'])) {
				$asset->removeJob($jobs[$i]);
			}
		}

		return true;

	}//end processJobs()


	/**
	* Yes getting the date time field requires it's own function because it is so
	* fucking hard to set-up
	*
	* @param object Asset				&$asset	the owning asset
	* @param string						$prefix	prefix for form elements
	*
	* @return object DateTime_Field
	* @access private
	*/
	function _getEpochDateTimeField(&$asset, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
		$epoch_date = getdate((int) $asset->attr('epoch'));

		$value_arr = Array(
						'y'	=> $epoch_date['year'],
						'm'	=> $epoch_date['mon'],
						'd'	=> $epoch_date['mday'],
						'h'	=> $epoch_date['hours'],
						'i'	=> $epoch_date['minutes'],
						's'	=> $epoch_date['seconds'],
					 );

		$date = getdate();
		$parameters = Array(
						'min'			=> '2003-01-01 00:00:00',
						'max'			=> ($date['year'] + 1).'-12-31 23:59:59',
						'allow_circa'	=> '0',
						'print_format'	=> 'd/m/y h:i:s',
						'show'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
						'style'			=> Array(
											'y'	=> 's',
											'm'	=> 's',
											'd'	=> 's',
											'h'	=> 's',
											'i'	=> 's',
											's'	=> 's',
										   ),
					  );

		$value = '';
		$field = new DateTime_Field($prefix.'_epoch', $value, $parameters);
		$value = $field->unitsArrayToStr($value_arr);
		$field->setValue($value);

		return $field;

	}//end _getEpochDateTimeField()


	/**
	* Prints the interface for editing the cron manager's Epoch
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEpoch(&$asset, &$o, $prefix)
	{
		$field = $this->_getEpochDateTimeField($asset, $prefix);

		// take into account lock, plus only root can edit
		if ($asset->writeAccess('attributes') && $GLOBALS['SQ_SYSTEM']->userRoot()) {
			$field->printField();
		} else {
			echo $field->format();
		}//end if

		$attr = &$asset->getAttribute('epoch');
		echo '<br />&nbsp;';
		$o->note($attr->description);

	}//end paintEpoch()


	/**
	* Processes the interface for editing the cron manager's Epoch
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processEpoch(&$asset, &$o, $prefix)
	{
		$field = $this->_getEpochDateTimeField($asset, $prefix);

		// take into account lock, plus only root can edit
		if ($asset->writeAccess('attributes') && $GLOBALS['SQ_SYSTEM']->userRoot()) {
			if ($field->processField()) {
				$value_arr = $field->strToUnitsArray($field->value);
				$epoch = mktime($value_arr['h'], $value_arr['i'], $value_arr['s'], $value_arr['m'], $value_arr['d'], $value_arr['y']);
				return $asset->setAttrValue('epoch', $epoch);

			}//end if
		}//end if

		return false;

	}//end processEpoch()


	/**
	* Prints the interface for showing when the cron manager last ran
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLastRun(&$asset, &$o, $prefix)
	{
		echo $GLOBALS['SQ_SYSTEM']->datetime((int) $asset->attr('last_run'));
	}//end paintLastRun()


	/**
	* Prints the interface for showing whether the cron manager is currently running or not
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDeadLockStatus(&$asset, &$o, $prefix)
	{
		// if we have a suspicion of being in DeadLock, then allow them to override
		if ($asset->attr('running') && (int) $asset->attr('run_check') >= (int) $asset->attr('warn_after_num_run_checks')) {
			$o->openField('<div class="sq-backend-warning" style="text-align: center;">POTENTIAL CRON MANAGER DEAD LOCK DETECTED</div>', 'new_line');
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
				if ($asset->writeAccess('all')) {
					?>
					<table border="0">
						<tr>
							<td class="sq-backend-data">
								<?php echo translate('cron_deadlock_warning_section_1', $asset->attr('warn_after_num_run_checks')); ?>
								<ol class="sq-backend-data">
									<li class="sq-backend-data"><?php echo translate('cron_deadlock_warning_list_item_1'); ?></li>
									<li class="sq-backend-data"><?php echo translate('cron_deadlock_warning_list_item_2'); ?></li>
								</ol>
								<?php echo translate('cron_deadlock_warning_check_error_log', hide_system_root(SQ_LOG_PATH).'/'.$asset->error_log_file_name.SQ_CONF_LOG_EXTENSION); ?>
								<p class="sq-backend-data"><?php echo translate('cron_deadlock_warning_section_2'); ?></p>
								<p class="sq-backend-data">
								<b><?php echo translate('cron_deadlock_warning_section_3'); ?></b>
								</p>
							</td>
						</tr>
						<tr>
							<td valign="top" class="sq-backend-data">
								<?php
									check_box($prefix.'_force_remove_lock');
									label(translate('cron_force_lock_removal'));
								?>
							</td>
						</tr>
						<tr>
							<td valign="top" class="sq-backend-data">
								<?php security_key(25, 30, 2, true); ?>
							</td>
						</tr>
					</table>
					<?php
				} else {
					echo translate('cron_deadlock_must_acquire_lock');
				}
			} else {
				echo translate('cron_deadlock_warning_contact_root');
			}//end if

		// else everything is OK
		} else {
			$o->openField(translate('cron_manager_appears_to_be_fine'), 'new_line');

		}//end if run_check

		return true;

	}//end paintDeadLockStatus()


	/**
	* Processes the interface for showing whether the cron manager is currently running or not
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDeadLockStatus(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_force_remove_lock'])) {
			return false;
		}

		if ($asset->attr('running') && (int) $asset->attr('run_check') >= (int) $asset->attr('warn_after_num_run_checks')) {
			// if we have a suspicion of being in DeadLock, then allow them to override
			if ($GLOBALS['SQ_SYSTEM']->userRoot() && $asset->writeAccess('all')) {
				if (validate_security_key()) {
					$asset->setAttrValue('running', false);
					$asset->setAttrValue('run_check', 0);
					return true;

				} else {
					trigger_localised_error('CRON0013', E_USER_NOTICE);
				}//end if
			}//end if
		}//endif

		return false;

	}//end processDeadLockStatus()


}//end class
?>
