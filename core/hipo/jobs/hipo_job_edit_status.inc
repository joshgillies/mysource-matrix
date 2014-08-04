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
* $Id: hipo_job_edit_status.inc,v 1.74 2013/04/12 04:03:20 cupreti Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to change the status of an asset
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.74 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Edit_Status extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename for the HIPO
	*/
	function HIPO_Job_Edit_Status($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('edit_status_threshold'));

		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_EDIT_STATUS_THRESHOLD]', SQ_HIPO_EDIT_STATUS_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_EDIT_STATUS_THRESHOLD;
		}
		echo ' '.translate('assets');
		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_EDIT_STATUS_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 1),
			   );

	}//end getConfigVars()


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
		$running_dependants = '';
		if (!empty($this->_running_vars['process_dependants'])) {
			$running_dependants = 'dependants';
		}
		return parent::getCodeName().$this->_running_vars['assetid'].$running_dependants;

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_edit_status');

	}//end getHipoName()


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
				Array(
					'name'			=> translate('hipo_editing_status'),
					'function_call'	=> Array(
										'process_function'	=> 'processStatus',
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
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_EDIT_STATUS_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}

		return (((count($this->_running_vars['todo_assetids']) + 1) / SQ_HIPO_EDIT_STATUS_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{

		// to start editing a status, we need to have an asset id to edit
		if (!isset($this->_running_vars['assetid'])) {
			trigger_localised_error('HIPO0012', E_USER_WARNING);
			return '';
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_localised_error('HIPO0011', E_USER_WARNING, $this->_running_vars['assetid']);
			return '';
		}

		if (!empty($this->_running_vars['process_dependants'])) {
			// we are _only_ going to do the dependant assets, no independant ones at all. this should only ever
			// happen from spawned jobs
			$this->_running_vars['todo_assetids'] = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($asset->id);
			$this->_running_vars['done_root'] = TRUE;
		} else {
			// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
			// we will be edting the status of

			// if there is no dependants setting, or if it is true, get only dependants
			if (!isset($this->_running_vars['dependants_only']) || !empty($this->_running_vars['dependants_only'])) {
				$this->_running_vars['dependants_only'] = FALSE;
				$child_assets = Array();
			} else {
				// get all children
				$this->_running_vars['dependants_only'] = TRUE;
				$child_assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, '', TRUE, FALSE);
			}
			// exclude shadow children
			foreach ($child_assets as $id => $asset) {
				$id_parts = explode(':', $id);
				if(isset($id_parts[1])) {
					unset($child_assets[$id]);
				}
			}

			$this->_running_vars['todo_assetids'] = $child_assets;
			$this->_running_vars['done_root']     = FALSE;
		}
		$this->_running_vars['done_assetids'] = Array();

		// internal messages being stored
		if (!isset($this->_running_vars['messages']) || empty($this->_running_vars['messages'])) {
			$this->_running_vars['messages']     = Array();
		}

		unset($asset);

		$to_do = array_keys($this->_running_vars['todo_assetids']);
		if (empty($this->_running_vars['process_dependants'])) {
			$to_do[] = $this->_running_vars['assetid'];
		}
		$this->_running_vars['current_asset_statuses'] = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($to_do, '', TRUE, 'status');

		return parent::prepare();

	}//end prepare()


	/**
	* Load the HIPO vars from the database
	*
	* @param string	$code_name	a unique code_name the HIPO
	*
	* @return boolean
	* @access public
	*/
	function load($code_name)
	{
		// need this because it'll complain about Internal Message being an incomplete class
		require_once SQ_INCLUDE_PATH.'/internal_message.inc';
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		return parent::load($code_name);

	}//end load()


	/**
	* Paint information about the process of the HIPO
	*
	* This function calls the paint function for the step currently being run and also outputs
	* a progress bar to indicate to the user where they are in the HIPO process
	*
	* @param object		&$o		Backend_Outputter, reference to the backend outputter
	* @param boolean	$nested	true if the steps are nested
	*
	* @return void
	* @access public
	*/
	function paint(&$o, $nested=FALSE)
	{
		parent::paint($o, $nested);

		// paint dependant hipo job here
		if (!empty($this->_running_vars['dependant_hipo_job'])) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hipo = $hh->getJob($this->_running_vars['dependant_hipo_job']);
			if (!is_null($hipo)) {
				$hipo->paint($o, TRUE);
			} else {
				unset($this->_running_vars['dependant_hipo_job']);
			}
		}

	}//end paint()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids']) || !$this->_running_vars['done_root'] || !empty($this->_running_vars['dependant_hipo_job'])) {
			if (!$this->processStatus($this->_steps[0], get_class($this), TRUE)) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Process the current step of a HIPO in a server environment
	*
	* This function manages the message queue during the status changes
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processServer(&$step_data, $prefix)
	{
		// we are probably going to be sending some internal messages during this
		// process, so we'll open a new queue here
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openQueue();

		if (!parent::processServer($step_data, $prefix)) {
			$ms->abortQueue();
			return FALSE;
		}

		// close the internal message queue and send the messages that
		// were generated during this status change
		$ms->closeQueue();

		return TRUE;

	}//end processServer()


	/**
	* Edit the status of a single asset in the to-do list
	*
	* @param array		&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	* @param boolean	$freestyle	if this function is being freestyled or not
	*
	* @return boolean
	* @access public
	*/
	function processStatus(&$step_data, $prefix, $freestyle=FALSE)
	{
		$send_messages = array_get_index($this->_running_vars, 'send_messages', TRUE);

		// process dependant hipo_job_edit_status if necessary
		if (!empty($this->_running_vars['dependant_hipo_job'])) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hipo = $hh->getJob($this->_running_vars['dependant_hipo_job']);

			// bail if we've completed this job
			if (is_null($hipo)) return FALSE;

			if ($freestyle && $hipo->freestyle()) {
				// we're freestylin' and that it, done
				unset($this->_running_vars['dependant_hipo_job']);

				// Add the messages saved from the sub-job into our list of messages
				$this->_running_vars['messages'] = array_get_index($hipo->_running_vars, 'messages', Array());

				$hipo->save();
				$hh->uncacheJob($hipo->code_name);
			} else if ($hipo->process() && $hipo->complete()) {
				// process the sub hipo on each refresh
				$running_vars = $hipo->getRunningVars();
				if (isset($running_vars['assetid'])) {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($running_vars['assetid']);
					$asset->_updated(FALSE);
				}

				// Add the messages saved from the sub-job into our list of messages
				$this->_running_vars['messages'] = array_get_index($hipo->_running_vars, 'messages', Array());

				unset($this->_running_vars['dependant_hipo_job']);

			}

			// HIPO job completed, and there are no assets left to process
			// => we are done
			if ((empty($this->_running_vars['dependant_hipo_job']) === TRUE) && (empty($this->_running_vars['todo_assetids']) === TRUE)) {
				$step_data['percent_done'] = 100;
				$step_data['complete'] = TRUE;
			}

			return TRUE;

		}
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

		// create a queue to store the messages
		$ms->openQueue();


		if (!empty($this->_running_vars['todo_assetids']) || !$this->_running_vars['done_root']) {
			if (empty($this->_running_vars['done_assetids']) && !$this->_running_vars['done_root']) {

				// make sure we process the root asset first
				$assetid = $this->_running_vars['assetid'];
				$this->_running_vars['done_root'] = TRUE;

			} else {
				$tmp_keys = array_keys($this->_running_vars['todo_assetids']);
				$assetid = $tmp_keys[0];
				unset($this->_running_vars['todo_assetids'][$assetid]);
			}
			$new_status = $this->_running_vars['new_status'];
			// if the current status of the status is the same as the new status
			// if its same there is nothing to be done. otherwise processStatusChange on the asset
			// one exception is that current asset is in a workflow, the status will always be pending approval
			$current_status = array_get_index($this->_running_vars['current_asset_statuses'], $assetid, 0);
			if (($current_status != $new_status) || ($current_status & SQ_SC_STATUS_PENDING)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!is_null($asset)) {
					// if an asset has dependants, we don't want to run _updated() on it until
					// after we have processed them. _updated is run after the execution of the
					// sub-sub-hipo thingy has completed
					$dependants = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($asset->id);

					// If we are moving into a "pending approval" state and we have been sent
					// a workflow stream name, then set that as the stream to use before changing status
					if ((($current_status & (SQ_SC_STATUS_CAN_APPROVE | SQ_STATUS_LIVE)) > 0) && (($new_status & SQ_SC_STATUS_PENDING) > 0)) {
						if (array_get_index($this->_running_vars, 'workflow_stream') !== NULL) {
							$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
							$wfm->setStartingStream($assetid, $this->_running_vars['workflow_stream']);
						}
					}
					if (empty($dependants) || !empty($this->_running_vars['process_dependants'])) {
						$asset->processStatusChange($new_status, FALSE);
					} else {
						$asset->processStatusChange($new_status, FALSE, FALSE);
					}
					$step_data['message'] = translate('hipo_edit_status', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), $asset->id);

					// this asset has dependants, so we are going to spawn another edit_status hipo
					// to take care of processing them
					if (!empty($dependants) && empty($this->_running_vars['process_dependants'])) {
						$messages = array_merge($this->_running_vars['messages'], $ms->queueContents());
						if ($code_name = $this->spawnDependantStatusHipo($asset->id, $new_status, $messages)) {
							$this->_running_vars['dependant_hipo_job'] = $code_name;
						}
					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				} else {
					$step_data['message'] = translate('hipo_skipping_asset', $assetid);
					$this->_addError(translate('hipo_cannot_edit_status', $assetid));
				}
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
			}
			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}//end if empty done ids and not done root

		// (FB'08: "Workflow email not sent for dependant asset if it is not linked".)
		// If a dependant asset is not linked elsewhere than where it is now,
		// assume that its workflow messages would be duplicated by the parent,
		// and not send it. Otherwise, add it to the queue.
		if(empty($this->_running_vars['process_dependants']) && isset($assetid)) {
			$this->_running_vars['messages'] = array_merge($this->_running_vars['messages'], $ms->queueContents());
		} else {
			foreach($ms->queueContents() as $internal_message) {
				$asset_id = $internal_message->parameters['assetid'];
				$parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($asset_id,'',TRUE,FALSE);
				if(count($parents) > 1 ) {
					$this->_running_vars['messages'][] = $internal_message;
				}
			}
		}

		// We've plucked out the messages we want, now dump the rest
		$ms->abortQueue();

		if (empty($this->_running_vars['todo_assetids'])) {
			// We only send or messages if we are not a dependant job, and
			// we are not asked to not send our messages (send_messages === FALSE)
			if (empty($this->_running_vars['process_dependants']) && ($send_messages === TRUE) && isset($assetid)) {
				// send workflow email at this point
				// there might still be a dependant job waiting to be processed
				$ms->send($this->_running_vars['messages']);
			}

			if (empty($this->_running_vars['dependant_hipo_job'])) {
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
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processStatus()


	/**
	* Starts another Hipo_Job_Edit_Status to process dependants of given asset
	*
	* @param string	$assetid	The ID of the asset to process dependants of
	* @param int	$new_status	The status to change dependants to
	* @param array	$messages	The current message queue
	*
	* @return string
	* @access private
	*/
	function spawnDependantStatusHipo($assetid, $new_status, $messages)
	{
		$init_hipo = new Hipo_Job_Edit_Status();

		$options['auto_complete'] = TRUE;

		// we pass the asset id of the 'umbrella' for the dependant assets
		// and the 'process_dependants' flag. the sub-sub-hipo thingy will
		// take care of working out what assets (depdants) to process
		$running_vars = Array(
							'assetid'				=> $assetid,
							'new_status'			=> $new_status,
							'process_dependants'	=> TRUE,
							'messages'				=> $messages,
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

	}//end spawnDependantStatusHipo()


}//end class

?>
