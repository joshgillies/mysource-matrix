<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Cron_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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

		$this->static_screens =	Array(
									'details' => Array(
										'name'			=> 'View Jobs',
										'force_unlock'	=> true,
									),
								'permissions' => Array('name' => 'Permissions',
													   'force_unlock' => true
													  ),
								);

	}//end constructor


	/**
	* Prints the tasks interface
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
		$o->openField('', 'new_line');
		$jobs = &$asset->getJobs('cron_job', false);
		if (empty($jobs)) {
			echo 'No Jobs registered';
		} else {

			$whens = Array(	
						'OO' => Array(),
						'HR' => Array(),
						'DL' => Array(),
						'WK' => Array(),
						'MT' => Array(),
						'YR' => Array(),
					);

			for($i = 0, $total = count($jobs); $i < $total; $i++) {
				$w = $jobs[$i]->attr('when');
				$whens[substr($w, 0, 2)][$i] = $jobs[$i]->attr('when');
			}


			?>
			<table border="0" cellspacing="1" cellpadding="2" bgcolor="#000000" width="100%">
				<tr>
					<td bgcolor="#DDDDDD" class="sq-backend-field" style="font-weight: bold;">
						&nbsp;
					</td>
					<td bgcolor="#DDDDDD" class="sq-backend-field" style="font-weight: bold;">
						When
					</td>
					<td bgcolor="#DDDDDD" class="sq-backend-field" style="font-weight: bold;">
						Name
					</td>
				</tr>
			<?php

			for(reset($whens); NULL !== ($type = key($whens)); next($whens)) {
				if (empty($whens[$type])) continue;
				asort($whens[$type], SORT_STRING);
				#pre_echo($fs_links);
				$print_type = true;
				foreach($whens[$type] as $i => $when) {
				?>
					<tr>
						<td class="sq-backend-field" style="font-weight: bold;">
							<?php echo ($print_type) ? Cron_Job::whenTypeName($type) : '&nbsp;'; ?>
						</td>
						<td class="sq-backend-data">
							<?php echo $jobs[$i]->readableWhen(false); ?>
						</td>
						<td class="sq-backend-data">
							<?php echo $jobs[$i]->name; ?>
						</td>
					</tr>
				<?php
					$print_type = false;
				}// end foreach
			}// end for

			?>
			</table>
			<?php

		}// end if

		return true;

	}//end paintJobs()


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
	function paintCurrentRunStatus(&$asset, &$o, $prefix)
	{
		$o->openField('Currently Running ?');
		echo ($asset->attr('running')) ? 'Yes' : 'No';

		// if we have a suspicion of being in DeadLock, then allow them to override
		if ($GLOBALS['SQ_SYSTEM']->userRoot() && $asset->attr('running') && (int) $asset->attr('run_check') >= (int) $asset->attr('warn_after_num_run_checks')) {
			$o->closeSection();
			$o->openSection('<div class="sq-backend-warning" style="text-align: center;">POTENTIAL CRON MANAGER DEAD LOCK DETECTED</div>');
			$o->openField('');
			if ($asset->writeAccess()) {
				?>
				<table border="0">
					<tr>
						<td class="sq-backend-data">
							This is only a potential warning brought about because the cron manager has attempted to
							run <?php echo $asset->attr('warn_after_num_run_checks'); ?> times, but has not been able
							to. Two potential reasons for this are :
							<ol class="sq-backend-data">
								<li class="sq-backend-data">An existing cron manager instance is currently running an extremely long running job.</li>
								<li class="sq-backend-data">Some thing has caused the cron manager to exit uncleanly when it was running and the lock wasn't able to be removed.</li>
							</ol>
							Check the cron error log (in [DATA DIR]/<?php echo $asset->error_log_path; ?>) for any information
							<p class="sq-backend-data">
							To remove the lock flag for the cron manager enter the 25 character string you see
							below into the box supplied, then press the Commit button
							</p>
							<p class="sq-backend-data">
							<b>ONLY REMOVE THE LOCK IF YOU KNOW FOR CERTAIN THAT THE CRON MANAGER IS NOT RUNNING</b>
							</p>
						</td>
					</tr>
					<tr>
						<td valign="top" class="sq-backend-data">
							<?php check_box($prefix.'_force_remove_lock'); ?> Force Removal of Lock
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
				echo 'You need to aquire the lock in order resolve this issue';
			}// end if

		}// end if run_check

		return true;
	}//end paintCurrentRunStatus()


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
	function processCurrentRunStatus(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_force_remove_lock'])) return false;

		if ($asset->attr('running') && (int) $asset->attr('run_check') >= (int) $asset->attr('warn_after_num_run_checks')) {
			// if we have a suspicion of being in DeadLock, then allow them to override
			if ($GLOBALS['SQ_SYSTEM']->userRoot() && $asset->writeAccess()) {
				if (validate_security_key()) {
					$asset->setAttrValue('running', false);
					$asset->setAttrValue('run_check', 0);
					return true;

				} else {
					trigger_error('The security key entered was incorrect.', E_USER_NOTICE);
				}// end if
			}// end if
		}// endif

		return false;

	}// end processCurrentRunStatus()

}//end class
?>
