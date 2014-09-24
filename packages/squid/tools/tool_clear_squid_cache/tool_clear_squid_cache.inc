<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: tool_clear_squid_cache.inc,v 1.11 2013/08/09 06:11:59 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Tool_Clear_Squid_Cache
*
* Purpose
*
* Tool to clear Squid cache using asset's URLs
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/

class Tool_Clear_Squid_Cache extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid  the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param Backend_Outputter	$o			reference to the backend outputter of the object painting the tool
	* @param string				$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	*/
	public static function paintTool(Backend_Outputter $o, $type_code)
	{
		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);

		require_once SQ_DATA_PATH.'/private/conf/tools.inc';
		$preferencesBroken = array();
		$squid_hostnames = trim(SQ_TOOL_SQUID_CACHE_HOSTNAMES);
		if (empty($squid_hostnames) === TRUE) {
			$preferencesBroken[] = 'squid_tool_no_hostnames';
		}

		$squidclient_path = trim(SQ_TOOL_SQUID_CACHE_PATH);
		if (empty($squidclient_path) === TRUE) {
			$preferencesBroken[] = 'squid_tool_path_empty';
		} else {
			if (is_file($squidclient_path) === FALSE) {
				$preferencesBroken[] = 'squid_tool_path_invalid';
			}
		}

		if (empty($preferencesBroken) === FALSE) {
			$o->openField(translate('Squid Cache is not configured'));

			foreach ($preferencesBroken as $brokenItem) {
				$o->note(translate($brokenItem));
			}
			$o->closeField();
			$o->closeSection();
			return;
		}

		// interface to get to asset using asset finder
		$o->openField(translate('Asset'));

			asset_finder($type_code.'_assetid');
			$o->note(translate('Squid cache will be cleared for all URLs applied to this asset.'));

		$o->closeField();

		// if clear now box is checked, cache is cleared at commit
		$o->openField(translate('Clear Now'));

			check_box($type_code.'_clear_now', TRUE);
		$o->closeField();

		// if populate now box is checked, will request page using CURL
		$o->openField(translate('Repopulate?'));

			check_box($type_code.'_populate_now', TRUE);
			echo translate('Request each URL after the purge action so that Squid can recache it.');

		$o->closeField();

		// cascade options
		$o->openField(translate('Cascade?'));

			$options = Array(
						'single'	=> translate('This asset'),

						'children'	=> translate('This asset and its children'),

					   );
			check_box($type_code.'_cascade', TRUE);
			// combo_box($type_code.'_cascade', $options, '', FALSE);
			echo translate('Clear the cache of the selected asset and its children.');

			echo '<br /><br />';
			$o->note(translate('Note: Repopulate and Cascade option will also be applied to the Scheduled Job if you schedule one.'));

		$o->closeField();

		$o->closeSection();

		// scheduled jobs section
		$o->openSection(translate('Schedule Clear Cache'));


		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_squid_cache_purge');
		$purge_jobs = Cron_Job_Squid_Cache_Purge::getActiveJobs();

		// print scheduled jobs, one_off or repeating
		if (!empty($purge_jobs)) {
			$o->openField(translate('Scheduled Jobs'), 'new_line');

			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('Asset'); ?></td>

					<td class="sq-backend-table-header"><?php echo translate('When'); ?></td>

					<td align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('Delete?'); ?></td>

				</tr>
				<?php
					for ($i = 0; $i < count($purge_jobs); $i++) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo get_asset_tag_line($purge_jobs[$i]->attr('ownerid')); ?>

						</td>
						<td class="sq-backend-table-cell">
							<?php echo $purge_jobs[$i]->readableWhen(); ?>
						</td>
						<td align="center" class="sq-backend-table-cell">
						<?php
							// only print checkbox if user has write access to asset
							if ($purge_jobs[$i]->canDelete()) {
								if ($purge_jobs[$i]->writeAccess('')) {
									check_box($type_code.'_remove_fs[]', $purge_jobs[$i]->id);
								}
							} else {
								echo '&nbsp;';
							}
						?>
						</td>
					</tr>
					<?php
				}//end for
				?>
			</table><br />
			<?php
			$o->closeField();
		}//end if

		$cron_epoc = (int)$cron_mgr->attr('epoch');
		$cron_refresh = (int)$cron_mgr->attr('refresh_time');
		if ($cron_epoc > 0 && $cron_refresh > 0) {

			// prints the interface for adding a new one-off job
			$o->openField(translate('Schedule a One Off Job'), 'new_line');

				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$new_job = new Cron_Job_Squid_Cache_Purge();
				$edit_fns = $new_job->getEditFns();
				$edit_fns->paintInLineBackend($new_job, $o, $type_code.'_one_off');
				echo '&nbsp;<br /><br />';
			$o->closeField();

			// prints the interface for adding a new repeating job
			$o->openField(translate('Schedule a Repeating Job'), 'new_line');

				$new_job = new Cron_Job_Squid_Cache_Purge();
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					$new_job->setAttrValue('type', 'repeating');
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$edit_fns = $new_job->getEditFns();
				$edit_fns->paintInLineBackend($new_job, $o, $type_code.'_repeating');
				$o->note(sprintf(translate('The Scheduled Jobs Manager runs every <b>%1$s</b>. It will next run on the <b>%2$s</b>.'), $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun())));

			$o->closeField();

		} else {
			$o->openField('Note');
				$o->note(translate('The Scheduled Jobs Manager has not been configured. No future statuses can be set for this asset.'));

			$o->closeField();
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);

		$o->closeSection();

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param Backend_Outputter	$o			reference to the backend outputter of the object painting the tool
	* @param string				$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	*/
	public static function processTool(Backend_Outputter $o, $type_code)
	{
		// If this isn't  set in the post array at all, then there was an error
		// being shown (eg empty hostnames or invalid squidclient path).
		// So don't process anything.
		if (isset($_POST[$type_code.'_assetid']) === FALSE) {
			return FALSE;
		}

		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_squid_cache_purge');

		// remove jobs
		if (!empty($_POST[$type_code.'_remove_fs'])) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_squid_cache_purge');
			$purge_jobs = Cron_Job_Squid_Cache_Purge::getActiveJobs();
			if (!empty($purge_jobs)) {
				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
					for ($i = 0; $i < count($purge_jobs); $i++) {
						if (in_array($purge_jobs[$i]->id, $_POST[$type_code.'_remove_fs'])) {
							$cron_mgr->removeJob($purge_jobs[$i]);
						}
					}

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');
				} else {
					trigger_localised_error('SYS0232', translate('Unable to acquire the lock on the Cron Manager, unable to remove jobs'), E_USER_NOTICE);
				}
			}
			// remove job only, do not print error msg
			if (!$_REQUEST[$type_code.'_assetid']['assetid']) {
				return FALSE;
			}
		}

		// error: asset is not defined
		if (!$_REQUEST[$type_code.'_assetid']['assetid']) {
			trigger_localised_error('SQUID0001', translate('The Asset to be purged is not defined.'), E_USER_NOTICE);
			return FALSE;
		}

		// set HIPO running_vars
		$assetid = $_REQUEST[$type_code.'_assetid']['assetid'];
		$cascade = (isset($_REQUEST[$type_code.'_cascade'])) ? $_REQUEST[$type_code.'_cascade'] : FALSE;
		$populate_now = FALSE;
		if (isset($_REQUEST[$type_code.'_populate_now']) && $_REQUEST[$type_code.'_populate_now']) {
			$populate_now = TRUE;
		}
		$vars = Array(
					'assetid'		=> $assetid,
					'cascade'		=> $cascade,
					'repopulate'	=> $populate_now,
				);

		// schedule job
		$owner = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

		$urls = $GLOBALS['SQ_SYSTEM']->am->getURLs($assetid);
		if (empty($urls)) {
			return FALSE;
		}

		if ($owner->adminAccess('')) {

		    	// run HIPO job if the 'clear now' box is checked
			$run_now = isset($_REQUEST[$type_code.'_clear_now']) && $_REQUEST[$type_code.'_clear_now'];
			if ($run_now) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hh->queueHipo('hipo_job_tool_clear_squid_cache', $vars, '', SQ_PACKAGES_PATH.'/squid/hipo_jobs');
				$url = $hh->runQueuedJobs();
				if (!empty($url)) $o->setRedirect($url);
				return TRUE;
			}

			// one off
			$new_job = new Cron_Job_Squid_Cache_Purge();
			$edit_fns = $new_job->getEditFns();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$new_job->setAttrValue('ownerid', $assetid);
				$new_job->setAttrValue('cascade', $cascade);
				$new_job->setAttrValue('repopulate', $populate_now);
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			if ($edit_fns->processInLineBackend($new_job, $o, $type_code.'_one_off')) {
				$cron_mgr->addJob($new_job, $GLOBALS['SQ_SYSTEM']->user);
			}

			// repeating
			$new_job = new Cron_Job_Squid_Cache_Purge();
			$edit_fns = $new_job->getEditFns();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$new_job->setAttrValue('type', 'repeating');
				$new_job->setAttrValue('ownerid',$assetid);
				$new_job->setAttrValue('cascade', $cascade);
				$new_job->setAttrValue('repopulate', $populate_now);
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			if ($edit_fns->processInLineBackend($new_job, $o, $type_code.'_repeating')) {
				$cron_mgr->addJob($new_job, $GLOBALS['SQ_SYSTEM']->user);
			}



		} else {
			trigger_localised_error('SQUID0003', sprintf(translate('Unable to process tool, no admin access found, permission denied.'), sprintf(translate('Purge Squid Cache Scheduled Job #%s'), $owner->id)), E_USER_NOTICE);

		}//end if has write access

		return FALSE; // we want to repaint the tool

	}//end processTool()


}//end class


?>
