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
* $Id: hipo_job_rename_thesaurus_term.inc,v 1.8 2013/07/25 23:23:49 lwright Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to rename thesaurus terms
* This HIPO will be restricted to do one thesaurus at a time, but many terms and root nodes
*
* <pre>
* Initial Running Vars :
*    string		thesaurusid		- the asset id of the thesaurus
*    array		terms			- an array of terms to change Array('old_term'=>'new_term')
*    array		assetids		- assetids to change
*	 boolean	cascade			- Cascade down these assetids (defaults to FALSE)
* </pre>
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Rename_Thesaurus_Term extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Rename_Thesaurus_Term($code_name='')
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
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('hipo_rename_thesaurus_term'));

		if ($write_access) {
			text_box($class.'[SQ_HIPO_RENAME_THESAURUS_TERM_THRESHOLD]', SQ_HIPO_RENAME_THESAURUS_TERM_THRESHOLD, 5);
		} else {
			echo SQ_HIPO_RENAME_THESAURUS_TERM_THRESHOLD;
		}
		echo ' '.translate('terms');

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
				'SQ_HIPO_RENAME_THESAURUS_TERM_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 1),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Acquire Lock HIPO appends the ID of the asset that the lock is being acquired on
	* (the top most parent) so that any other locking jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		$code_name = parent::getCodeName();
		return $code_name.'-'.md5(implode(',', $this->_running_vars['assetids']));

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_rename_thesaurus_term');

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
					'name'			=> translate('hipo_rename_thesaurus_term'),
					'function_call'	=> Array(
										'process_function'	=> 'processRenameTerm',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('hipo_generating_metadata_files'),
					'hipo_job'		=> Array(
										'init_details_function'	=> 'regenerateMetadata',
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
	* Returns a value that represents the total percentage that this hipo would run in
	* freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_RENAME_THESAURUS_TERM_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['total_term_count']) || empty($this->_running_vars['total_term_count'])) {
			return 0;
		}
		return (($this->_running_vars['total_term_count'] / SQ_HIPO_RENAME_THESAURUS_TERM_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$step_data = Array();

		// do the rename
		if (!$this->processRenameTerm($step_data, '')) {
			$this->_addError('updating thesaurus terms failed');
			return FALSE;
		}//end if

		// Nothing done?
		$schemas = array_keys($this->_running_vars['schemas']);
		if (empty($schemas)) {
			return TRUE;
		}//end if

		// Vars for the HIPO
		$vars = Array('schemaids' => $schemas);

		// Regenerate metadata
		$errors = $hh->freestyleHipo('hipo_job_regenerate_metadata', $vars);
		if (!empty($errors)) {
			$this->_addError('regenerating metadata failed');
			return FALSE;
		}//end if

		// All good
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// No thesaurus specified
		if (empty($this->_running_vars['thesaurusid'])) {
			trigger_localised_error('HIPO0085', E_USER_WARNING);
			return FALSE;
		}//end if

		// No terms to change, bailout
		if (empty($this->_running_vars['terms'])) {
			return FALSE;
		}//end if

		// No thesaurus found
		$thesaurus = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['thesaurusid'], '', TRUE);
		if (is_null($thesaurus) || $thesaurus->type() != 'thesaurus') {
			trigger_localised_error('HIPO0086', E_USER_WARNING);
			return FALSE;
		}//end if

		if (!isset($this->_running_vars['assetids'])) {
			$this->_running_vars['assetids'] = Array();
		}//end if

		$cascade = array_get_index($this->_running_vars, 'cascade', FALSE);
		if (!isset($this->_running_vars['assetids'])) {
			$this->_running_vars['assetids'] = Array();
		} else if (!empty($this->_running_vars['assetids']) && $cascade) {
			$assetids = Array();
			foreach ($this->_running_vars['assetids'] as $assetid) {
				array_splice($assetids, count($assetids), 0, array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($assetid)));
			}//end foreach
			array_splice($this->_running_vars['assetids'], count($this->_running_vars['assetids']), 0, $assetids);
		}//end if

		// Find all the schemas/fields needed for this operation
		$this->_findSchemas();

		if (!isset($this->_running_vars['fieldids']) || empty($this->_running_vars['fieldids'])) {
			return FALSE;
		}//end if

		return parent::prepare();

	}//end prepare()


	/**
	* This function is used to regenerate metadata on all affected schemas
	*
	* @param string	&$job_type		the hipo job type
	* @param array	&$running_vars	array contains the information for the hipo to run
	* @param array	&$options		options
	*
	* @return void
	* @access public
	*/
	function regenerateMetadata(&$job_type, &$running_vars, &$options)
	{
		$job_type = 'hipo_job_regenerate_metadata';

		$options['auto_complete'] = TRUE;

		$running_vars = Array('schemaids' => array_keys($this->_running_vars['schemas']));

	}//end regenerateMetadata()


	/**
	* Rename the thesaurus terms
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processRenameTerm(&$step_data, $prefix)
	{
		// Just starting let's gear up
		if (!isset($this->_running_vars['todo_terms'])) {
			$this->_running_vars['todo_terms'] = $this->_running_vars['terms'];
			$this->_running_vars['done_term_count'] = 0;
			$this->_running_vars['todo_term_count'] = count($this->_running_vars['todo_terms']);
			$this->_running_vars['total_term_count'] = count($this->_running_vars['todo_terms']);
		}//end if

		// Grab the current term
		$term_info = array_slice($this->_running_vars['todo_terms'], -1, 1, TRUE);
		$old_term = key($term_info);
		$new_term = $term_info[$old_term];

		// If either is empty, continue to the next one
		if (!empty($old_term) || !empty($new_term)) {
			// Do the rename per asset and play nice with ORACLE
			$chunk_size = 1000;
			$old_term     = str_replace(chr(0), '', $old_term);
			$new_term     = str_replace(chr(0), '', $new_term);
			$field_chunks = array_chunk($this->_running_vars['fieldids'], $chunk_size);
			foreach ($field_chunks as $field_chunk) {
				// Quoting Shakespeare
				foreach ($field_chunk as $index => $field_assetid) {
					$field_chunk[$index] = MatrixDAL::quote($field_assetid);
				}//end foreach

				// bug fix #4430 Thesaurus Terms Update this term system wide Issue
				// if we have more than one term applied, then we do it differently
				$sql  = 'SELECT value, assetid, fieldid, contextid FROM sq_ast_mdata_val';
				$sql .= ' WHERE value like '.MatrixDAL::quote($old_term.',%');
				$sql .= ' OR value like '.MatrixDAL::quote('%,'.$old_term);
				$sql .= ' OR value like '.MatrixDAL::quote('%,'.$old_term.',%');

				$results = MatrixDAL::executeSqlAssoc($sql);
				if (!empty($results)) {

					foreach($results as $index => $result) {
						$asset_id = $result['assetid'];
						$value    = $result['value'];
						$field_id = $result['fieldid'];
						$contextid= $result['contextid'];

						$pattern_1 = '/(.*)'.$old_term.'$/';
						$pattern_2 = '/^'.$old_term.'(.*)/';
						$pattern_3 = '/(.*)'.$old_term.'(.*)/';

						$replacement_1 = '$1'.$new_term;
						$replacement_2 = $new_term.'$1';
						$replacement_3 = '$1'.$new_term.'$2';

						if (preg_match($pattern_2, $value)) {
							$new_value = preg_replace($pattern_2, $replacement_2, $value);
						} else if (preg_match($pattern_1, $value)) {
							$new_value = preg_replace($pattern_1, $replacement_1, $value);
						} else {
							$new_value = preg_replace($pattern_3, $replacement_3, $value);
						}

						// Run the Query against the current assetid
						if (MatrixDAL::getDbType() === 'oci') {
							$sql  = 'UPDATE sq_ast_mdata_val SET value=:new_value WHERE TO_CHAR(value)=:old_value AND contextid=:contextid AND fieldid=:fieldid AND assetid=:assetid';
						} else {
							$sql  = 'UPDATE sq_ast_mdata_val SET value=:new_value WHERE value=:old_value AND contextid=:contextid AND fieldid=:fieldid AND assetid=:assetid';
						}//end if

						if(!empty($this->_running_vars['assetids'])) {
							foreach ($this->_running_vars['assetids'] as $assetid) {
								try{
									$query = MatrixDAL::preparePdoQuery($sql);
									MatrixDAL::bindValueToPdo($query, 'new_value', $new_value);
									MatrixDAL::bindValueToPdo($query, 'old_value', $value);
									MatrixDAL::bindValueToPdo($query, 'contextid', $contextid);
									MatrixDAL::bindValueToPdo($query, 'fieldid', $field_id);
									MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
									MatrixDAL::execPdoQuery($query);
								} catch (Exception $e) {
									throw new Exception('DB Error: '.$e->getMessage());
								}
							}
						} else {
							try{
								$query = MatrixDAL::preparePdoQuery($sql);
								MatrixDAL::bindValueToPdo($query, 'new_value', $new_value);
								MatrixDAL::bindValueToPdo($query, 'old_value', $value);
								MatrixDAL::bindValueToPdo($query, 'contextid', $contextid);
								MatrixDAL::bindValueToPdo($query, 'fieldid', $field_id);
								MatrixDAL::bindValueToPdo($query, 'assetid', $asset_id);
								MatrixDAL::execPdoQuery($query);
							} catch (Exception $e) {
								throw new Exception('DB Error: '.$e->getMessage());
							}
						}
					}
				}
				
				// Run the Query against the current assetid
				if (MatrixDAL::getDbType() === 'oci') {
					$sql  = 'UPDATE sq_ast_mdata_val SET value=:new_term WHERE TO_CHAR(value)=:old_term AND fieldid IN ('.implode(',', $field_chunk).')';
				} else {
					$sql  = 'UPDATE sq_ast_mdata_val SET value=:new_term WHERE value=:old_term AND fieldid IN ('.implode(',', $field_chunk).')';
				}//end if
				if (!empty($this->_running_vars['assetids'])) {
					// Update by asset
					$sql .= ' AND assetid=:assetid';
					foreach ($this->_running_vars['assetids'] as $assetid) {
						try {
							$query = MatrixDAL::preparePdoQuery($sql);
							MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
							MatrixDAL::bindValueToPdo($query, 'new_term', $new_term);
							MatrixDAL::bindValueToPdo($query, 'old_term', $old_term);
							MatrixDAL::execPdoQuery($query);
						} catch (Exception $e) {
							throw new Exception('DB Error: '.$e->getMessage());
						}
					}//end foreach
				} else {
					// Update EVERYTHING
					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'new_term', $new_term);
						MatrixDAL::bindValueToPdo($query, 'old_term', $old_term);
						MatrixDAL::execPdoQuery($query);
					} catch (Exception $e) {
						throw new Exception('DB Error: '.$e->getMessage());
					}
				}//end if
			}//end foreach
		}//end if

		// Move the asset out and increate step data
		$this->_running_vars['done_term_count']++;
		$dump = array_pop($this->_running_vars['todo_terms']);

		// Update Step data
		if (empty($this->_running_vars['todo_terms'])) {
			unset($this->_running_vars['done_term_count']);
			unset($this->_running_vars['todo_term_count']);
			unset($this->_running_vars['total_term_count']);
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$step_data['percent_done'] = (int)(($this->_running_vars['done_term_count'] / $this->_running_vars['total_term_count']) * 100);
			$step_data['complete']     = FALSE;
		}//end if

		return TRUE;

	}//end processRenameTerm()


	/**
	*  Find a schemas and fields per thesaurus
	*
	* @return boolean
	* @access public
	*/
	function _findSchemas()
	{
		// Match the schemas and fields to the terms to change
		$current_schemas = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('metadata_schema', TRUE);
		foreach ($current_schemas as $schemaid) {
			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'metadata_schema', TRUE);
			if (!is_null($schema)) {
				$this->_running_vars['schemas'][$schemaid] = Array();
				$fields = $GLOBALS['SQ_SYSTEM']->am->getChildren($schemaid, 'metadata_field_thesaurus');
				foreach ($fields as $fieldid => $field_type) {
					$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid, 'metadata_field_thesaurus', TRUE);
					if (!is_null($field)) {
						$field_thesaurus = $field->attr('thesaurus_assetid');
						if ($field_thesaurus == $this->_running_vars['thesaurusid']) {
							// We have a winner!
							$this->_running_vars['schemas'][$schemaid][] = $fieldid;
							$this->_running_vars['fieldids'][] = $fieldid;
						}//end if
					}//end if
				}//end foreach

				// No fields, so don't worry about this schema
				if (count($this->_running_vars['schemas'][$schemaid]) == 0) {
					unset($this->_running_vars['schemas'][$schemaid]);	
				}//end if
			}//end if
		}//end foreach

		return TRUE;

	}//end _findSchemas()


}//end class

?>
