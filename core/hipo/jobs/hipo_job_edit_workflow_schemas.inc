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
* $Id: hipo_job_edit_workflow_schemas.inc,v 1.49 2012/08/30 01:09:18 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit workflow schemas
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.49 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Edit_Workflow_Schemas extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Workflow_Schemas($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param string						$class		calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('Workflow Schemas Threshold'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_WORKFLOW_SCHEMAS_THRESHOLD]', SQ_HIPO_WORKFLOW_SCHEMAS_THRESHOLD, 4);
		} else {
			echo SQ_HIPO_WORKFLOW_SCHEMAS_THRESHOLD;
		}
		echo ' '.translate('assets');

		$o->note(translate('Value will be divided by number of affected schemas.'));

		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return Array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_WORKFLOW_SCHEMAS_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 20),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Edit Workflow Schema HIPO appends the ID of the asset that the schemas are being set at
	* (the top most parent) so that any other schema jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.md5(serialize($this->_running_vars['schema_changes']));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Edit Workflow Schemas');


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
					'name'			=> translate('Updating Workflow Schemas'),

					'function_call'	=> Array(
										'process_function'	=> 'processSchemas',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run
	* in freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_WORKFLOW_SCHEMAS_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		return ($this->_running_vars['total_changes'] / SQ_HIPO_WORKFLOW_SCHEMAS_THRESHOLD) * 100;

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_assetids'])) {
			if (!$this->processSchemas($this->_steps[0], get_class($this))) {
				return FALSE;
			}
		}
		return TRUE;

	}


	/**
	* Prepares the running vars and gets the children of the asset in question
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start editing schemas, we need to have an asset id to start editing schemas at
		if (empty($this->_running_vars['schema_changes'])) {
			trigger_localised_error('HIPO0010', translate('Unable to initialise Edit Workflow Schemas HIPO, no asset IDs specified'), E_USER_WARNING);
			return false;
		}

		$this->_running_vars['done_changes']  = 0;
		$this->_running_vars['total_changes'] = 0;

		if (empty($this->_running_vars['schema_changes'])) {
			// no schema changes, so do nothing
			$this->_running_vars['todo_assetids'] = Array();
		} else {
			$this->_running_vars['todo_assetids'] = Array();
			foreach ($this->_running_vars['schema_changes'] as $schema_change) {
				$assetids = $schema_change['assetids'];
				foreach ($assetids as $assetid) {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

					if (is_null($asset)) {
						trigger_localised_error('HIPO0009', sprintf(translate('Unable to initialise Edit Workflow Schemas HIPO, invalid asset ID specified (%s)'), $assetid), E_USER_WARNING);
						return '';
					}

					if ((!array_get_index($schema_change, 'cascades')) || array_get_index($schema_change, 'dependants_only')) {
						$child_assets = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);
					} else {
						$child_assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);
					}

					// add the parent asset to the list too
					$child_assets[$assetid]    = Array (
												0    => Array (
														'type_code'    => $asset->type(),
													   ),
											  );

					$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
					// Now assign schema changes to each asset
					foreach ($child_assets as $child_assetid => $child_asset) {
						if (!$wfm->allowsWorkflow($child_assetid)) {
							continue;
						}//end if
						$type_code = $child_asset[0]['type_code'];

						$this_todo =& $this->_running_vars['todo_assetids'][$child_assetid];
						$this_todo['type_code'] = $type_code;
						$this_todo['schema_changes'][] = $schema_change;
						$this->_running_vars['total_changes']++;
					}

					unset($asset);
				}

			}
		}

		return parent::prepare();

	}//end prepare()


   /**
	* Edit schemas for a single asset in the to-do list
	*
	* @param array        &$step_data    a reference to the array of information about the current step
	* @param string        $prefix        prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processSchemas(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			reset($this->_running_vars['todo_assetids']);
			$assetid = key($this->_running_vars['todo_assetids']);
			$asset_type = $this->_running_vars['todo_assetids'][$assetid]['type_code'];
			$schema_changes = $this->_running_vars['todo_assetids'][$assetid]['schema_changes'];
			unset($this->_running_vars['todo_assetids'][$assetid]);

			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset)) {
				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'workflow')) {
					foreach ($schema_changes as $set_data) {
						$this->_running_vars['done_changes']++;

						if ($set_data['schemaid']) {
							$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($set_data['schemaid']);
							if (is_null($schema)) {
								$schema_name = 'Unknown Schema';
							} else {
								$schema_name = $schema->name;
							}
						} else {
							continue;
						}

						$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

						if ($set_data['previous_access'] === null) {
							// to edit this schema, there must not currently be a schema set
							// at all - either grant OR deny - for this asset
							$schemas = $wfm->getSchemas($asset->id, $set_data['granted']);
							if (isset($schemas[$set_data['schemaid']])) {
								$new_access     = ($set_data['granted']) ? translate('Apply') : translate('Deny');


								$current_access = ($schemas[$set_data['schemaid']]) ? translate('Applied') : translate('Denied');


								$this->_addError(sprintf(translate('Could not %1$s schema "%2$s" on "%3$s" because this asset already has this schema %4$s. The current schema setting for "%3$s" has been retained'), $new_access, $schema_name, $asset->name, $current_access), true);

								continue;
							}
						} else {
							// we need to have a schema set and the access level be the same
							$schemas = $wfm->getSchemas($asset->id);

							if (!isset($schemas[$set_data['schemaid']])) {
								continue;
							} else if ($schemas[$set_data['schemaid']] != $set_data['previous_access']) {
								$new_access     = ($set_data['previous_access']) ? translate('Revoke')  : translate('Apply');


								$current_access = ($set_data['previous_access']) ? translate('Applied') : translate('Denied');


								$this->_addError(sprintf(translate('Could not %1$s schema "%2$s" on "%3$s" because this asset already has this schema %4$s. The current schema setting for "%3$s" has been retained'), $new_access, $schema_name, $asset->name, $current_access), true);

								continue;
							}
						}

						switch ($set_data['granted']) {
							case -1 :
								// delete a schema
								$step_data['message'] = htmlentities('Deleting "'.$schema->name.'" (#'.$schema->id.') from "'.$asset->name.'" (#'.$asset->id.')', ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
								$wfm->deleteSchema($asset->id, $set_data['schemaid']);
							break;

							case 0  :
							case 1  :
								// deny a schema
								// apply a permission
								$step_data['message'] = htmlentities(($set_data['granted'] ? 'Apply' : 'Deny').'ing "'.$schema->name.'" (#'.$schema->id.') to "'.$asset->name.'" (#'.$asset->id.')', ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
								$wfm->setSchema($asset->id, $set_data['schemaid'], $set_data['granted'], $set_data['cascades']);
							break;
						}
					}

					// If there are multiple schema changes to make, say we are
					// (otherwise use more specific message set above)
					if (count($schema_changes) > 1) {
						$step_data['message'] = htmlentities('Making '.count($schema_changes).' schema changes to '.$asset->name.' (Id: #'.$asset->id.')', ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
					} else if (count($schema_changes) == 0) {
						$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

					}

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'workflow');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				} else {
					// we do not acquire lock
					trigger_localised_error('SYS0100', sprintf(translate('Unable to acquire "%1$s" lock on "%2$s", %3$s', 'workflow'), $asset->name, ' skipping asset.'), E_USER_WARNING);
				}

			} else {
				$step_data['message'] = sprintf(translate('Skipping asset #%s'), $assetid);

				$this->_addError(sprintf(translate('Could not edit schemas for asset #%s, asset does not exist'), $assetid));

			}
		}

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = ($this->_running_vars['done_changes'] / $this->_running_vars['total_changes']) * 100;
			$step_data['complete']     = FALSE;
		}

		return true;

	}//end processSchemas()


}//end class

?>
