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
* $Id: hipo_job_export_thesaurus_xml.inc,v 1.11 2012/08/30 01:09:18 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**
* Highly Intensive Processing Object (HIPO) to import thesaurus xml
*
* Initial Running Vars :
*    thesaurus_id	-	the ID of the thesaurus that is being added to
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Export_Thesaurus_XML extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Export_Thesaurus_XML($code_name='')
	{
		$this->uses_trans = TRUE;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['thesaurus_id'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Export Thesaurus XML');


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
					'name'			=> translate('Starting up process'),

					'function_call'	=> Array(
										'process_function'	=> 'processStart',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> translate('Setting up the environment'),

				),
				Array(
					'name'			=> translate('Acquiring Permission Locks'),

					'hipo_job'		=> Array(
										'init_details_function'	=> 'getLockAssetDetails',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Writing XML file'),

					'function_call'	=> Array(
										'process_function'	=> 'processCreateXML',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> FALSE,
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Finalising'),

					'function_call'	=> Array(
										'process_function'	=> 'processFinish',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> FALSE,
					'auto_step'		=> TRUE,
					'allow_cancel'	=> FALSE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start regenerating metadata, we need to have a root assetid - default to root folder if none is supplied
		if (is_null($this->_running_vars['thesaurus_id'])) {
			trigger_localised_error('HIPO0072', E_USER_WARNING);
			return FALSE;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = $this->getThesaurusRef();
		if (is_null($asset)) {
			trigger_localised_error('HIPO0073', E_USER_WARNING, $this->_running_vars['thesaurus_id']);
			return FALSE;
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Dummy function that runs an empty step, so that there is no empty screen
	* before the end of the first real step
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processStart(&$step_data, $prefix)
	{
		$step_data['percent_done'] = 100;
		$step_data['complete'] = TRUE;
		return TRUE;

	}//end processStart()


	/**
	* Mark the contents changed attribute as false now that a new file has been generated
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processFinish(&$step_data, $prefix)
	{

		$thesaurus = $this->getThesaurusRef();
		$thesaurus->setAttrValue('contents_changed', FALSE);
		$thesaurus->saveAttributes();
		$step_data['percent_done'] = 100;
		$step_data['complete'] = TRUE;
		return TRUE;

	}//end processFinish()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$step_data = Array();

		// skip the confirmation screen, go straight into locking
		$job_type = '';
		$running_vars = Array();
		$options = '';
		$this->getLockAssetDetails($job_type, $running_vars, $options);
		$errors = $hh->freestyleHipo($job_type, $running_vars);
		if (!empty($errors)) {
			$this->_addError('acquiring locks failed');
			return FALSE;
		}

		// generate the xml
		do {
			if (!$this->processCreateXML($step_data, '')) {
				$this->_addError('generating the xml failed');
				$success = FALSE;
				return FALSE;
			}
		} while (!$step_data['complete']);

		return TRUE;

	}//end freestyle()


	/**
	* Create an XML representation of the term table and output it to file
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processCreateXML(&$step_data, $prefix)
	{
		$thesaurus = $this->getThesaurusRef();
		$filepath = $thesaurus->getXmlFilePath();
		if (file_exists($filepath)) {
			$filepath = realpath($filepath);
		}
		$finished = FALSE;
		$iterations_per_step = 100;
		$iterations_this_step = 0;

		$all_relations = $thesaurus->getActiveRelations();

		if (!isset($step_data['unprocessed'])) {
			if (file_exists($filepath)) unlink($filepath);
			create_directory(dirname($filepath));
			touch($filepath);

			$step_data['total_terms'] = $thesaurus->countTerms();
			$step_data['links_closed'] = 0;
			$filehandle = fopen($filepath, 'wb');

			$step_data['unprocessed'] = $thesaurus->getAllTerms();
			$step_data['seen_terms'] = Array();

			fwrite($filehandle, '<!-- MySource Matrix Thesaurus -->'."\n");
			fwrite($filehandle, '<!-- Generated on '.date('r').' -->'."\n");
			fwrite($filehandle, '<!-- Do not import into system version less than 3.12 -->'."\n");
			fwrite($filehandle, '<thesaurus>'."\n");

		} else {
			$filehandle = fopen($filepath, 'ab');
		}

		if (!is_writable($filepath)) {
			trigger_localised_error('HIPO0074', E_USER_WARNING, $filepath);
		}

		$unprocessed =& $step_data['unprocessed'];
		$seen_terms =& $step_data['seen_terms'];

		while (!$finished && $iterations_this_step <= $iterations_per_step) {
			$iterations_this_step++;

			if (empty($unprocessed)) {
				$finished = TRUE;
				continue;
			}

			array_unshift($seen_terms, array_shift($unprocessed));

			$current_term =& $seen_terms[0];

			$termid = $current_term['termid'];
			$term = $current_term['term'];
			$term_notes = $thesaurus->getTermNotes($termid);
			$child_relations = $thesaurus->getChildRelationsForTerm($termid);

			if (empty($term_notes) && empty($child_relations)) {
				fwrite($filehandle, "\t".'<term name="'.htmlentities($term, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'" />'."\n");
				continue;
			}

			fwrite($filehandle, "\t".'<term name="'.htmlentities($term, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'">'."\n");

			foreach ($term_notes as $note_array) {
				fwrite($filehandle, "\t\t".'<note name="'.htmlentities($note_array['name'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'">'.$note_array['value'].'</note>'."\n");
			}

			foreach ($child_relations as $relid => $relname) {
				$extra_tab = '';
				if (!is_null($relname)) {
					fwrite($filehandle, "\t\t".'<relation name="'.htmlentities($relname, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'">'."\n");
					$extra_tab = "\t";
				}
				$rel_terms = $thesaurus->getChildTerms($termid, $relid);
				foreach ($rel_terms as $term) {
					fwrite($filehandle, $extra_tab."\t\t".'<term name="'.htmlentities($term['term'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'" />'."\n");
				}
				if (!is_null($relname)) {
					fwrite($filehandle, "\t\t".'</relation>'."\n");
				}
			}

			fwrite($filehandle, "\t".'</term>'."\n");

		}//end while

		$step_data['percent_done'] = (count($seen_terms)/$step_data['total_terms']) * 100;
		$step_data['message'] = 'Processed '.count($seen_terms).' of '.$step_data['total_terms'].' terms';

		if ($finished) {
			fwrite($filehandle, '</thesaurus>'."\n");
			$step_data['complete'] = TRUE;
		} else {
			$step_data['complete'] = FALSE;
		}
		fclose($filehandle);
		return TRUE;

	}//end processCreateXML()


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
							'assetids'			=> Array($this->_running_vars['thesaurus_id'],),
							'lock_type'			=> 'attributes',
							'dependants_only'	=> FALSE,
							'forceably_acquire'	=> FALSE,
						);

	}//end getLockAssetDetails()


	/**
	* Get a reference to the thesaurus asset
	*
	* @return object
	* @access public
	*/
	function getThesaurusRef()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['thesaurus_id']);

	}//end getThesaurusRef()


}//end class
?>
