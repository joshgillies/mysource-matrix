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
* $Id: workflow_bundle_edit_fns.inc,v 1.9 2012/08/30 01:09:17 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Workflow_Bundle_Edit_Fns
*
* Purpose
*    Edit functions file for the Workflow Bundle asset.
*    Primarily used to provide a custom Workflow screen, allowing the one-click
*    Approve/Reject of assets in the bundle. The normal (static) Workflow screen
*    will be disabled entirely, since workflow will not be used on bundles
*    themselves.
*
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Bundle_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Handles painting of the Current Status field
	*
	* Here since it needs custom processing
	*
	* @param Workflow_Bundle	$asset	The asset being processed
	* @param Backend_Outputter	$o		The outputter class being used
	* @param string				$prefix	Prefix for form elements
	*
	* @return boolean
	*/
	function paintCurrentStatus(Workflow_Bundle $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/asset_static_screens/details.inc';
		_paintDetailsCurrentStatus($asset, $o, $prefix);

		return FALSE;

	}//end paintCurrentStatus()


	/**
	* Handles painting of the Status Change field
	*
	* Here since it needs custom processing
	*
	* @param Workflow_Bundle	$asset	The asset being processed
	* @param Backend_Outputter	$o		The outputter class being used
	* @param string				$prefix	Prefix for form elements
	*
	* @return boolean
	*/
	function paintCustomStatusChange(Workflow_Bundle $asset, Backend_Outputter $o, $prefix)
	{
		// Don't need write access to process status changes, since the changes
		// allowed are handled by the asset status objects. So process it here
		// where Matrix will (generally) not have a temper tantrum.
		if ($this->processCustomStatusChange($asset, $o, $prefix) === TRUE) {
			// Was a HIPO (for bundle status change) queued? If so, then point
			// to the URL so it runs
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$url = $hh->runQueuedJobs();
			if (empty($url) === FALSE) $o->setRedirect($url);
			return FALSE;
		}

		// Otherwise, paint the status change - or the message saying you can't
		// do it here; do it on the Workflow screen instead
		if ((int)$asset->status === SQ_STATUS_PENDING_APPROVAL) {
			$need_commit = FALSE;
			$o->note(sprintf(translate('<i>This asset is currently in workflow. Approval or rejection of the changes must be done on the <a href="%s">asset workflow screen</a></i>.'), $asset->getBackendHref('workflow')));

		} else {
			require_once SQ_INCLUDE_PATH.'/asset_static_screens/details.inc';
			ob_start();
				$need_commit = _paintDetailsChangeStatus($asset, $o, $prefix);
			$status_output = ob_get_clean();

			$filter_output = '';
			$found = preg_match('/\<select.*"'.$prefix.'_change_status"[^>]*\>.*\<\/select\>/U', $status_output, $select_matches);
			if ($found === 1) {
				$filter_output .= $select_matches[0].' ';
				$found = preg_match('/\<input.*"'.$prefix.'_change_status_all_children"[^>]*\>\<label [^>]*\>.*\<\/label\>/', $status_output, $cb_matches);
				if ($found === 1) {
					$filter_output .= $cb_matches[0].' ';
				}

				$status_output = $filter_output;
			}

			// Do we need to add the streams?
			// (Do so if we're Editing, we can go to Approving, and there's at
			// least one alternate stream available.)
			$available_statuses = $asset->getAvailableStatii();
			if (((int)$asset->status === SQ_STATUS_UNDER_CONSTRUCTION) && ((array_key_exists(SQ_STATUS_PENDING_APPROVAL, $available_statuses) === TRUE) || (array_key_exists(SQ_STATUS_APPROVED, $available_statuses) === TRUE))) {
				$streams['workflow'] = $asset->getAllBundleStreams();

				if (count($streams) > 0) {
					$bypass_streams = $asset->getAllBypassableBundleStreams();
					foreach ($bypass_streams as $bypass_streamid) {
						$streams['bypass'][$bypass_streamid] = $streams['workflow'][$bypass_streamid];
						unset($streams['workflow'][$bypass_streamid]);
					}

					// Get what the status change check-box looks like
					// so we can use it to inject the stream drop-down
					ob_start();
						check_box($prefix.'_change_status_all_children');
					$cascade_box = ob_get_clean();

					// Buffer the stream drop-down
					ob_start();
						$options = Array(
									'bypass'	=> Array(),
									'workflow'	=> Array(),
								   );

						if ($asset->isDefaultStreamBypassable() === TRUE) {
							$options['bypass'] = Array('SQ_USE_DEFAULT' => 'Default Stream');
						} else {
							$options['workflow'] = Array('SQ_USE_DEFAULT' => 'Default Stream');
						}

						$total_options = 1;
						foreach (array_keys($streams) as $stream_type) {
							$streams[$stream_type] = array_unique($streams[$stream_type]);
							foreach ($streams[$stream_type] as $workflow_stream_name) {
								$options[$stream_type][$workflow_stream_name] = $workflow_stream_name;
								$total_options++;
							}
						}

						if ($total_options > 1) {
							ob_start();
								combo_box($prefix.'_change_status_stream', $options['workflow'], FALSE, Array(), 0, ' disabled="disabled" id="'.$prefix.'_change_status_stream_workflow"');
								combo_box($prefix.'_change_status_stream', $options['bypass'],   FALSE, Array(), 0, ' disabled="disabled" id="'.$prefix.'_change_status_stream_bypass"');
							$cb = ob_get_clean();
							echo sprintf(translate('using workflow stream: %1$s'), $cb);

						} else {
							// Only one option; hide the stream field and chuck it into a hidden field
							$all_options = $options['bypass'] + $options['workflow'];
							reset($all_options);
							hidden_field($prefix.'_change_status_stream', key($all_options));
						}
						?>
						<script type="text/javascript">
						//<![CDATA[
							var status_cb = document.getElementById("<?php echo $prefix ?>_change_status");
							var stream_wf_cb = document.getElementById("<?php echo $prefix ?>_change_status_stream_workflow");
							var stream_byp_cb = document.getElementById("<?php echo $prefix ?>_change_status_stream_bypass");

							if (stream_wf_cb.options.length == 0) {
								stream_wf_cb.style.display = 'none';
							} else {
								stream_byp_cb.style.display = 'none';
							}

							status_cb.onchange = function() {
								if ((this.value & <?php echo SQ_SC_STATUS_PENDING ?>) > 0) {
									stream_wf_cb.disabled = false;
									stream_wf_cb.style.display = 'inline';
									stream_byp_cb.style.display = 'none';
								} else {
									stream_wf_cb.disabled = true;
								}

								if ((this.value & <?php echo SQ_SC_STATUS_ALL_APPROVED ?>) > 0) {
									stream_byp_cb.disabled = false;
									stream_byp_cb.style.display = 'inline';
									stream_wf_cb.style.display = 'none';
								} else {
									stream_byp_cb.disabled = true;
								}
							}
						// ]]></script>
						<?php
					$stream_box = ob_get_clean();
					echo str_replace($cascade_box, $stream_box.$cascade_box, $status_output);
					if (isset($this->_tmp['incomplete_metadata'])) {
						$new_status = isset($_POST[$prefix.'_change_status']) ? ' to \''.get_status_description($_POST[$prefix.'_change_status']).'\'' : '';
						echo "<p>".sprintf(translate('Asset status cannot be changed%s. Following assets are missing some mandatory metadata fields.'), $new_status);

						foreach (array_keys($this->_tmp['incomplete_metadata']) as $incomplete_meta_assetid) {
							$assetinfo = get_asset_tag_line($incomplete_meta_assetid, 'metadata');
							echo "<br />$assetinfo";
						}
						echo "</p>";
					}
				} else {
					echo $status_output;
				}
			} else {
				echo $status_output;
			}
		}

		return $need_commit;

	}//end paintCustomStatusChange()


	/**
	* Handles custom processing of the Status Change field
	*
	* Makes it use the "Edit Bundled Asset Status" HIPO instead of the normal
	* "Edit Status" HIPO.
	*
	* NB: called from the paint function since there is nothing that should
	* stop this from running (since being allowed to change is determined by
	* the status object). If we were to call it from the edit interface XML,
	* the lack of write access would block anything from happening.
	*
	* @param Workflow_Bundle	$asset	The asset being processed
	* @param Backend_Outputter	$o		The outputter class being used
	* @param string				$prefix	Prefix for form elements
	*
	* @return boolean
	*/
	function processCustomStatusChange(Workflow_Bundle $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_POST[$prefix.'_change_status'])) {
			$dependants_only = (isset($_POST[$prefix.'_change_status_all_children']) && $_POST[$prefix.'_change_status_all_children']) ? FALSE : TRUE;
			if ($_POST[$prefix.'_change_status'] > 0 || !$dependants_only) {
				$new_status = 0;
				if ($_POST[$prefix.'_change_status'] == 0 && !$dependants_only) {
					$new_status = $asset->status;
				} else if ($_POST[$prefix.'_change_status'] > 0) {
					$new_status = $_POST[$prefix.'_change_status'];
				}
				// before we try to change the asset status lets see if the
				// required metadata fields are set or else notify user abt it
				$this->_checkRequiredMetadataComplete($asset);

				if (!isset($this->_tmp['incomplete_metadata'])) {
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$vars = Array(
								'bundle_assetid'	=> $asset->id,
								'new_status'		=> $new_status,
							);
					if (isset($_POST[$prefix.'_change_status_stream'])) {
						if ($_POST[$prefix.'_change_status_stream'] !== 'SQ_USE_DEFAULT') {
							$vars['workflow_stream'] = $_POST[$prefix.'_change_status_stream'];
						}
					}
					$hh->queueHipo('hipo_job_edit_bundled_assets_status', $vars);
					return TRUE;
				}
			}
		}

		return FALSE;

	}//end processCustomStatusChange()


	/**
	* Handles painting of the Future Status section
	*
	* Here since it needs custom processing. Much the same as printing the future
	* status sections in the normal Details static screen.
	*
	* @param Workflow_Bundle	$asset	The asset being processed
	* @param Backend_Outputter	$o		The outputter class being used
	* @param string				$prefix	Prefix for form elements
	*
	* @return boolean
	*/
	function paintFutureStatus(Workflow_Bundle $asset, Backend_Outputter $o, $prefix)
	{
		$ok = FALSE;

		require_once SQ_INCLUDE_PATH.'/asset_static_screens/details.inc';
		ob_start();
			$need_commit = _paintDetailsFutureStatusCurrent($asset, $o, $prefix);
			$ok = $ok || $need_commit;
		$current_fs_contents = ob_get_clean();

		ob_start();
			$need_commit = _paintDetailsFutureStatusNew($asset, $o, $prefix);
			$ok = $ok || $need_commit;
		$new_fs_contents = ob_get_clean();

		// Only show current contents if there are set FS jobs
		if (empty($current_fs_contents) === FALSE) {
			$o->openField(translate('Scheduled Future Status Changes'), 'new_line');

				echo $current_fs_contents;
			$o->closeField();
		}

		// Only show Add FS section if write access allowed
		if ($asset->writeAccess()) {
			$o->openField(translate('Add New Future Status'), 'new_line');

				echo $new_fs_contents;
			$o->closeField();
		}//end if

		return $ok;

	}//end paintFutureStatus()


	/**
	* Handles processing of the New Future Status section
	*
	* @param Workflow_Bundle	$asset	The asset being processed
	* @param Backend_Outputter	$o		The outputter class being used
	* @param string				$prefix	Prefix for form elements
	*
	* @return boolean
	*/
	function processFutureStatus(Workflow_Bundle $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/asset_static_screens/details.inc';
		$need_commit = _processDetailsFutureStatusNew($asset, $o, $prefix);

		return $need_commit;

	}//end processFutureStatus()


	/**
	* Paints the custom workflow screen for the bundle
	*
	* @param Workflow_Bundle	$asset	The asset being processed
	* @param Backend_Outputter	$o		The outputter class being used
	* @param string				$prefix	Prefix for form elements
	*
	* @return boolean
	*/
	public function paintWorkflow(Workflow_Bundle $asset, Backend_Outputter $o, $prefix)
	{
		// Cheat the system, since we shouldn't need write access
		// so long as we have the right permissions to approve workflow
		if ($this->processWorkflow($asset, $o, $prefix) === TRUE) {
			// Was a HIPO (for bundle status change) queued? If so, then point
			// to the URL so it runs
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$url = $hh->runQueuedJobs();
			if (empty($url) === FALSE) $o->setRedirect($url);
			return FALSE;
		}

		// Otherwise, paint the approval interface - or the message saying you
		// can't do it here because no workflow is running
		if ((int)$asset->status === SQ_STATUS_PENDING_APPROVAL) {
			if ($this->showIfCanApprove($asset) === TRUE) {
				$o->openField(translate('Log Message'));

					text_area($prefix.'_workflow_new_log_message', '', 60, 6);
					if ($this->bundleRequiresComment($asset) === TRUE) {
						$comment_button_js = "elt = document.getElementById('".$prefix."_workflow_new_log_message'); if (elt && elt.value.replace(".'/^\s+|\s+$/g'.", '').length == 0) { alert(js_translate('You must enter a comment for this workflow step.')); } else { submit_form(); }";

						$o->note(translate('You must enter the reason for your decision in the box above so that other approvers know why you have accepted or rejected the changes. At least one bundled asset requires you do this.'));

					} else {
						$comment_button_js = 'submit_form();';
						$o->note(translate('You can enter a reason for your decision in the box above so that other approvers know why you have accepted or rejected the changes.'));

					}
				$o->closeField();
				$o->openField('');
					$button_js = 'document.getElementById("'.$prefix.'_workflow_new_status").value = %status%; '.$comment_button_js;

					hidden_field($prefix.'_workflow_new_status', '0');
					$available_statuses = $asset->getAvailableStatii();

					$reject_status = NULL;
					$approve_status = NULL;

					foreach ($available_statuses as $new_status => $status_text) {
						if ((int)$new_status < (int)$asset->status) {
							$reject_status = Array($new_status => $status_text);
						} else {
							$approve_status = Array($new_status => $status_text);
						}
					}

					// Put the approve button first, if any
					if ($approve_status !== NULL) {
						normal_button($prefix.'_workflow_status_update_approve', current($approve_status), str_replace('%status%', (string)(key($approve_status)), $button_js));
					}

					if ($reject_status !== NULL) {
						normal_button($prefix.'_workflow_status_update_reject', current($reject_status), str_replace('%status%', (string)(key($reject_status)), $button_js));
					}
					if (isset($this->_tmp['incomplete_metadata'])) {
						$new_status = isset($_POST[$prefix.'_workflow_new_status']) ? ' to \''.get_status_description($_POST[$prefix.'_workflow_new_status']).'\'' : '';
						echo "<p>".sprintf(translate('Asset status cannot be changed%s. Following assets are missing some mandatory metadata fields.'), $new_status);

						foreach (array_keys($this->_tmp['incomplete_metadata']) as $incomplete_meta_assetid) {
							$assetinfo = get_asset_tag_line($incomplete_meta_assetid, 'metadata');
							echo "<br />$assetinfo";
						}
						echo "</p>";
					}

				$o->closeField();
			} else {
				$o->openField(translate('Workflow Status'));

					echo translate('To be able to approve or reject workflow at a bundle level, you must be able to approve or reject at least one bundled asset.');

				$o->closeField();
			}
		} else {
			$o->openField(translate('Workflow Status'));

				echo translate('This bundle must be in "Approving" status to be able to approve or reject workflow at a bundle level.');

			$o->closeField();
		}

		return FALSE;

	}//end paintWorkflow()


	/**
	* Processes the custom workflow screen for the bundle
	*
	* NB: called from the paint function since there is nothing that should
	* stop this from running (since being allowed to change is determined by
	* being in a current workflow step). If we were to call it from the edit
	* interface XML, the lack of write access would block anything from happening.
	*
	* @param Workflow_Bundle	$asset	The asset being processed
	* @param Backend_Outputter	$o		The outputter class being used
	* @param string				$prefix	Prefix for form elements
	*
	* @return boolean
	*/
	public function processWorkflow(Workflow_Bundle $asset, Backend_Outputter $o, $prefix)
	{
		if ($this->showIfCanApprove($asset) === FALSE) {
			return FALSE;
		}

		if (isset($_POST[$prefix.'_workflow_new_status'])) {
			$new_status = (int)$_POST[$prefix.'_workflow_new_status'];
			$log_message = array_get_index($_POST, $prefix.'_workflow_new_log_message', '');

			// before we try to change the asset status lets see if the
			// required metadata fields are set or else notify user abt it
			// if we are rejecting the workflow then its alright but
			// don't approve the changes if the metadata is gone missing
			if (!($new_status & SQ_STATUS_UNDER_CONSTRUCTION)) $this->_checkRequiredMetadataComplete($asset);

			if (!isset($this->_tmp['incomplete_metadata'])) {
				if ($new_status > 0) {
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$vars = Array(
								'bundle_assetid'	=> $asset->id,
								'new_status'		=> $new_status,
								'log_message'		=> $log_message,
							);
					$hh->queueHipo('hipo_job_edit_bundled_assets_status', $vars);
					return TRUE;
				}
			}
		}

		return FALSE;

	}//end processWorkflow()


	/**
	* Show-if for the custom workflow screen
	*
	* Show the custom workflow if:
	* - We are an admin for the asset (since we can always reject), or
	* - We can approve at least one asset in the bundle
	*
	* @param Workflow_Bundle	$asset	The bundle to test
	*
	* @return boolean
	*/
	public function showIfCanApprove(Workflow_Bundle $asset)
	{
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		$bundled_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE);
		foreach ($bundled_asset_links as $linkid => $bundled_asset_link) {
			$bundled_assetid = $bundled_asset_link['minorid'];
			$approvers       = $wfm->whoCanPublish($bundled_assetid);

			// If the current user is in the workflow's current step, then
			// all is good - there is something to approve
			if (in_array($current_userid, $approvers) === TRUE) {
				return TRUE;
			} else {
				$bundled_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($bundled_assetid);
				if ($bundled_asset->adminAccess() === TRUE) {
					// We have admin access to the bundled asset
					return TRUE;
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bundled_asset);
				unset($bundled_asset);
			}
		}

		return FALSE;

	}//end showIfCanApprove()


	/**
	* Test whether approving or rejecting the bundle requires a comment
	*
	* If at least one of the currently in-workflow-step assets requires a
	* comment (either through being marked as such for the current workflow
	* step, or through "unlisted users"), then the bundle should require one
	* to proceed.
	*
	* @param Workflow_Bundle	$asset	The bundle to test
	*
	* @return boolean
	*/
	public function bundleRequiresComment(Workflow_Bundle $asset)
	{
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		$bundled_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE);
		foreach ($bundled_asset_links as $linkid => $bundled_asset_link) {
			$bundled_assetid = $bundled_asset_link['minorid'];

			// If the current user would be required to enter a comment, then
			// we don't need to go any further - the bundle needs a comment
			if ($wfm->requiresComment($bundled_assetid, $current_userid) === TRUE) {
				return TRUE;
			}
		}

		return FALSE;

	}//end showIfCanApprove()


	/**
	* Checks if the assets under the bundle have the required metadata
	* populated.
	*
	* @param Workflow_Bundle	$asset	The bundle to test
	*
	* @return boolean
	*/
	private function _checkRequiredMetadataComplete(Workflow_Bundle $asset)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE);
		$mm    = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$not_cool_assets = Array();

		foreach ($links as $link) {
			$assetid = $link['minorid'];
			if (!isset($this->_tmp['incomplete_metadata'][$assetid]) && !$mm->requiredFieldsComplete($assetid)) {
				$this->_tmp['incomplete_metadata'][$assetid] = TRUE;
			}
		}//end foreach

	}//end _checkRequiredMetadataComplete


}//end class

?>
