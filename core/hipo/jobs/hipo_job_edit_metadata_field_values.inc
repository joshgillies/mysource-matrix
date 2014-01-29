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
* $Id: hipo_job_edit_metadata_field_values.inc,v 1.6 2012/08/30 01:09:18 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to edit Metadata Field values
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Edit_Metadata_Field_Values extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Edit_Metadata_Field_Values($code_name='')
	{
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
		// hipo_job_edit_metadata_schemas.inc will handle this instead, so just do nothing
		return;
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
		return translate('hipo_name_edit_metadata_field_values');

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
					'name'			=> translate('hipo_updating_metadata_field_values'),
					'function_call'	=> Array(
										'process_function'	=> 'processFieldValues',
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
		return ((count($this->_running_vars['todo_assetids']) / (SQ_HIPO_METADATA_THRESHOLD / count($this->_running_vars['metadata_field_value_changes']))) * 100);

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
			if (!$this->processFieldValues($this->_steps[0], get_class($this))) {
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
		// To start editing Metadata Field values, we need to have an asset id as a starting point
		if (empty($this->_running_vars['assets'])) {
			trigger_localised_error('HIPO0017', E_USER_WARNING);
			return FALSE;
		}

		// Allow an empty context ID, however it should be discouraged because
		// running it inside a Squiz Server process could have unpredictable
		// results
		if (empty($this->_running_vars['contextid'])) {
			$this->_running_vars['contextid'] = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		$this->_running_vars['done_assetids'] = Array();
		if (empty($this->_running_vars['metadata_field_value_changes'])) {
			// no Metadata Field Value changes supplied, so do nothing
			$this->_running_vars['todo_assetids'] = Array();
		} else {

			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$assets = $this->_running_vars['assets'];
			$child_assets = Array();
			for (reset($assets); NULL !== ($assetid = key($assets)); next($assets)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) {
					trigger_localised_error('HIPO0016', E_USER_WARNING, $assetid);
					return '';
				}
				$child_assets += $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid);
				$child_assets[$assetid]	= Array (
											0	=> Array (
													'type_code'	=> $asset->type(),
												   ),
										  );
				unset($asset);
			}
			$types_with_assets = Array();

			foreach ($child_assets as $assetid => $asset_info) {
				// if this asset does not allow the user to set metadata values then
				// I think its okay to assume that we dont need to set any schemas
				// because nothing is ever going to get filled in - thus required
				if (!$mm->allowsMetadata($assetid)) {
					unset($child_assets[$assetid]);
				}//end if
			}//end foreach

			$this->_running_vars['todo_assetids'] = $child_assets;
		}//end else

		return parent::prepare();

	}//end prepare()


	/**
	* Edit schemas for a single asset in the to-do list
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processFieldValues(&$step_data, $prefix)
	{
		$GLOBALS['SQ_SYSTEM']->changeContext($this->_running_vars['contextid']);

		if (!empty($this->_running_vars['todo_assetids'])) {
			reset($this->_running_vars['todo_assetids']);
			$assetid = key($this->_running_vars['todo_assetids']);
			$asset_type = $this->_running_vars['todo_assetids'][$assetid][0]['type_code'];
			unset($this->_running_vars['todo_assetids'][$assetid]);

			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

			if (!is_null($asset)) {
				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'metadata')) {
					foreach ($this->_running_vars['metadata_field_value_changes'] as $set_data) {
						$schemata = $mm->getSchemas($assetid);

						if (!empty($schemata) && $set_data['field_asset_id']) {
							$metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($set_data['field_asset_id']);
							if (is_null($metadata_field)) {
								$field_name = 'Unknown Field';
							} else {
								$field_name = $metadata_field->name;

								$mm->setMetadata($assetid, $set_data['field_value']);
								// All context metadata files need to be regenerated if this metadata field is non-contextable
								if ((boolean)$metadata_field->attr('is_contextable') === FALSE || $metadata_field instanceof Metadata_Field_Select) {
									$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
									foreach ($all_contexts as $contextid => $context_data) {
										$mm->generateContentFile($assetid, FALSE, $contextid);
									}
								} else {
									$mm->generateContentFile($assetid);
								}
							}
						}

					}//end foreach

					$step_data['message'] = translate('hipo_editing_metadata_field_values', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'metadata');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					unset($asset);
				} else {
					// we do not acquire lock
					trigger_localised_error('SYS0100', E_USER_WARNING, 'metadata', $asset->name, ' skipping asset.');
				}
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $assetid);
				$this->_addError(translate('hipo_cannot_edit_schema', $assetid));
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}//end if

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			unset($this->_running_vars['done_assetids']);
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		return TRUE;

	}//end processFieldValues()


	/**
	* This function is used to set-up the details for the locking the assets by the nested HIPO_Job
	*
	* @param string	&$job_type		job type
	* @param array	&$running_vars	running vars
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function getLockAssetDetails(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_acquire_locks';

		$options['auto_complete'] = TRUE;

		$running_vars = Array(
							'assetids'			=> array_keys($this->_running_vars['assets']),
							'lock_type'			=> Array('metadata'),
							'forceably_acquire'	=> FALSE,
							'dependants_only'	=> FALSE,
						);

	}//end getLockAssetDetails()


	/**
	* Release the locks we acquired while setting the schemas
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
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
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (count($this->_running_vars['done_assetids']) / count($this->_running_vars['assets'])) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end releaseLocks()


}//end class

?>
