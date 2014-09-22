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
* $Id: cron_manager_edit_fns.inc,v 1.48 2012/08/30 01:09:10 ewang Exp $
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
* @version $Revision: 1.48 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Manager_Edit_Fns extends Asset_Edit_Fns
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
									'details'		=> Array(
														'name'			=> translate('View Scheduled Jobs'),

														'force_unlock'	=> FALSE,
													   ),
									'permissions'	=> Array(
														'name'			=> translate('Permissions'),

														'force_unlock'	=> TRUE,
													   ),
									'logs'			=> Array(
														'name'			=> translate('Logs'),

														'force_unlock'	=> TRUE,
													   ),
								);

		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['web_paths']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);

	}//end constructor


	/**
	* Prints the jobs interface
	*
	* @param Cron_Manager	$cron_mgr	the owning asset
	* @param string			$one_off	whether we are after one_off options or repeating
	*
	* @return array
	* @access public
	*/
	public function getWhenBoxHoursMins(Cron_Manager $cron_mgr, $one_off)
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
		if ($refresh_mins == 0) {
			$num_refreshes = 60;
		} else if ($refresh_mins >= 60) {
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
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintJobs(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$jobs = $asset->getJobs('cron_job', FALSE);
		if (empty($jobs)) {
			$o->openField('');
				$o->note(translate('No Jobs Registered'));

		} else {
			$num_per_page = 20;
			$total_assets = count($jobs);
			$num_pages = ceil($total_assets / $num_per_page);

			$link_offset = array_get_index($_POST, $prefix.'_link_offset', 0);

			// if the page number is greater to the total of pages
			if ( (round(($link_offset + $num_per_page) / $num_per_page)) > (ceil($total_assets / $num_per_page)) ) {
				$link_offset = (ceil($total_assets / $num_per_page)-1)*$num_per_page;
			}
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
			?>

			<!-- start print pagination bar top -->
			<?php $o->openField('', 'wide_col')?>
			<table class="sq-backend-table" style="border-bottom: none;">
				<tr><th style="text-align: center;">
					<?php
					hidden_field($prefix.'_link_offset', $link_offset);
					if ($link_offset > 0) {
					?>
						<a title="<?php echo translate('Go to the first page') ?>" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '0'); set_hidden_field('process_form', '0'); submit_form(); return false;">

					<?php
					} else {
						?><span title="<?php echo translate('This is the first page; you cannot go any further back') ?>" style="color: #333"><?php

					}
					?>
					&lt;&lt;
					<?php
					if ($link_offset > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?> &nbsp; &nbsp; <?php
					if ($link_offset > 0) {
						?><a title="<?php echo translate('Go to the previous page') ?>" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php

					} else {
						?><span title="<?php echo translate('This is the first page; you cannot go any further back') ?>" style="color: #333"><?php

					}
					?>
					&lt;
					<?php
					if ($link_offset > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					&nbsp; &nbsp; <?php echo sprintf(translate('Page %1$s of %2$s'), round(($link_offset + $num_per_page) / $num_per_page), ceil($total_assets / $num_per_page)); ?> &nbsp; &nbsp;

					<?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?><a title="<?php echo translate('Go to the next page') ?>" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php

					} else {
						?><span title="<?php echo translate('This is the last page; you cannot go any further forward') ?>" style="color: #333"><?php

					}
					?>
					&gt;
					<?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?></a><?php
					} else {
						?></span><?php
					}
					?> &nbsp; &nbsp; <?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?><a title="<?php echo translate('Go to the last page') ?>" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo ($num_pages - 1) * $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php

					} else {
						?><span title="<?php echo translate('This is the last page; you cannot go any further forward') ?>" style="color: #333"><?php

					}
					?>
					&gt;&gt;
					<?php
					if (($link_offset + $num_per_page) < $total_assets) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
				</th><tr>
				</table>
				<?php $o->closeField(); ?>
				<!-- end print pagination bar -->
			<?php
			$counter = 0;
			for (reset($whens); NULL !== ($type = key($whens)); next($whens)) {
				if (empty($whens[$type])) continue;
				asort($whens[$type], SORT_STRING);
				$o->openSection(Cron_Job::whenTypeName($type), 'new_line');
					$o->openField('','wide_col');
					?>
						<table class="sq-backend-table">
							<tr>
								<th style="width:25%;">
									<?php echo translate('When'); ?>

								</th>
								<th style="width:40%;">
									<?php echo translate('Name'); ?>

								</th>
								<th style="width:25%;">
									<?php echo translate('Run As'); ?>

								</th>
							<?php
								if ($asset->writeAccess('links')) {
								?>
								<th style="text-align: center;" style="width:10%;">
									<?php echo translate('Delete?'); ?>

								</th>
								<?php
								}//end if
							?>
							</tr>
						<?php
						foreach ($whens[$type] as $i => $when) {
							if (!(($counter < $link_offset) || (($link_offset+$num_per_page-1) < $counter))) {
						?>
							<tr>
								<td class="sq-backend-table-cell">
									<?php echo $jobs[$i]->readableWhen(FALSE); ?>
								</td>
								<td class="sq-backend-table-cell">
									<?php echo get_asset_tag_line($jobs[$i]->id); ?>
									<?php
									if (($asset->attr('current_job') == $jobs[$i]->id) && ((int) $asset->attr('run_check') >= (int) $asset->attr('warn_after_num_run_checks'))) {
										?>
										<br/><span class="sq-backend-warning">This job appears to have deadlocked the Cron Manager</span>
										<?php
									}
									?>
								</td>
								<td class="sq-backend-table-cell">
									<?php
										$user = $jobs[$i]->getRunningUser();
										if ($user) {
											echo get_asset_tag_line($user->id);
										} else {
											?><em><?php echo translate('System'); ?></em><?php

										}
									?>
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
							}//end if
							$counter++;
						}//end foreach
						?>
						</table>
					<?php
					$o->closeField();
				$o->closeSection();

			}//end for

		}//end else
		return TRUE;

	}//end paintJobs()


	/**
	* Processes the jobs interface
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processJobs(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix.'_remove_jobs'])) {
			return FALSE;
		}
		if (!$asset->writeAccess('links')) return FALSE;

		$jobs = $asset->getJobs('cron_job', FALSE);
		if (empty($jobs)) return FALSE;

		for ($i = 0, $total = count($jobs); $i < $total; $i++) {
			if (in_array($jobs[$i]->id, $_POST[$prefix.'_remove_jobs'])) {
				$asset->removeJob($jobs[$i]);
			}
		}

		return TRUE;

	}//end processJobs()


	/**
	* Yes getting the date time field requires it's own function because it is
	* such a pain to set-up
	*
	* @param Cron_Manager	$asset	the asset to which we belong
	* @param string			$prefix	prefix for form elements
	*
	* @return object
	* @access private
	*/
	protected function _getEpochDateTimeField(Cron_Manager $asset, $prefix)
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
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintEpoch(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$field = $this->_getEpochDateTimeField($asset, $prefix);

		// take into account lock, plus only root can edit
		if ($asset->writeAccess('attributes') && $GLOBALS['SQ_SYSTEM']->userRoot()) {
			echo '<span class="sq-inline-fields-wrapper">';
			$field->printField();
			echo '</span>';
		} else {
			echo $field->format();
		}//end if

		$attr = $asset->getAttribute('epoch');
		$o->note($attr->description);

	}//end paintEpoch()


	/**
	* Processes the interface for editing the cron manager's Epoch
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processEpoch(Cron_Manager $asset, Backend_Outputter $o, $prefix)
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

		return FALSE;

	}//end processEpoch()


	/**
	* Prints the interface for showing when the cron manager last ran
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintLastRun(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		echo $GLOBALS['SQ_SYSTEM']->datetime((int) $asset->attr('last_run'));

	}//end paintLastRun()


	/**
	* Prints the interface for showing whether the cron manager is currently running or not
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintDeadLockStatus(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		// if we have a suspicion of being in DeadLock, then allow them to override
		if ($asset->attr('running') && (int) $asset->attr('run_check') >= (int) $asset->attr('warn_after_num_run_checks')) {
			$o->openField('<div class="sq-backend-warning" style="text-align: center;">POTENTIAL CRON MANAGER DEAD LOCK DETECTED</div>', 'new_line');
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) {
				if ($asset->writeAccess('all')) {
					$current_job_id = $asset->attr('current_job');
					?>
					<table border="0">
						<tr>
							<td class="sq-backend-data">
								<?php echo sprintf(translate('This is only a potential warning brought about because the Scheduled Jobs Manager has attempted to run %s times, but has not been able to. Two potential reasons for this are :'), $asset->attr('warn_after_num_run_checks')); ?>

								<ol class="sq-backend-data">
									<li class="sq-backend-data"><?php echo translate('An existing Scheduled Jobs Manager instance is currently running an extremely long running job.'); ?></li>

									<li class="sq-backend-data"><?php echo translate('Some thing has caused the Scheduled Jobs Manager to exit uncleanly when it was running and the lock wasn\'t able to be removed.'); ?></li>

								</ol>

								<?php
								if (!empty($current_job_id)) {
								?>
								<p class="sq-backend-data">
									<?php echo sprintf(translate('The Scheduled Jobs Manager believes this job is still running: %s'), get_asset_tag_line($current_job_id)); ?>

								</p>
								<?php
								}
								?>
								<?php echo sprintf(translate('Check the cron error log (in "%s") for any information.'), hide_system_root(SQ_LOG_PATH).'/'.$asset->error_log_file_name.SQ_CONF_LOG_EXTENSION); ?>

								<p class="sq-backend-data"><?php echo translate('To confirm the removal of the lock flag for the Scheduled Jobs Manager, enter the character string you see below into the box supplied, then press the Commit button.'); ?></p>

								<p class="sq-backend-data">
								<b><?php echo translate('ONLY REMOVE THE LOCK IF YOU KNOW FOR CERTAIN THAT THE SCHEDULED JOBS MANAGER IS NOT RUNNING.'); ?></b>

								</p>
							</td>
						</tr>
						<tr>
							<td valign="top" class="sq-backend-data">
								<?php
									check_box($prefix.'_force_remove_lock');
									label(translate('Force Removal of Lock'));

								?>
							</td>
						</tr>
						<tr>
							<td valign="top" class="sq-backend-data">
								<?php security_key(4, 4, 2); ?>
							</td>
						</tr>
					</table>
					<?php
				} else {
					echo translate('You must acquire the lock in order resolve this issue');

				}
			} else {
				echo translate('You must contact the root user to resolve this issue');

			}//end if

		// else everything is OK
		} else {
			$o->openField('', 'wide_col');
			echo '<em>'.translate('Scheduled Jobs Manager appears to be fine.').'</em>';


		}//end if run_check

		return TRUE;

	}//end paintDeadLockStatus()


	/**
	* Processes the interface for showing whether the cron manager is currently running or not
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processDeadLockStatus(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix.'_force_remove_lock'])) {
			return FALSE;
		}

		if ($asset->attr('running') && (int) $asset->attr('run_check') >= (int) $asset->attr('warn_after_num_run_checks')) {
			// if we have a suspicion of being in DeadLock, then allow them to override
			if ($GLOBALS['SQ_SYSTEM']->userRoot() && $asset->writeAccess('all')) {
				if (validate_security_key()) {
					$asset->setAttrValue('running', FALSE);
					$asset->setAttrValue('run_check', 0);
					return TRUE;

				} else {
					trigger_localised_error('CRON0013', translate('The security key entered was incorrect.'), E_USER_NOTICE)
				}//end if
			}//end if
		}//endif

		return FALSE;

	}//end processDeadLockStatus()


	/**
	* Paints the 'excluded time' rule
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	public function paintExcludedTime(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$hours_array = Array(
						0	=> '00',
						1	=> '01',
						2	=> '02',
						3	=> '03',
						4	=> '04',
						5	=> '05',
						6	=> '06',
						7	=> '07',
						8	=> '08',
						9	=> '09',
						10	=> '10',
						11	=> '11',
						12	=> '12',
						13	=> '13',
						14	=> '14',
						15	=> '15',
						16	=> '16',
						17	=> '17',
						18	=> '18',
						19	=> '19',
						20	=> '20',
						21	=> '21',
						22	=> '22',
						23	=> '23',
					   );
		$days_array = Array(
						0	=> translate('Sunday'),

						1	=> translate('Monday'),

						2	=> translate('Tuesday'),

						3	=> translate('Wednesday'),

						4	=> translate('Thursday'),

						5	=> translate('Friday'),

						6	=> translate('Saturday'),

					  );

		$write_access = $asset->writeAccess('all') && $GLOBALS['SQ_SYSTEM']->userRoot();

		$exclude_times = $asset->attr('exclude_times');

		if ($write_access || !empty($exclude_times)) {
		?>
		<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header"><?php echo translate('Blackout Periods') ?></th>

					<?php
						if ($write_access) {
							?><th class="sq-backend-table-header" style="text-align:center;"><?php echo translate('Delete') ?>?</th><?php

						}
					?>
				</tr><?php

		}

		if ($write_access) {

			foreach ($exclude_times as $key => $details) {
				?>
				<tr>
				<?php
				$field_prefix = $prefix.'_exlude_times['.$key.']';
				ob_start();
					combo_box($field_prefix.'[from]', $hours_array, FALSE, $details['from']);
					$from = ob_get_contents();
				ob_end_clean();
				ob_start();
					combo_box($field_prefix.'[to]', $hours_array, FALSE, $details['to']);
					$to = ob_get_contents();
				ob_end_clean();
				ob_start();
					combo_box($field_prefix.'[days]', $days_array, TRUE, $details['days']);
					$days = ob_get_contents();
				ob_end_clean();
				?><td class="sq-backend-table-cell sq-top-align-children">
					<span class="sq-inline-fields-wrapper">
					<?php
					echo sprintf(translate('Do not process jobs between %1$s:00 and %2$s:00'), $from, $to);

					echo sprintf(translate('on %s'), $days);

					?>
					</span>
				</td><td class="sq-backend-table-cell" style="text-align:center;"><?php
					check_box($field_prefix.'[delete]', TRUE, FALSE);?></td></tr><?php
			}

			?>
			<tr><td colspan="2">
				<?php
					check_box($prefix.'_add_exclude_time', TRUE, FALSE);
					label('Add new blackout period', $prefix.'_add_exclude_time');
				?></tr>
			<?php

		} else {
			if (!empty($exclude_times)) {

				foreach ($exclude_times as $key => $details) {
					?>
					<tr>
					<?php
					$from = $hours_array[$details['from']];
					$to = $hours_array[$details['to']];
					if (!empty($details['days'])) {
						$selected_days = Array();
						foreach ($details['days'] as $day) {
							$selected_days[] = $days_array[$day];
						}
						$days = implode(', ', $selected_days);
					}
					?><td class="sq-backend-table-cell" colspan="2"><?php
						echo sprintf(translate('Do not process jobs between %1$s:00 and %2$s:00'), $from, $to);

						if (!empty($days)) {
							if (count($selected_days) == 7) {
								echo translate('on every day');

							} else {
								echo sprintf(translate('on %s'), $days);

							}
						}
					?></td></tr><?php
				}
			} else {
				echo translate('There are no blackout periods');

			}
		}

		if ($write_access || !empty($exclude_times)) {
			?></table><?php
		}

		return $write_access;

	}//end paintExcludedTime()


	/**
	* Processes the 'excluded time' rule
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return void
	* @access private
	*/
	public function processExcludedTime(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$modified = FALSE;

		$exlude_times = Array();
		if (isset($_POST[$prefix.'_exlude_times'])) {
			$modified = TRUE;
			foreach ($_POST[$prefix.'_exlude_times'] as $key => $exclude_period) {
				if (isset($_POST[$prefix.'_exlude_times'][$key]['delete'])) {
					continue;
				}

				if (!isset($exclude_period['days'])) {
					$exclude_period['days'] = Array();
				}

				$exlude_times[] = $exclude_period;
			}
		}

		if (isset($_POST[$prefix.'_add_exclude_time'])) {
			$modified = TRUE;
			$new_exclude_time = Array();
			$new_exclude_time['from'] = 0;
			$new_exclude_time['to'] = 0;
			$new_exclude_time['days'] = Array();
			$exlude_times[] = $new_exclude_time;

		}

		if ($modified) {
			return $asset->setAttrValue('exclude_times', $exlude_times);
		}

		return FALSE;

	}//end processExcludedTime()


	/**
	* Paints the 'log rotate time' field
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	public function paintLogRotateTime(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$hours_array = Array(
						0	=> '00',
						1	=> '01',
						2	=> '02',
						3	=> '03',
						4	=> '04',
						5	=> '05',
						6	=> '06',
						7	=> '07',
						8	=> '08',
						9	=> '09',
						10	=> '10',
						11	=> '11',
						12	=> '12',
						13	=> '13',
						14	=> '14',
						15	=> '15',
						16	=> '16',
						17	=> '17',
						18	=> '18',
						19	=> '19',
						20	=> '20',
						21	=> '21',
						22	=> '22',
						23	=> '23',
					   );

		$write_access = $asset->writeAccess('all') && $GLOBALS['SQ_SYSTEM']->userRoot();

		$lm = $GLOBALS['SQ_SYSTEM']->getLogManager();
		$log_rotate_time = $lm->attr('log_rotate_time');

		echo '<span class="sq-inline-fields-wrapper">';
			if ($write_access) {
				combo_box($prefix.'_log_rotate_time', $hours_array, FALSE, $log_rotate_time);
			} else {
				echo $hours_array[$log_rotate_time];
			}//end if

			echo translate(':00');

		echo '</span>';

		return $write_access;

	}//end paintLogRotateTime()


	/**
	* Processes the 'log rotate time' field
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return void
	* @access private
	*/
	public function processLogRotateTime(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$lm = $GLOBALS['SQ_SYSTEM']->getLogManager();
		if (isset($_POST[$prefix.'_log_rotate_time'])) {
			$log_rotate_time = (int) $_POST[$prefix.'_log_rotate_time'];

			$GLOBALS['SQ_SYSTEM']->am->acquireLock($lm->id, 'attributes');
			$lm->setAttrValue('log_rotate_time', $log_rotate_time);
			$lm->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($lm->id, 'attributes');

			return TRUE;
		}

		return FALSE;

	}//end processLogRotateTime()


	/**
	* Paints the interface for user to select the users to send deadlock mail to
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	public function paintEmailSelection(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{

		// get all the user already defined to send the dead lock emails to
		$users = $asset->attr('dead_lock_email_to');

		/// and we have to be root
		$write_access = $asset->writeAccess('all') && $GLOBALS['SQ_SYSTEM']->userRoot();

		if ($write_access) {
			multiple_asset_finder($prefix.'_dead_lock_emails_to', $users, Array('user' => 'D', 'user_group' => 'D'));
		} else {
			?><ul><?php

				foreach ($users as $assetid) {
					if ($assetid == '0') continue;
					echo '<li>'.get_asset_tag_line($assetid).'</li>';
				}
			?></ul><?php
		}

		return TRUE;

	}// end paintEmailSelection()


	/**
	* Processes the interface for user to select the users to send deadlock mail to
	*
	* @param Cron_Manager		$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	public function processEmailSelection(Cron_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_dead_lock_emails_to'])) return FALSE;

		$new_users = $_POST[$prefix.'_dead_lock_emails_to'];
		$users_to_mail = Array();
		foreach ($new_users as $index => $new_user) {
			// skip if the assetid is not set or if it is root user
			if ($new_user['assetid'] == '0' || $new_user['assetid'] == '12') continue;
			$users_to_mail[] = $new_user['assetid'];
		}

		// process new stuff here
		$new_value = serialize($users_to_mail);
		$asset->setAttrValue('dead_lock_email_to', $new_value);
		return $asset->saveAttributes();

	}// end processEmailSelection()


}//end class
?>
