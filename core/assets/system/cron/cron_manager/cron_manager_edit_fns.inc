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

		pre_echo($asset->getLinks(SQ_LINK_TYPE_3, 'cron_job', false));

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
								<li class="sq-backend-data">Some thing has caused the cron manager to exit uncleanly and the lock wasn't able to be removed.</li>
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
