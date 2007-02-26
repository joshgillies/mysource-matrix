<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: cache_manager_edit_fns.inc,v 1.13 2007/02/26 05:02:55 skim Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';

/**
* Cache_Manager_Edit_Fns
*
* Edit functions for the Cache manager.
*
* @see Asset_Edit_Fns
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.13 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackag __core__
*/
class Cache_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Cache_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['web_paths']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);

	}//end constructor


//--        CACHE EXPIRY        --//


	/**
	* Paints the interface for selecting an asset type to customise
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintChooseTypeCode(&$asset, &$o, $prefix)
	{
		$type_codes = $asset->attr('type_codes');
		$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();

		foreach ($asset_types as $type_code) {
			$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			$options[$type_code] = $info['name'];
		}
		asort($options);

		// we dont use the combo_box function here as we are going
		// to colourise the options that have been customised

		if (isset($_REQUEST[$prefix.'_select_type'])) {
			$selected = $_REQUEST[$prefix.'_select_type'];
		} else {
			$selected = (isset($_REQUEST[$prefix.'_type'])) ? $_REQUEST[$prefix.'_type'] : '';
		}

		?>
		<select name="<?php echo $prefix; ?>_select_type" onchange="this.form.submit();">
			<option value="">-- <?php echo translate('select_asset_type'); ?> --</option>
			<?php
			foreach ($options as $type_code => $name) {
				$style = (isset($type_codes[$type_code])) ? ' style="background-color: #C8BDCB";' : '';
				?>
				<option<?php echo $style; ?> value="<?php echo $type_code; ?>"<?php echo ($selected == $type_code) ? ' selected' : ''; ?>>
					<?php echo $name; ?>
				</option>
				<?php
			}
		?></select><?php

		$o->addFormActionGetVar($prefix.'_type', $selected, TRUE);
		submit_button($prefix.'_go', translate('go'));

		if (isset($_REQUEST[$prefix.'_select_type']) && !empty($_REQUEST[$prefix.'_select_type'])) {
			echo '<br/><br/><b>'.translate('currently_customising', $options[$_REQUEST[$prefix.'_select_type']]).'</b>';
		}

		return FALSE;

	}//end paintChooseTypeCode()


	/**
	* Processes the interface for selecting an asset type to customise
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processChooseTypeCode(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processChooseTypeCode()


	/**
	* Paints the interface for customising an individual asset type's cache options
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCustomiseTypeCodeCaching(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if (isset($_REQUEST[$prefix.'_select_type'])) {
			$selected = $_REQUEST[$prefix.'_select_type'];
		} else {
			$selected = (isset($_REQUEST[$prefix.'_type'])) ? $_REQUEST[$prefix.'_type'] : '';
		}

		if (!empty($selected)) {

			$type_codes = $asset->attr('type_codes');

			// field opened by asset_edit_interface
				$using_default = !isset($type_codes[$selected]['enabled']);
				if ($using_default) {
					echo translate('using_default_value_of',(($asset->attr('enabled')) ? translate('on') : translate('off'))).'<br/>';
				} else {
					$caching = ($type_codes[$selected]['enabled']) ? 1 : 0;
					if ($write_access) {
						combo_box($prefix.'_enabled', Array('1' => translate('on'), '0' => translate('off')), FALSE, $caching);
					} else {
						echo ($type_codes[$selected]['enabled']) ? translate('on') : translate('off');
					}
				}
				if ($write_access) {
					check_box($prefix.'_default_enabled', '1', $using_default);
					echo ' '.translate('use_default');
				}
			$o->closeField();

			$o->openField(translate('default_expiry'), '', translate('valid_cache_length'));
				$using_default = !isset($type_codes[$selected]['expiry']);
				if ($using_default) {
					echo 'Using default value of '.$asset->attr('expiry').' seconds<br/>';
				} else {
					if ($write_access) {
						text_box($prefix.'_expiry', $type_codes[$selected]['expiry'], 5);
					} else {
						echo $type_codes[$selected]['expiry'];
					}
				}
				if ($write_access) {
					check_box($prefix.'_default_expiry', '1', $using_default);
					echo ' '.translate('use_default');
				}
			// field closed by asset_edit_interface

		} else {
			echo translate('customise_asset_selection');
			return FALSE;

		}//end if !empty()

		return $write_access;

	}//end paintCustomiseTypeCodeCaching()


	/**
	* Processes the interface for customising an individual asset type's cache options
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCustomiseTypeCodeCaching(&$asset, &$o, $prefix)
	{
		$type_codes = $asset->attr('type_codes');

		if (isset($_REQUEST[$prefix.'_type'])) {
			$type = $_REQUEST[$prefix.'_type'];
			$defaults = Array('enabled', 'expiry');
			$save = FALSE;

			foreach ($defaults as $default) {
				$using_default = FALSE;
				if (!isset($_POST[$prefix.'_default_'.$default])) {
					if (!isset($type_codes[$type][$default])) {
						$save = TRUE;
						$type_codes[$type][$default] = $asset->attr($default);
					}
				} else {
					$save = TRUE;
					$using_default = TRUE;
					if (isset($type_codes[$type][$default])) {
						unset($type_codes[$type][$default]);
					}
				}
				if (isset($_POST[$prefix.'_'.$default]) && !$using_default) {
					if (!isset($type_codes[$type][$default]) || $type_codes[$type][$default] != $_POST[$prefix.'_'.$default]) {
						$type_codes[$type][$default] = $_POST[$prefix.'_'.$default];
						$save = TRUE;
					}
				}
			}
			if (isset($type_codes[$type]) && empty($type_codes[$type])) {
				unset($type_codes[$type]);
			}
			return ($save) ? $asset->setAttrValue('type_codes', $type_codes) : FALSE;
		}
		return FALSE;

	}//end processCustomiseTypeCodeCaching()


//--        CLEAR CACHE        --//


	/**
	* Paints the interface for clearing the cache files
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintClearCache(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			echo translate('cannot_clear_cache');
			return FALSE;
		}
		asset_finder($prefix.'_clear_cache', '');

		$o->openField(translate('level'));
			$options = Array(
						'single'		=> translate('this_asset'),
						'dependants'	=> translate('this_asset_and_dependants'),
						'children'		=> translate('this_asset_and_children'),
					   );
			combo_box($prefix.'_level', $options, '', FALSE);
		$o->closeField();

		$o->openField(translate('asset_types'));
			$type_options = Array();
			$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
			foreach ($asset_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$type_options[$type_code] = $info['name'];
				}
			}

			asort($type_options);
			$type_options = array_reverse($type_options, TRUE);
			$type_options[''] = '-- '.translate('all_asset_types').' --';
			$type_options = array_reverse($type_options, TRUE);
			combo_box($prefix.'_type_codes', $type_options, TRUE, '', 8);
		$o->closeField();

		return TRUE;

	}//end paintClearCache()


	/**
	* Processes the interface for clearing the cache files, and starts the clear cache hipo
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processClearCache(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_clear_cache']) && !empty($_POST[$prefix.'_clear_cache']['assetid']) && $_POST[$prefix.'_delete_all'] != 'yes') {
			$level = (isset($_POST[$prefix.'_level'])) ? $_POST[$prefix.'_level'] : 'single';

			if (isset($_POST[$prefix.'_type_codes']) && !empty($_POST[$prefix.'_type_codes'][0])) {
				$type_codes = $_POST[$prefix.'_type_codes'];
			} else {
				$type_codes = Array();
			}

			$delete_all = $_POST[$prefix.'_delete_all'];

			$assetid = $_POST[$prefix.'_clear_cache']['assetid'];
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'assetid'		=> $assetid,
						'level'			=> $level,
						'type_codes'	=> $type_codes,
						'delete_all'	=> $delete_all,
					);
			$hh->queueHipo('hipo_job_clear_cache', $vars);

		}

		return TRUE;

	}//end processClearCache()


	/**
	* Paints the interface for clearing the cache files
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintSystemWideClearCache(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			echo translate('cannot_clear_cache');
			return FALSE;
		}

		$o->openField(translate('delete_all_cache'));
			$delete_all_ops = Array(
								'yes'	=> translate('yes'),
								'no'	=> translate('no'),
							  );
			combo_box($prefix.'_delete_all', $delete_all_ops, FALSE, 'no', 0, "onChange=\"Javascript: if (this.value == 'yes') { if(!confirm(js_translate('confirm_purge_cache'))) { this.value = 'no'; } } \"");
			$o->note(translate('trash_cache_warning'));
		$o->closeField();

		return TRUE;

	}//end paintSystemWideClearCache()


	/**
	* Processes the interface for clearing the cache files, and starts the clear cache hipo
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processSystemWideClearCache(&$asset, &$o, $prefix)
	{
		if ($_POST[$prefix.'_delete_all'] == 'yes') {
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'delete_all'	=> 'yes',
					);
			$hh->queueHipo('hipo_job_clear_cache', $vars);
		}

		return TRUE;

	}//end processSystemWideClearCache()


//--        REFRESH CACHE        --//


	/**
	* Paints the interface for refreshing asset cache
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintAddJob(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$caching_on = $asset->attr('enabled');
		if ($wa) {
			if ($this->cronConfigured()) {
				$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');

				// Include Refresh Cache Cron Job
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_refresh_cache');

				// Caching is off. Show the warning only
				if (!$caching_on) {
					?>
					<span class="sq-backend-warning"><b>NOTE:</b> The caching is turned off at the moment. Although you can add a job here, it will not have any effect</span><br /><br />
					<?php
				}

				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				$cron_job =& new Cron_Job_Refresh_Cache();
				$edit_fns = $cron_job->getEditFns();
				$edit_fns->paintInLineBackend($cron_job, $o, $prefix.'_new_rc');
				$o->note(translate('cron_manager_next_run', $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun())));

			} else {
				// Cron Manager is not configured yet.
				echo translate('cron_manager_not_configured_refresh');
			}
		}

		return TRUE;

	}//end paintAddJob()


	/**
	* Processes the interface for refreshing asset cache
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processAddJob(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa && $this->cronConfigured()) {
			$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');

			// Include Refresh Cache Cron Job
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_refresh_cache');

			if ($asset->writeAccess('')) {
				$rc =& new Cron_Job_Refresh_Cache();
				$edit_fns = $rc->getEditFns();
				if ($edit_fns->processInLineBackend($rc, $o, $prefix.'_new_rc')) {
					$rc->saveAttributes();
				} else {
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				if ($cron_mgr->addJob($rc, $GLOBALS['SQ_SYSTEM']->user)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				} else {
					trigger_localised_error('CRON0054', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return FALSE;
				}
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			}

		} else {
			// Cron Manager not configured
			return FALSE;
		}

		return TRUE;

	}//end processAddJob()


	/**
	* Paints the currently scheduled jobs
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentJobs(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_refresh_cache');
		$rc_jobs = $cron_mgr->getJobs('cron_job_refresh_cache');

		if (empty($rc_jobs)) {
			echo translate('cron_manager_refresh_no_job');
		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header">When</th>
					<th class="sq-backend-table-header">Status</th>
					<th class="sq-backend-table-header">Run As</th>
					<th class="sq-backend-table-header">Delete</th>
				</tr>
				<?php
				foreach ($rc_jobs as $rc_job) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
						<?php
							echo get_asset_tag_line($rc_job->attr('asset'));
							echo '<br /><b>Scheduled At:&nbsp;</b>';
							echo $rc_job->readableWhen();
						?>
						</td>
						<td class="sq-backend-table-cell">
						<?php
							if ($rc_job->attr('long') && $rc_job->attr('running')) {
								echo translate('cron_manager_refresh_cache_running');
							} else {
								echo translate('cron_manager_refresh_cache_not_running');
							}
						?>
						</td>
						<td class="sq-backend-table-cell">
						<?php
							$user =& $rc_job->getRunningUser();
							echo get_asset_tag_line($user->id);
							unset($user);
						?>
						</td>
						<td class="sq-backend-table-cell">
						<?php
							if ($wa) {
								check_box($prefix.'_remove_job_'.$rc_job->id);
							}
						?>
						</td>
					</tr>
					<?php
				}//end foreach
				?>
			</table>
			<?php
		}//end else
		return TRUE;

	}//end paintCurrentJobs()


	/**
	* Processes the currently scheduled jobs
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCurrentJobs(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa && $this->cronConfigured()) {
			$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');

			// Include Refresh Cache Cron Job
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_refresh_cache');

			// Any job delete?
			$jobs = $cron_mgr->getJobs('cron_job_refresh_cache');
			for ($i = 0; $i < count($jobs); $i++) {
				if (!empty($_POST[$prefix.'_remove_job_'.($jobs[$i]->id)])) {

					if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
						if (!$cron_mgr->removeJob($jobs[$i])) {
							trigger_localised_error('CRON0055', E_USER_WARNING);
						}
						$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');
					}
				}
			}//end for delete job
		}

		return TRUE;

	}//end processCurrentJobs()


	/**
	* Returns TRUE if the cron is configured in the system
	*
	* @return boolean
	* @access public
	*/
	function cronConfigured()
	{
		$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$cron_epoc = (int)$cron_mgr->attr('epoch');
		$cron_refresh = (int)$cron_mgr->attr('refresh_time');

		return ($cron_epoc > 0) && ($cron_refresh > 0);

	}//end cronConfigured()


