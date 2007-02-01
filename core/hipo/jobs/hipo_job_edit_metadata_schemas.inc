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
* $Id: hipo_job_edit_metadata_schemas.inc,v 1.38.2.1 2007/02/01 23:32:00 skim Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit metadata schemas
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.38.2.1 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Edit_Metadata_Schemas extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Metadata_Schemas($code_name='')
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
	function paintConfig(&$o, $class)
	{
		$write_access = $this->writeAccess();

		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('edit_metadata_threshold'));

		if ($write_access) {
			text_box($class.'[SQ_HIPO_METADATA_THRESHOLD]', SQ_HIPO_METADATA_THRESHOLD, 5);
			echo ' '.translate('assets');
			$o->note(translate('hipo_value_divided_by_schema'));
		} else {
			echo SQ_HIPO_METADATA_THRESHOLD;
			echo ' '.translate('assets');
		}

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return Array
	* @access public
	*/
	function getConfigVars()
	{
		return Array(
				'SQ_HIPO_METADATA_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 1),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Edit Metadata Schema HIPO appends the ID of the asset that the schemas are being set at
	* (the top most parent) so that any other schema jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.md5(implode('-',array_keys($this->_running_vars['assets'])));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_edit_metadata_schemas');

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
					'name'			=> translate('hipo_acquiring_metadata_locks'),
					'hipo_job'		=> Array(
										'init_details_function'	=> 'getLockAssetDetails',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_updating_metadata_schemas'),
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
				Array(
					'name'			=> translate('hipo_releasing_metadata_locks'),
					'function_call'	=> Array(
										'process_function'	=> 'releaseLocks',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> false,
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
	* in freestyle mode in its current state
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_METADATA_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		return ((count($this->_running_vars['todo_assetids']) / (SQ_HIPO_METADATA_THRESHOLD / count($this->_running_vars['schema_changes']))) * 100);

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
			$id = key($this->_running_vars['todo_assetids']);
			$result = $GLOBALS['SQ_SYSTEM']->am->acquireLock($id, 'metadata');
			if ($result) {
				if (!$this->processSchemas($this->_steps[1], get_class($this))) {
					return FALSE;
				}
				if ($result == 1) {
					$result = $GLOBALS['SQ_SYSTEM']->am->releaseLock($id, 'metadata');
				}
			} else {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start editing schemas, we need to have an asset id to start editing schemas at
		if (empty($this->_running_vars['assets'])) {
			trigger_localised_error('HIPO0017', E_USER_WARNING);
			return false;
		}

		$this->_running_vars['done_assetids'] = Array();
		if (empty($this->_running_vars['schema_changes'])) {
			// no schema changes, so do nothing
			$this->_running_vars['todo_assetids'] = Array();
		} else {
			$assets = $this->_running_vars['assets'];
			$child_assets = Array();
			for (reset($assets); null !== ($assetid = key($assets)); next($assets)) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) {
					trigger_localised_error('HIPO0016', E_USER_WARNING, $assetid);
					return '';
				}
				$child_assets += $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);
				$child_assets[$assetid] = $asset->type();
				unset($asset);
			}
			$this->_running_vars['todo_assetids'] = $child_assets;
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Edit schemas for a single asset in the to-do list
	*
	* @param array		&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processSchemas(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift(array_keys($this->_running_vars['todo_assetids']));
			$asset_type = $this->_running_vars['todo_assetids'][$assetid];
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);
			$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

			if (!is_null($asset)) {
				// if this asset does not allow the user to set metadata values then
				// I think its okay to assume that we dont need to set any schemas
				// because nothing is ever going to get filled in - thus required
				// fields with defaults will always be blank
				if ($mm->allowsMetadata($asset->id)) {

					foreach ($this->_running_vars['schema_changes'] as $set_data) {
						if ($set_data['schemaid']) {
							$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($set_data['schemaid']);
							if (is_null($schema)) {
								$schema_name = 'Unknown Schema';
							} else {
								$schema_name = $schema->name;
							}
						} else {
							continue;
						}

						$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

						if ($set_data['previous_access'] === null) {
							// to edit this schema, there must not currently be a schema set
							// at all - either grant OR deny - for this asset
							$schemas = $mm->getSchemas($asset->id, $set_data['granted']);
							if (isset($schemas[$set_data['schemaid']])) {
								$new_access     = ($set_data['granted']) ? translate('apply') : translate('deny');
								$current_access = ($schemas[$set_data['schemaid']]) ? translate('applied') : translate('denied');
								$this->_addError(translate('hipo_cannot_modify_schema', $new_access, $schema_name, $asset->name, $current_access), true);
								continue;
							}
						} else {
							// we need to have a schema set and the access level be the same
							$schemas = $mm->getSchemas($asset->id);
							if (!isset($schemas[$set_data['schemaid']])) {
								continue;
							} else if ($schemas[$set_data['schemaid']] != $set_data['previous_access']) {
								$new_access     = ($set_data['previous_access']) ? translate('revoke')  : translate('apply');
								$current_access = ($set_data['previous_access']) ? translate('applied') : translate('denied');
								$this->_addError(translate('hipo_cannot_modify_schema', $new_access, $schema_name, $asset->name, $current_access), true);
								continue;
							}
						}

						switch ($set_data['granted']) {
							case -1 :
								// delete a schema
								$mm->deleteSchema($asset->id, $set_data['schemaid']);
							break;

							case 0  :
							case 1  :
								// apply a schema
								$mm->setSchema($asset->id, $set_data['schemaid'], $set_data['granted']);
							break;
						}
					}

					$step_data['message'] = translate('hipo_editing_metadata_schema', $asset->name);

				} else {
					$step_data['message'] = translate('hipo_skipping_metadata_schema', $asset->name);

				}//end if metadata screen exists


				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				$this->_addError(translate('hipo_cannot_edit_schema', $assetid));
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
			unset($this->_running_vars['done_assetids']);
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processSchemas()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string		&$job_type
	* @param array		&$running_vars
	* @param array		&$options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';

		$options['auto_complete'] = true;

		$running_vars = Array(
							'assetids'			=> array_keys($this->_running_vars['assets']),
							'lock_type'			=> Array('metadata'),
							'forceably_acquire'	=> false,
							'dependants_only'	=> false,
						);

	}//end getLockAssetDetails()


	/**
	* Release the locks we acquired while setting the schemas
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function releaseLocks(&$step_data, $prefix)
	{
		if (!isset($this->_running_vars['re_lock_assets'])) {
			$this->_running_vars['re_lock_assets'] = $this->_running_vars['assets'];
		}

		$assetid = key($this->_running_vars['re_lock_assets']);
		unset($this->_running_vars['re_lock_assets'][$assetid]);

		$children = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);

		foreach ($children as $dep_child_id => $dep_child_type_code) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($dep_child_id, 'metadata');
		}

		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($assetid));

		$step_data['message'] = translate('hipo_release_locks', $asset_info[$assetid]['name']).translate('hipo_release_locks_dependents');

		$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'all');

		$this->_running_vars['done_assetids'][] = $assetid;

		if (empty($this->_running_vars['re_lock_assets'])) {
			unset($this->_running_vars['done_assetids']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end releaseLocks()


}//end class

?>
