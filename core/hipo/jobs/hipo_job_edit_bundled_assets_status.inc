<?php


require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';


class HIPO_Job_Edit_Bundled_Assets_Status extends HIPO_Job
{


	/**
	* Defines the "usual" statuses for each new bundle status
	* Anything other than these should be considered worth throwing a warning for
	*
	* Indexed by (new bundle status => bit-mask of asset statuses)
	*
	* @var array
	*/
	protected $_usual_statuses = Array(
									SQ_STATUS_UNDER_CONSTRUCTION	=> SQ_SC_STATUS_CAN_APPROVE,
									SQ_STATUS_PENDING_APPROVAL		=> SQ_SC_STATUS_PENDING,
									SQ_STATUS_APPROVED			 	=> SQ_SC_STATUS_ALL_APPROVED,
									SQ_STATUS_LIVE					=> SQ_STATUS_LIVE,
								 );



	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_EDIT_BUNDLED_ASSETS_STATUS_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 1),
			   );

	}//end getConfigVars()


	/**
	* Return this job's threshold percentage
	*
	* This is the percentage of this HIPO Job's threshold that would run if the
	* job is freestyled (eg. if threshold is 10 assets, and a job would process
	* 20 assets, then this should return 200%).
	*
	* This is used in conjunction with the "Freestyle Threshold" config var
	* (SQ_HIPO_TOTAL_THRESHOLD) to determine whether a job that doesn't care how
	* its run is run through HIPO, or "freestyled" (run in one hit). This
	* setting normally defaults to 80% (ie. if threshold is 10 assets, it would
	* try to freestyle if processing 8 assets or less).
	*
	* If multiple HIPOs are queued and can fit underneath this figure, then they
	* will be freestyled until the combined total rises above it.
	*
	* Return zero (0) to force the job to run through HIPO mode.
	*
	* @return float
	*/
	public function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_EDIT_BUNDLED_ASSETS_STATUS_THRESHOLD < 1) {
			return 0;
		}

	}//end getThresholdPercentageRequired()





	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Edit Bundle Status');


	}//end getHipoName()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Edit Status HIPO appends the ID of the asset that the status is being edited for
	* (the top most parent) so that any other status jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['bundle_assetid'];

	}//end getCodeName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				0	=> Array(
						'name'			=> translate('Editing Status of Bundled Assets'),

						'function_call'	=> Array(
											'paint_function'	=> 'paintStatusChangeWarning',
											'process_function'	=> 'processStatusChangeWarning',
										   ),
						'running_mode'	=> 'web',
						'auto_step'		=> FALSE,
						'skip_step'		=> FALSE,
						'allow_cancel'	=> TRUE,
						'percent_done'	=> 0,
						'complete'		=> FALSE,
						'message'		=> '',
					   ),
				1	=> Array(
						'name'			=> translate('Editing Status of Bundled Assets'),

						'function_call'	=> Array(
											'process_function'	=> 'processBundledAsset',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> TRUE,
						'allow_cancel'	=> TRUE,
						'percent_done'	=> 0,
						'complete'		=> FALSE,
						'message'		=> '',
					   ),
				2	=> Array(
						'name'			=> translate('Sending Bundle Workflow Message'),

						'function_call'	=> Array(
											'process_function'	=> 'sendBundleWorkflowMessage',
										   ),
						'running_mode'	=> 'server',
						'auto_step'		=> TRUE,
						'allow_cancel'	=> TRUE,
						'percent_done'	=> 0,
						'complete'		=> FALSE,
						'message'		=> '',
					   ),
			   );

	}//end getInitialStepData()


	/**
	* Prepare and validate the HIPO vars before running
	*
	* @return boolean
	*/
	public function prepare()
	{
		// We must have a bundle asset ID
		if (array_key_exists('bundle_assetid', $this->_running_vars) === FALSE) {
			trigger_localised_error('HIPO0092', translate('Unable to run HIPO job; this job requires a bundle asset ID'), E_USER_WARNING);
			return FALSE;
		}

		// Validate the bundle asset ID and ensure it's actually a bundle
		$bundle_assetid = $this->_running_vars['bundle_assetid'];
		$type_code = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($bundle_assetid, '', TRUE, 'type_code');

		// The asset doesn't exist...
		if (count($type_code) === 0) {
			trigger_localised_error('HIPO0093', sprintf(translate('Unable to run HIPO job; the passed bundle asset ID (#%s) does not exist'), $bundle_assetid), E_USER_WARNING);
			return FALSE;
		}

		// It's not a bundle...
		if ($type_code[$bundle_assetid] !== 'workflow_bundle') {
			trigger_localised_error('HIPO0094', sprintf(translate('Unable to run HIPO job; the passed bundle asset ID (#%s) is not a Workflow Bundle asset'), $bundle_assetid), E_USER_WARNING);
			return FALSE;
		}

		if (array_key_exists('workflow_stream', $this->_running_vars) === FALSE) {
			$this->_running_vars['workflow_stream'] = '';
		}

		$todo_assetids = $this->_getBundledAssets($bundle_assetid);

		$this->_running_vars['todo_assetids'] = Array();
		$this->_running_vars['warn_assetids'] = Array();

		$bundle_status = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($bundle_assetid), '', FALSE, 'status');
		$this->_running_vars['bundle_current_status'] = (int)$bundle_status[$bundle_assetid];

		foreach ($todo_assetids as $todo_assetid) {
			$new_asset_status = $this->_getNewAssetStatus($todo_assetid, (int)$this->_running_vars['bundle_current_status'], (int)$this->_running_vars['new_status'], $this->_running_vars['workflow_stream']);
			$this->_running_vars['todo_assetids'][$todo_assetid] = $new_asset_status;

			// Is this is not an asset status that we expect?
			if ($new_asset_status !== NULL) {
				if (((int)$new_asset_status & $this->_usual_statuses[(int)$this->_running_vars['new_status']]) === 0) {
					$this->_running_vars['warn_assetids'][$new_asset_status][] = $todo_assetid;
				}
			}
		}

		if (count($this->_running_vars['warn_assetids']) === 0) {
			$this->skipStep(0);
		}

		$this->_running_vars['done_root'] = FALSE;
		$this->_running_vars['done_assetids'] = Array();

		$this->_running_vars['msg_filters'] = Array();

		return parent::prepare();

	}//end prepare()


	/**
	* Paint information about the process of the HIPO
	*
	* This function calls the paint function for the step currently being run and also outputs
	* a progress bar to indicate to the user where they are in the HIPO process
	*
	* @param Backend_Outputter	&$o		Backend_Outputter, reference to the backend outputter
	* @param boolean			$nested	TRUE if the steps are nested
	*
	* @return void
	* @access public
	*/
	function paint(&$o, $nested=FALSE)
	{
		parent::paint($o, $nested);

		// paint dependant hipo job here
		if (!empty($this->_running_vars['sub_job'])) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$sub_job = $hh->getJob($this->_running_vars['sub_job']);
			if (!is_null($sub_job)) {
				$sub_job->paint($o, TRUE);
			} else {
				unset($this->_running_vars['sub_job']);
			}
		}

	}//end paint()


	/**
	* Run this HIPO quietly, and in one go
	*
	* @return boolean
	*/
	public function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids']) || !empty($this->_running_vars['sub_job'])) {
			if ($this->processBundledAsset($this->_steps[0], get_class($this), TRUE) !== TRUE) {
				return FALSE;
			}
		}

		if ($this->sendBundleWorkflowMessage($this->_steps[1], get_class($this), TRUE) !== TRUE) {
			return FALSE;
		}

		return TRUE;

	}//end freestyle()


	/**
	* Paint the "status change warning" step
	*
	* Only painted if there are assets that we need to warn about. For instance,
	* if we are going from "Editing" to "Approving", any assets that will go to
	* something other than a "in workflow" status (eg. to Live since there is
	* no workflow, or Approved to Go Live due to being able to complete the
	* workflow) would need attention drawn to.
	*
	* @param array					&$step_data	a reference to the array of information about the current step
	* @param Hipo_Backend_Outputter	$o			The backend outputter used for painting this HIPO
	* @param string					$prefix		prefix for form vars
	*
	* @return boolean
	*/
	public function paintStatusChangeWarning(Array &$step_data, Hipo_Backend_Outputter $o, $prefix)
	{
		$o->openSection(translate('Editing Status of Bundled Assets'));


		$o->openRaw();
			echo translate('<p>The following bundled assets will not follow the change of status expected from this bundle status change, and will instead change to the status(es) listed below. This may be due to a lack of workflow on the affected assets, or due to your ability to complete any workflow on them straight away.<br/><br/>

Please check that these intended statuses are correct, and if desired, manually change the affected assets\' status and/or workflow before continuing.<br/><br/>

If you are ready to continue, please click <strong>Next</strong> to proceed with the bundle status change.</p>');

		$o->closeRaw();

		foreach ($this->_running_vars['warn_assetids'] as $status => $warn_assetids) {
			$o->openSection(sprintf(translate('New Status: %1$s %2$s'), get_asset_status_icon($status), get_status_description($status)));

			$o->openRaw();
			?><ul><?php
			foreach ($warn_assetids as $assetid) {
				?><li><?php
				echo get_asset_tag_line($assetid);
				?></li><?php
			}
			?></ul><?php
			$o->closeRaw();
			$o->closeSection();
		}

		$o->closeSection();

		return TRUE;

	}//end paintStatusChangeWarning()


	/**
	* Process the "status change warning" step
	*
	* We need this solely to make sure the status change warning triggers - or
	* not - depending on whether we need it.
	*
	* @param array		&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	*/
	public function processStatusChangeWarning(Array &$step_data, $prefix)
	{
		$complete = (($step_data['percent_done'] > 0) || (empty($this->_running_vars['warn_assetids']) === TRUE));

		if ($complete === FALSE) {
			$step_data['percent_done'] = 50;
		} else {
			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		}

		return TRUE;

	}//end processStatusChangeWarning()


	/**
	* Process a single bundled asset
	*
	* @param array		&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	* @param boolean	$freestyle	TRUE = this HIPO is being freestyled
	*
	* @return boolean
	*/
	public function processBundledAsset(&$step_data, $prefix, $freestyle=FALSE)
	{
		$run_level_set = FALSE;
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		if (empty($this->_running_vars['sub_job']) === FALSE) {
			$sub_job = $hh->getJob($this->_running_vars['sub_job']);
			if (empty($sub_job) === TRUE) {
				return FALSE;
			}
		} else {
			$current_status   = (int)$this->_running_vars['bundle_current_status'];
			$new_status       = (int)$this->_running_vars['new_status'];

			if ($this->_running_vars['done_root'] === FALSE) {
				$assetid = $this->_running_vars['bundle_assetid'];
				$new_asset_status = $this->_running_vars['new_status'];
				$this->_running_vars['done_root'] = TRUE;
			} else {
				$new_asset_status = reset($this->_running_vars['todo_assetids']);
				$assetid          = key($this->_running_vars['todo_assetids']);
				unset($this->_running_vars['todo_assetids'][$assetid]);
			}

			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (isset($this->_running_vars['no_status_integrity'][$assetid])) {
				$run_level = $GLOBALS['SQ_SYSTEM']->getRunLevel();
				$GLOBALS['SQ_SYSTEM']->setRunLevel($run_level ^ SQ_SECURITY_STATUS_INTEGRITY);
				$run_level_set = TRUE;
			}

			// NULL asset status means we should be skipping this asset instead
			if ($new_asset_status !== NULL) {
				$code_name = $this->_prepareStatusChangeHIPO($assetid, $new_asset_status);
				$asset_current_status = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), 'asset', FALSE, 'status');
				$asset_current_status = $asset_current_status[$assetid];

				// If the asset is currently in workflow, send a log message.
				// (It does not matter if the change itself goes through - this
				// is consistent with normal Workflow screen workings, where the
				// log is logged first, then the status change run)
				if (empty($this->_running_vars['log_message']) === FALSE) {
					if (($asset_current_status & SQ_SC_STATUS_PENDING) > 0) {
						$this->_sendUserLogMessage($assetid, $this->_running_vars['log_message']);
					}
				}

				$this->_running_vars['sub_job'] = $code_name;
				$sub_job = $hh->getJob($code_name);
				$step_data['message'] = sprintf(translate('Editing status of "%1$s" (Id: #%2$s)'), htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $assetid);

			} else {
				$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

			}
		}

		$filters = Array(
					'start'					=> 'asset.workflow.invitation',
					'progress'				=> 'asset.workflow.invitation.progress',
					'approve'				=> 'asset.workflow.announce.approve',
					'reject'				=> 'asset.workflow.announce.reject',
					'auto_approve_progress'	=> 'asset.workflow.announce.started',
				   );

		$msg_types = $this->_getRequiredBundleMessages($this->_running_vars['bundle_current_status'], $this->_running_vars['new_status']);
		$used_filters = Array();
		foreach ($msg_types as $msg_type) {
			$used_filters[$msg_type] = $filters[$msg_type];
		}

		if (empty($sub_job) === FALSE) {
			if ($freestyle === TRUE) {
				// in stereo, with tha freestylah =P
				if ($sub_job->freestyle() === TRUE) {
					$running_vars = $sub_job->getRunningVars();
					$this->_running_vars['done_assetids'][] = $running_vars['assetid'];

					// If we are not the bundle, examine the messages we get,
							// filter the invite ones out
					if ((string)$running_vars['assetid'] !== (string)$this->_running_vars['bundle_assetid']) {
						$job_messages = array_get_index($running_vars, 'messages', Array());
						$this->_filterMessages($running_vars['assetid'], $job_messages, $used_filters);
					} else {
						$job_messages = Array();
					}

					// Grab the errors from the status sub-job, since we need
					// to show them to the user afterwards
					$this->_hipo_vars['errors'] = array_merge($this->_hipo_vars['errors'], $sub_job->_hipo_vars['errors']);
					unset($this->_running_vars['sub_job']);
					$sub_job->save();
					$hh->uncacheJob($sub_job->code_name);
				} else {
					// Failed to process! Get the errors at least, and bail
					$this->_hipo_vars['errors'] = array_merge($this->_hipo_vars['errors'], $sub_job->_hipo_vars['errors']);
					return FALSE;
				}
			} else {
				if ($sub_job->process() === TRUE) {
					if ($sub_job->complete() === TRUE) {
						// process the sub hipo on each refresh
						$running_vars = $sub_job->getRunningVars();
						if (isset($running_vars['assetid'])) {
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($running_vars['assetid']);
							$asset->_updated(FALSE);
							$this->_running_vars['done_assetids'][] = $running_vars['assetid'];

							// If we are not the bundle, examine the messages we get,
							// filter the invite ones out
							if ((string)$running_vars['assetid'] !== (string)$this->_running_vars['bundle_assetid']) {
								$job_messages = array_get_index($running_vars, 'messages', Array());
								$this->_filterMessages($running_vars['assetid'], $job_messages, $used_filters);
							} else {
								$job_messages = Array();
							}
						}

						// Grab the errors from the status sub-job, since we need
						// to show them to the user afterwards
						$this->_hipo_vars['errors'] = array_merge($this->_hipo_vars['errors'], $sub_job->_hipo_vars['errors']);
						unset($this->_running_vars['sub_job']);
					}
				} else {
					// Failed to process! Get the errors at least, and bail
					$this->_hipo_vars['errors'] = array_merge($this->_hipo_vars['errors'], $sub_job->_hipo_vars['errors']);
					return FALSE;
				}
			}
		}

		if (empty($job_messages) === FALSE) {
			// Send any messages we have left - these would be log
			// messages, so these should still be passed on as usual
			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$ms->send($job_messages);
		}


		if (empty($this->_running_vars['todo_assetids'])) {
			if (empty($this->_running_vars['sub_job'])) {
				// todo array is empty, and no dependant job left
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
			} else {
				// todo array is empty, but sub-hipo is processing the dependants
				// so we'll continue until it completes
				$step_data['complete']     = FALSE;
			}
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		if ($run_level_set && isset($this->_running_vars['no_status_integrity'][$assetid])) {
			unset($this->_running_vars['no_status_integrity'][$assetid]);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		return TRUE;

	}//end processBundledAsset()


	/**
	* Send the relevant workflow bundle message for this status change
	*
	* @param Array		&$step_data	HIPO data for the current step
	* @param string		$prefix		Prefix for form elements
	* @param boolean	$freestyle	TRUE = this HIPO is being freestyled
	*
	* @return boolean
	*/
	public function sendBundleWorkflowMessage(&$step_data, $prefix, $freestyle=FALSE)
	{
		$bundle = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['bundle_assetid']);

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$msg_list = Array();
		$msg_types = Array();

		// Convert from our message types to internal message type codes
		$im_types = Array();

		$im_types['start'] = Array(
		    'subject' => translate('Workflow Approval Required'),
		    'body'    => translate('The bundle "%bundle_name%" has been placed into workflow, along with its bundled assets.')."\n\n".
                            translate('There are assets within this bundle that require your review, and you are now required to approve or reject the changes made to them so the workflow process can continue:')."\n\n".
                            '%asset_list%'."\n\n".
                            translate('You can approve or reject the above assets one at a time. Clicking on the asset name will take you to its Workflow screen, where you may take action.')."\n".
                            translate('You may also <SQ_MSG_LINK href="%bundle_approve_url%">Approve</SQ_MSG_LINK> all assets for which you have access to workflow, or <SQ_MSG_LINK href="%bundle_reject_url%">Reject</SQ_MSG_LINK> the entire bundle\'s changes.'),
		    'code'    => 'asset.workflow.bundle.invitation',
		);

		$im_types['progress'] = Array(
		    'subject' => translate('Workflow Approval Required'),
		    'body'    => translate('The bundle "%bundle_name%" has been placed into workflow, along with its bundled assets.')."\n\n".
                            translate('There are assets within this bundle that have progressed in their workflow, and now require your review. You are now required to approve or reject the changes made to them so the workflow process can continue:')."\n\n".
                            '%asset_list%'."\n\n".
                            translate('You can approve or reject the above assets one at a time. Clicking on the asset name will take you to its Workflow screen, where you may take action.')."\n".
                            translate('You may also <SQ_MSG_LINK href="%bundle_approve_url%">Approve</SQ_MSG_LINK> all assets for which you have access to workflow, or <SQ_MSG_LINK href="%bundle_reject_url%">Reject</SQ_MSG_LINK> the entire bundle\'s changes.'),
		    'code'    => 'asset.workflow.bundle.invitation.progress',
		);

		$im_types['approve'] = Array(
		    'subject' => translate('Asset Changes Approved'),
		    'body'    => translate('User "%workflow_user%" has approved the changes to the following assets in the bundle "%bundle_name%":')."\n\n".'%asset_list%',
		    'code'    => 'asset.workflow.bundle.announce.approve',
		);

		$im_types['reject'] = Array(
		    'subject' => translate('Asset Changes Rejected'),
		    'body'    => translate('User "%workflow_user%" has approved the changes to the following assets in the bundle "%bundle_name%":')."\n\n".
		                    '%asset_list%'."\n\n".
		                    translate('The status of the bundle "%bundle_name%" is now "%new_status%"'),
		    'code'    => 'asset.workflow.bundle.announce.reject',
		);

		$im_types['auto_approve_progress'] = Array(
		    'subject' => translate('Workflow Approval Required'),
		    'body'    => translate('The bundle "%bundle_name%" has been placed into workflow, along with its bundled assets.')."\n\n".
                            translate('There are assets within this bundle that require your review, and you are now required to approve or reject the changes made to them so the workflow process can continue:')."\n\n".
                            '%asset_list%'."\n\n".
                            translate('You can approve or reject the above assets one at a time. Clicking on the asset name will take you to its Workflow screen, where you may take action.')."\n".
                            translate('You may also <SQ_MSG_LINK href="%bundle_approve_url%">Approve</SQ_MSG_LINK> all assets for which you have access to workflow, or <SQ_MSG_LINK href="%bundle_reject_url%">Reject</SQ_MSG_LINK> the entire bundle\'s changes.'),
		    'code'    => 'asset.workflow.bundle.started',
		);

		$msg_types = $this->_getRequiredBundleMessages($this->_running_vars['bundle_current_status'], $this->_running_vars['new_status']);

		if (count($msg_types) > 0) {
			foreach ($msg_types as $msg_type) {
				if (empty($this->_running_vars['msg_filters'][$msg_type]) === FALSE) {
					foreach ($this->_running_vars['msg_filters'][$msg_type] as $userid => $assets) {
						$asset_list_lines = Array();

						foreach ($assets as $assetid) {
							$workflow_url = $this->_getBackendScreenKeywordReplacement($assetid, 'workflow');
							$preview_url  = $this->_getBackendScreenKeywordReplacement($assetid, 'preview');

							$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetid);
							$format_line = '<a href="'.$workflow_url.'">'.$asset_info[$assetid]['name'].' (Id: #'.$assetid.')</a> [<a href="'.$preview_url.'">Preview</a>]';

							$asset_list_lines[] = $format_line;

						}//end foreach asset

						$body = trim($bundle->attr($msg_type.'_workflow_msg'));
						$subject = trim($bundle->attr($msg_type.'_workflow_subject'));
						$keywords = extract_keywords($body);

						$replacements = Array();

						// Bundle keywords
						$replacements['bundle_reject_url'] = $this->_getBackendScreenKeywordReplacement($bundle->id, 'workflow').'&asset_version='.$bundle->version.'&workflow_link_action=reject';
						$replacements['bundle_approve_url'] = $this->_getBackendScreenKeywordReplacement($bundle->id, 'workflow').'&asset_version='.$bundle->version.'&workflow_link_action=approve';
						$replacements['bundle_workflow_url'] = $this->_getBackendScreenKeywordReplacement($bundle->id, 'workflow');
						$replacements['bundle_name'] = $bundle->name;
						$replacements['bundle_assetid'] = $bundle->id;
						$replacements['new_status'] = $bundle->getStatus()->getDescription();

						// Asset list
						if (count($asset_list_lines) > 0) {
							$replacements['asset_list'] = '<ul><li>'.implode('</li><li>', $asset_list_lines).'</li></ul>';
						} else {
							$replacements['asset_list'] = '';
						}

						// User keywords
						$replacements['workflow_user'] = $GLOBALS['SQ_SYSTEM']->user->short_name;
						$replacements['workflow_user_first_name'] = $GLOBALS['SQ_SYSTEM']->user->attr('first_name');
						$replacements['workflow_user_last_name'] = $GLOBALS['SQ_SYSTEM']->user->attr('last_name');
						$replacements['workflow_user_email'] = $GLOBALS['SQ_SYSTEM']->user->attr('email');

						$msg = $ms->newMessage(Array($userid), $im_types[$msg_type]['code'], $replacements);
						if ($subject !== '') {
							$msg->subject = $subject;
						} else {
						    $msg->subject = $im_types[$msg_type]['subject'];
						}

						if ($body !== '') {
							$msg->body = $body;
						} else {
						    $msg->body = $im_types[$msg_type]['body'];
						}

						$msg_list[] = $msg;
					}//end foreach item in this filter

				}//end if this filter type was used

			}//end foreach message type


		}//end if there are message types to take notice of

		if (count($msg_list) > 0) {
			$ms->send($msg_list);
		}

		$step_data['percent_done'] = 100;
		$step_data['complete'] = TRUE;

		return TRUE;

	}//end sendBundleWorkflowMessage()


	/**
	* Prepare a dependant status change HIPO for a single bundled asset
	*
	* Effects of dependant HIPOs spawned by this should be to dependant assets ONLY.
	*
	* Return FALSE if the sub-job couldn't be initialised, otherwise return the
	* code-name of it
	*
	* @param string	$assetid
	* @param int	$new_status
	*
	* @return mixed string|boolean
	*/
	protected function _prepareStatusChangeHIPO($assetid, $new_status)
	{
		require_once(SQ_SYSTEM_ROOT.'/core/hipo/jobs/hipo_job_edit_status.inc');

		$init_hipo = new Hipo_Job_Edit_Status();
		$options['auto_complete'] = TRUE;

		// Create a sub-job for this asset ID. Always dependants only.
		// Also stop it from sending messages, because we will send out our
		// own instead
		$running_vars = Array(
							'assetid'			=> $assetid,
							'new_status'		=> $new_status,
							'dependants_only'	=> TRUE,
							'send_messages'		=> FALSE,
						);

		if (array_key_exists('workflow_stream', $this->_running_vars) === TRUE) {
			$running_vars['workflow_stream'] = $this->_running_vars['workflow_stream'];
		}

		$init_hipo->setRunningVars($running_vars);
		foreach ($options as $k => $v) {
			$init_hipo->setOption($k, $v);
		}

		// create our sub-sub-hipo thingy, until this job is completed, we need
		// to process it in place of our todo assets
		$code_name = $init_hipo->initialise($this->source_code_name);
		if ($code_name) return $code_name;
		return FALSE;

	}//end prepareStatusChangeHIPO()


	/**
	* Get the bundled assets that we will be working upon
	*
	* Returned array will be just of asset ID, no specific key
	*
	* @param string	$bundle_assetid	The bundle that we are checking links of
	*
	* @return array
	*/
	protected function _getBundledAssets($bundle_assetid)
	{
		$linked_assets = Array();
		$linked_asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($bundle_assetid, SQ_LINK_NOTICE);

		foreach ($linked_asset_links as $linked_asset_link) {
			$linked_assets[] = $linked_asset_link['minorid'];
		}

		return $linked_assets;

	}//end _getBundledAssets()


	/**
	* Determines an asset's intended status, given the bundle's current status
	* and what it's changing to.
	*
	* Returns NULL if no status change should apply. The new status can be the
	* same as the old status if the old status supports it (eg. Pending Approval
	* => Pending Approval, for progressing a workflow).
	*
	* @param string	$assetid		The asset ID of the child asset
	* @param int	$bundle_status	The current status of the bundle
	* @param int	$new_status		The new status of the bundle
	*
	* @return int
	*/
	protected function _getNewAssetStatus($assetid, $bundle_status, $new_status, $workflow_stream='')
	{
		$status_changes = Array();

		// Sent the bundle asset...it changes to what it changes to
		if ($assetid === $this->_running_vars['bundle_assetid']) {
			return $new_status;
		}

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

		$asset_status = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), '', FALSE, 'status');
		$asset_status = (int)$asset_status[$assetid];

		$schemas = $wfm->getSchemas($assetid);

		switch ($bundle_status) {
			case SQ_STATUS_UNDER_CONSTRUCTION:
				if ($new_status === SQ_STATUS_UNDER_CONSTRUCTION) {
					// Reinforcing "Editing" status
					$status_changes = Array(
										// Don't allow any backward changes, but otherwise
										// get some assets up to speed
										SQ_STATUS_ARCHIVED				=> SQ_STATUS_UNDER_CONSTRUCTION,
										SQ_STATUS_UNDER_CONSTRUCTION	=> NULL,
										SQ_STATUS_PENDING_APPROVAL		=> NULL,
										SQ_STATUS_APPROVED				=> NULL,
										SQ_STATUS_LIVE					=> SQ_STATUS_EDITING,
										SQ_STATUS_LIVE_APPROVAL			=> NULL,
										SQ_STATUS_EDITING				=> NULL,
										SQ_STATUS_EDITING_APPROVAL		=> NULL,
										SQ_STATUS_EDITING_APPROVED		=> NULL,
									  );

				} else if ($new_status === SQ_STATUS_PENDING_APPROVAL) {
					// Changing from "Editing" to "Approving".
					$status_changes = Array(
										// Archived => Pending Approval is a known error
										// So is Live => Safe Edit Pending - setting to "Up for Review"
										// is not the best move here since we should have been in Editing
										// status before, which means we should be in Safe Edit at this point.
										// Otherwise, only forward jumps in status allowed (ie.
										// Approved statuses won't be rejected, but won't be made
										// live either)
										SQ_STATUS_ARCHIVED				=> SQ_STATUS_PENDING_APPROVAL,
										SQ_STATUS_UNDER_CONSTRUCTION	=> SQ_STATUS_PENDING_APPROVAL,
										SQ_STATUS_PENDING_APPROVAL		=> NULL,
										SQ_STATUS_APPROVED				=> NULL,
										SQ_STATUS_LIVE					=> SQ_STATUS_EDITING_APPROVAL,
										SQ_STATUS_LIVE_APPROVAL			=> NULL,
										SQ_STATUS_EDITING				=> SQ_STATUS_EDITING_APPROVAL,
										SQ_STATUS_EDITING_APPROVAL		=> NULL,
										SQ_STATUS_EDITING_APPROVED		=> NULL,
									  );

					if (count($schemas) === 0) {
						// There's actually no workflow here, so we can jump
						// to Live where we can
						// EXCEPT: Safe Edit - can only jump to S/E Approved
						//         (since changing to Live is a cancelling).
						//    AND: Safe Edit Approved - no change since this is
						//         still valid when no workflow exists

						// Valid changes
						$status_changes[SQ_STATUS_UNDER_CONSTRUCTION] = SQ_STATUS_LIVE;
						$status_changes[SQ_STATUS_EDITING]            = SQ_STATUS_EDITING_APPROVED;

						// This is still an error condition...
						$status_changes[SQ_STATUS_ARCHIVED]           = SQ_STATUS_LIVE;
						$status_changes[SQ_STATUS_LIVE]               = SQ_STATUS_EDITING_APPROVED;

						// The remainder are pretty much FIXME statuses without schemas,
						// so don't know whether it will error or not, but oh well
						$status_changes[SQ_STATUS_PENDING_APPROVAL]   = SQ_STATUS_APPROVED;
						$status_changes[SQ_STATUS_APPROVED]           = SQ_STATUS_LIVE;
						$status_changes[SQ_STATUS_EDITING_APPROVAL]   = SQ_STATUS_EDITING_APPROVED;

					} else if ($wfm->testPublish($assetid, $GLOBALS['SQ_SYSTEM']->currentUserId(), $workflow_stream) === TRUE) {
						// We would complete this workflow in one go

						// Valid changes
						$status_changes[SQ_STATUS_UNDER_CONSTRUCTION] = SQ_STATUS_APPROVED;
						$status_changes[SQ_STATUS_PENDING_APPROVAL]   = SQ_STATUS_APPROVED;
						$status_changes[SQ_STATUS_EDITING]            = SQ_STATUS_EDITING_APPROVED;
						$status_changes[SQ_STATUS_EDITING_APPROVAL]   = SQ_STATUS_EDITING_APPROVED;

						// This is still an error condition...
						$status_changes[SQ_STATUS_ARCHIVED]           = SQ_STATUS_APPROVED;
						$status_changes[SQ_STATUS_LIVE]               = SQ_STATUS_EDITING_APPROVED;

					}//end if workflow

				} else if ($new_status === SQ_STATUS_APPROVED) {
					// Changing from "Editing" to "Approved"
					// All possible workflows pre-determined to complete in one go.
					// Those assets in a workflow can go into their "approved to go live" status.

					$status_changes = Array(
										// Archived => Approved is a known error
										// Pending "from" statuses are probably a FIXME since we wouldn't
										// have offered a bundle status of Approved if we needed workflow.
										SQ_STATUS_ARCHIVED				=> SQ_STATUS_APPROVED,
										SQ_STATUS_UNDER_CONSTRUCTION	=> SQ_STATUS_APPROVED,
										SQ_STATUS_PENDING_APPROVAL		=> SQ_STATUS_APPROVED,
										SQ_STATUS_APPROVED				=> NULL,
										SQ_STATUS_LIVE					=> NULL,
										SQ_STATUS_LIVE_APPROVAL			=> SQ_STATUS_LIVE,
										SQ_STATUS_EDITING				=> SQ_STATUS_EDITING_APPROVED,
										SQ_STATUS_EDITING_APPROVAL		=> SQ_STATUS_EDITING_APPROVED,
										SQ_STATUS_EDITING_APPROVED		=> NULL,
									  );

					if (count($schemas) === 0) {
						// If we have no workflow, just push them live
						// (but not Safe Edit Approved, as this is valid when
						// there is no workflow)

						// Valid changes
						$status_changes[SQ_STATUS_UNDER_CONSTRUCTION] = SQ_STATUS_LIVE;

						// This is still an error condition...
						$status_changes[SQ_STATUS_ARCHIVED]           = SQ_STATUS_LIVE;

						// Still very much FIXME, but oh well
						$status_changes[SQ_STATUS_PENDING_APPROVAL]   = SQ_STATUS_LIVE;
						$status_changes[SQ_STATUS_APPROVED]           = SQ_STATUS_LIVE;
						$status_changes[SQ_STATUS_EDITING_APPROVAL]   = SQ_STATUS_EDITING_APPROVED;

					}//end if workflow is applied

				} else if ($new_status === SQ_STATUS_LIVE) {
					// Changing from "Editing" to "Live" since no assets have workflow
					// Make Live, whatever the consequencess

					$status_changes = Array(
										// Archived => Live is a known error
										// Pending => Live statuses are probably a FIXME since we
										// wouldn't have offered the bundle status if we had workflow
										// Safe Edit statuses can only safely go to Safe Edit Approved,
										// since a move to Live should be and Safe Edit => Live is
										// actually going backwards
										SQ_STATUS_ARCHIVED				=> SQ_STATUS_LIVE,
										SQ_STATUS_UNDER_CONSTRUCTION	=> SQ_STATUS_LIVE,
										SQ_STATUS_PENDING_APPROVAL		=> SQ_STATUS_LIVE,
										SQ_STATUS_APPROVED				=> SQ_STATUS_LIVE,
										SQ_STATUS_LIVE					=> NULL,
										SQ_STATUS_LIVE_APPROVAL			=> SQ_STATUS_LIVE,
										SQ_STATUS_EDITING				=> SQ_STATUS_EDITING_APPROVED,
										SQ_STATUS_EDITING_APPROVAL		=> SQ_STATUS_EDITING_APPROVED,
										SQ_STATUS_EDITING_APPROVED		=> SQ_STATUS_LIVE,
									  );

				}//end if status

			break;

			case SQ_STATUS_PENDING_APPROVAL:
				if ($new_status === SQ_STATUS_PENDING_APPROVAL) {
					// Reinforce "Approving" status
					// Progress the workflow

					$status_changes = Array(
										// Archived => Pending is a known error
										// Live => Safe Edit Pending is a known error, since we actually
										// have to jump to Safe Edit first
										SQ_STATUS_ARCHIVED				=> SQ_STATUS_PENDING_APPROVAL,
										SQ_STATUS_UNDER_CONSTRUCTION	=> SQ_STATUS_PENDING_APPROVAL,
										SQ_STATUS_PENDING_APPROVAL		=> SQ_STATUS_PENDING_APPROVAL,
										SQ_STATUS_APPROVED				=> NULL,
										SQ_STATUS_LIVE					=> SQ_STATUS_EDITING_APPROVAL,
										SQ_STATUS_LIVE_APPROVAL			=> SQ_STATUS_LIVE_APPROVAL,
										SQ_STATUS_EDITING				=> SQ_STATUS_EDITING_APPROVAL,
										SQ_STATUS_EDITING_APPROVAL		=> SQ_STATUS_EDITING_APPROVAL,
										SQ_STATUS_EDITING_APPROVED		=> NULL,
									  );

					if (count($schemas) === 0) {
						// If no workflow, then we're pretty much stuck
						// since sending no-workflow assets to Live on the basis
						// of a null change of status seems incorrect
						$status_changes = Array();

					} else if ($wfm->testPublish($assetid, $GLOBALS['SQ_SYSTEM']->currentUserId()) === TRUE) {
						// We would complete this workflow on the next time around

						// Valid changes
						$status_changes[SQ_STATUS_UNDER_CONSTRUCTION] = SQ_STATUS_APPROVED;
						$status_changes[SQ_STATUS_PENDING_APPROVAL]   = SQ_STATUS_APPROVED;
						$status_changes[SQ_STATUS_EDITING]            = SQ_STATUS_EDITING_APPROVED;
						$status_changes[SQ_STATUS_EDITING_APPROVAL]   = SQ_STATUS_EDITING_APPROVED;

						// This is still an error condition...
						$status_changes[SQ_STATUS_ARCHIVED]           = SQ_STATUS_APPROVED;

					}//end if workflow

				} else if ($new_status === SQ_STATUS_APPROVED) {
					// "Approving" => "Approved" means we believe all bundled
					// assets will finish their workflow this time around.

					$status_changes = Array(
										// Archived => Approved is a known error
										SQ_STATUS_ARCHIVED				=> SQ_STATUS_APPROVED,
										SQ_STATUS_UNDER_CONSTRUCTION	=> SQ_STATUS_APPROVED,
										SQ_STATUS_PENDING_APPROVAL		=> SQ_STATUS_APPROVED,
										SQ_STATUS_APPROVED				=> NULL,
										SQ_STATUS_LIVE					=> NULL,
										SQ_STATUS_LIVE_APPROVAL			=> SQ_STATUS_LIVE,
										SQ_STATUS_EDITING				=> SQ_STATUS_EDITING_APPROVED,
										SQ_STATUS_EDITING_APPROVAL		=> SQ_STATUS_EDITING_APPROVED,
										SQ_STATUS_EDITING_APPROVED		=> NULL,
									  );

					if (count($schemas) === 0) {
						// If no workflow, then just push U/C assets to Live
						$status_changes[SQ_STATUS_UNDER_CONSTRUCTION] = SQ_STATUS_LIVE;
					}

				} else if ($new_status === SQ_STATUS_UNDER_CONSTRUCTION) {

					//Bug Fix #4829: we want to reject all assets in the bundle regardless of permission to
					$this->_running_vars['no_status_integrity'][$assetid] = TRUE;

					// Reject changes, revert from "Approving" to "Editing":
					// - Pre-live workflowed assets go back to Under Construction.
					// - Post-live workflowed assets go back to Safe Editing.
					// Live/Archived/UC have no change, as there are no changes to reject.
					// Safe Editing will not be reverted to Live; keep it in edit mode
					$status_changes = Array(
										SQ_STATUS_ARCHIVED				=> NULL,
										SQ_STATUS_UNDER_CONSTRUCTION	=> NULL,
										SQ_STATUS_PENDING_APPROVAL		=> SQ_STATUS_UNDER_CONSTRUCTION,
										SQ_STATUS_APPROVED				=> SQ_STATUS_UNDER_CONSTRUCTION,
										SQ_STATUS_LIVE					=> NULL,
										SQ_STATUS_LIVE_APPROVAL			=> SQ_STATUS_EDITING,
										SQ_STATUS_EDITING				=> NULL,
										SQ_STATUS_EDITING_APPROVAL		=> SQ_STATUS_EDITING,
										SQ_STATUS_EDITING_APPROVED		=> SQ_STATUS_EDITING,
									  );

				}//end if status

			break;

			case SQ_STATUS_APPROVED:
				if ($new_status === SQ_STATUS_LIVE) {
					// "Approved" => "Live"; make live, regardless the consequences
					// (except: Safe Editing status goes to Safe Edit Approved since
					// going straight to Live is actually going backwards)
					$status_changes = Array(
										SQ_STATUS_ARCHIVED				=> NULL,
										SQ_STATUS_UNDER_CONSTRUCTION	=> SQ_STATUS_LIVE,
										SQ_STATUS_PENDING_APPROVAL		=> SQ_STATUS_LIVE,
										SQ_STATUS_APPROVED				=> SQ_STATUS_LIVE,
										SQ_STATUS_LIVE					=> NULL,
										SQ_STATUS_LIVE_APPROVAL			=> SQ_STATUS_LIVE,
										SQ_STATUS_EDITING				=> SQ_STATUS_EDITING_APPROVED,
										SQ_STATUS_EDITING_APPROVAL		=> SQ_STATUS_EDITING_APPROVED,
										SQ_STATUS_EDITING_APPROVED		=> SQ_STATUS_LIVE,
									  );

				} else if ($new_status === SQ_STATUS_UNDER_CONSTRUCTION) {
					// Reject Changes; return to Editing. Whoops.
					$status_changes = Array(
										SQ_STATUS_ARCHIVED				=> NULL,
										SQ_STATUS_UNDER_CONSTRUCTION	=> NULL,
										SQ_STATUS_PENDING_APPROVAL		=> SQ_STATUS_UNDER_CONSTRUCTION,
										SQ_STATUS_APPROVED				=> SQ_STATUS_UNDER_CONSTRUCTION,
										SQ_STATUS_LIVE					=> NULL,
										SQ_STATUS_LIVE_APPROVAL			=> SQ_STATUS_EDITING,
										SQ_STATUS_EDITING				=> NULL,
										SQ_STATUS_EDITING_APPROVAL		=> SQ_STATUS_EDITING,
										SQ_STATUS_EDITING_APPROVED		=> SQ_STATUS_EDITING,
									  );

				}//end if status

			break;

			case SQ_STATUS_LIVE:
				if ($new_status === SQ_STATUS_UNDER_CONSTRUCTION) {
					// (re-)Start Editing - send those Live into Safe Edit,
					// leave Safe Edit assets where they are, move others into
					// U/C
					$status_changes = Array(
										SQ_STATUS_ARCHIVED				=> SQ_STATUS_UNDER_CONSTRUCTION,
										SQ_STATUS_UNDER_CONSTRUCTION	=> NULL,
										SQ_STATUS_PENDING_APPROVAL		=> NULL,
										SQ_STATUS_APPROVED				=> NULL,
										SQ_STATUS_LIVE					=> SQ_STATUS_EDITING,
										SQ_STATUS_LIVE_APPROVAL			=> NULL,
										SQ_STATUS_EDITING				=> NULL,
										SQ_STATUS_EDITING_APPROVAL		=> NULL,
										SQ_STATUS_EDITING_APPROVED		=> NULL,
									  );

				} else if ($new_status === SQ_STATUS_LIVE) {
					// Reinforcing "Editing" status
					$status_changes = Array(
										// Don't allow any backward changes, but otherwise
										// get some assets up to speed
										SQ_STATUS_ARCHIVED				=> NULL,
										SQ_STATUS_UNDER_CONSTRUCTION	=> NULL,
										SQ_STATUS_PENDING_APPROVAL		=> NULL,
										SQ_STATUS_APPROVED				=> SQ_STATUS_LIVE,
										SQ_STATUS_LIVE					=> NULL,
										SQ_STATUS_LIVE_APPROVAL			=> NULL,
										SQ_STATUS_EDITING				=> NULL,
										SQ_STATUS_EDITING_APPROVAL		=> NULL,
										SQ_STATUS_EDITING_APPROVED		=> SQ_STATUS_LIVE,
									  );

				}//end if new status

			break;

		}//end switch old status

		return array_get_index($status_changes, $asset_status, NULL);

	}//end _getNewAssetStatus()


	/**
	* Utility function to filter "invitation" and other undesirable messages
	*
	* By "undesirable", in this case this means asset-level messages that we'd
	* rather have the bundle send. We save details about those messages in
	* running vars so we can send asset info in the bundle message.
	*
	* Modifies a running var storing the users and assets that they would have
	* been sent messages about, and removes them from the job messages
	* argument, which is passed back to the caller.
	*
	* @param string	$assetid		The ID of the bundled asset generating these messages
	* @param array	&$job_messages	The messages generated by that asset's Edit Status job
	* @param array	$filters		Filter of 'name' => 'message type code'
	*
	* @return void
	*/
	protected function _filterMessages($assetid, Array &$job_messages, $filters)
	{
		if (count($filters) === 0) {
			return;
		}

		foreach ($job_messages as $msg_key => $message) {
			// If this is an invite message, then we may be interested
			// in adding this asset to the bundle list (since workflow
			// has progressed).
			foreach ($filters as $filter_name => $filter_msg_code) {
				if ($message->type === $filter_msg_code) {
					$msg_assetid = $message->parameters['assetid'];

					// Only interested if this is the one for the asset
					// itself, and not any dependants (there are no other
					// children we have to worry about here)
					if ((string)$msg_assetid === (string)$assetid) {
						foreach ($message->to as $to_user) {
							$this->_running_vars['msg_filters'][$filter_name][$to_user][] = $msg_assetid;

						}//end foreach to user in message

					}//end if not a dependant asset

					// Remove the message from the job list, so we just have the log
					// and approval messages left over.
					unset($job_messages[$msg_key]);
					break;


				}//end if invitation message
			}

		}//end foreach message

	}//end _filterInviteMessages()


	/**
	* Return an array of the messages we should be filtering and bundling
	* (either 'start', 'progress', 'approve' or 'invite').
	*
	* @param int	$old_status	The old bundle status
	* @param int	$new_status	The new bundle status
	*
	* @return array
	*/
	protected function _getRequiredBundleMessages($old_status, $new_status)
	{
		$msg_types = Array();

		// Determine the message(s) we need to send.
		switch ($old_status) {
			case SQ_STATUS_UNDER_CONSTRUCTION:
				switch ($new_status) {
					case SQ_STATUS_PENDING_APPROVAL:
						// Catch any possible approve messages that may appear
						// when the workflow may start (if the starting user can
						// also approve things)
						$msg_types = Array('start', 'approve', 'auto_approve_progress');
					break;

				}//end switch new status
			break;

			case SQ_STATUS_PENDING_APPROVAL:
				switch ($new_status) {
					case SQ_STATUS_UNDER_CONSTRUCTION:
						$msg_types = Array('reject');
					break;

					case SQ_STATUS_PENDING_APPROVAL:
						$msg_types = Array('approve', 'progress');
					break;

					case SQ_STATUS_APPROVED:
						$msg_types = Array('approve');
					break;

				}//end switch new status
			break;

			case SQ_STATUS_APPROVED:
				switch ($new_status) {
					case SQ_STATUS_UNDER_CONSTRUCTION:
						$msg_types = Array('reject');
					break;

				}//end switch new status
			break;

		}//end switch old status

		return $msg_types;

	}//end _getRequiredBundleMessages()


	/**
	* Returns a URL for the specified asset's passed screen
	*
	* @param string	$assetid		The specified asset ID
	* @param string	$screen_name	The screen we want the URL of
	*
	* @return string
	*/
	protected function _getBackendScreenKeywordReplacement($assetid, $screen_name)
	{
		$replacement = '';
		$hrefs = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($assetid => $screen_name), FALSE);

		if (empty($hrefs) === FALSE) {
			$href = str_replace('./', '', current($hrefs));

			$current_url = sq_web_path('root_url');
			if ($current_url === '') {
				$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
				$current_url = 'http://'.current($root_urls);
			}

			if (SQ_IN_LIMBO === TRUE) {
				$replacement  = $current_url;
				$replacement .= (strpos($href, SQ_CONF_LIMBO_SUFFIX) !== FALSE) ? '/'.$href : '/'.SQ_CONF_LIMBO_SUFFIX.$href;
			} else {
				$replacement  = $current_url;
				$replacement .= (strpos($href, SQ_CONF_BACKEND_SUFFIX) !== FALSE) ? '/'.$href : '/'.SQ_CONF_BACKEND_SUFFIX.$href;
			}
		}

		return $replacement;

	}//end _getBackendScreenKeywordReplacement()


	/**
	* Encapsulate the sending of user log messages for each asset
	*
	* @param string	$assetid		The asset ID being logged
	* @param string	$message_body	The user log message to send
	*
	*/
	protected function _sendUserLogMessage($assetid, $message_body)
	{
		$version = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid), 'asset', FALSE, 'version');
		$version = $version[$assetid];

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$msg_reps = Array(
						'message'	=> $message_body,
					);
		$message = $ms->newMessage(Array(), 'asset.workflow.userlog', $msg_reps);
		$message->parameters['assetid'] = $assetid;
		$message->parameters['version'] = substr($version, 0, strrpos($version, '.'));
		$message->send();

		return TRUE;

	}//end _sendUserLogMessage()


}//end class
?>