//--        DETAILS        --//


	/**
	* Paint the interface for choosing cachable root URLs
	*
	* @param object	&$asset	the asset we are working on
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCachableRootURLs(&$asset, &$o, $prefix)
	{
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$non_cachable_urls = $asset->attr('non_cachable_urls');
		$wa = $asset->writeAccess('attributes');
		?>
		<table class="sq-backend-table" style="width: auto">
			<thead>
				<tr>
					<th><?php echo translate('url'); ?></th>
					<th><?php echo translate('use_cache'); ?></th>
				</tr>
			</thead>
			<tbody>
			<?php
			foreach ($root_urls as $i => $url) {
				?>
				<tr>
					<td><?php echo $url; ?></td>
					<td>
						<?php
						if ($wa) {
							check_box($prefix.'_url_'.$i, 1, !in_array($url, $non_cachable_urls));
						} else {
							$filename = in_array($url, $non_cachable_urls) ? 'cross' : 'tick';
							echo '<img src="'.sq_web_path('lib').'/web/images/'.$filename.'.gif" />';
						}
						?>
					</td>
				</tr>
				<?php
			}
			?>
			</tbody>
		</table>
		<?php

	}//end paintCachableRootURLs()


	/**
	* Process the interface for choosing cachable root URLs
	*
	* @param object	&$asset	the asset we are working on
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCachableRootURLs(&$asset, &$o, $prefix)
	{
		$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
		$cachable = Array();
		foreach ($root_urls as $i => $url) {
			if (!empty($_POST[$prefix.'_url_'.$i])) {
				$cachable[] = $url;
			}
		}
		// Note that we save the *non-cachable* root URLs, so that any new URLs are
		// cachable by default.
		$non_cachable = array_diff($root_urls, $cachable);
		return $asset->setAttrValue('non_cachable_urls', $non_cachable);

	}//end processCachableRootURLs()


}//end class

?>