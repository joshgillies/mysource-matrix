<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_export_thesaurus_xml.inc,v 1.3 2005/12/09 04:02:21 emcdonald Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to import thesaurus xml
*
* Initial Running Vars :
*    thesaurus_id	-	the ID of the thesaurus that is being added to
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.3 $
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
		$this->uses_trans = true;
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
		return translate('thesaurus_hipo_name_export_thesaurus_xml');

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
					'name'			=> translate('thesaurus_hipo_starting_process'),
					'function_call'	=> Array(
										'process_function'	=> 'processStart',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> false,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> translate('thesaurus_hipo_setting_up_env'),
				),
				Array(
					'name'			=> translate('thesaurus_hipo_acquiring_locks'),
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
					'name'			=> translate('thesaurus_hipo_creating_xml'),
					'function_call'	=> Array(
										'process_function'	=> 'processCreateXML',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> false,
					'auto_step'		=> true,
					'allow_cancel'	=> false,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('thesaurus_hipo_finalising'),
					'function_call'	=> Array(
										'process_function'	=> 'processFinish',
									   ),
					'running_mode'	=> 'server',
					'skip_step'		=> false,
					'auto_step'		=> true,
					'allow_cancel'	=> false,
					'percent_done'	=> 0,
					'complete'		=> false,
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
			return false;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$this->getThesaurusRef();
		if (is_null($asset)) {
			trigger_localised_error('HIPO0073', E_USER_WARNING, $this->_running_vars['thesaurus_id']);
			return false;
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
		$step_data['complete'] = true;
		return true;

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

		$thesaurus = &$this->getThesaurusRef();
		$thesaurus->setAttrValue('contents_changed', false);
		$thesaurus->saveAttributes();
		$step_data['percent_done'] = 100;
		$step_data['complete'] = true;
		return true;

	}//end processFinish()


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
		$thesaurus = &$this->getThesaurusRef();
		$filepath = $thesaurus->getXmlFilePath();
		if (!file_exists($filepath)) touch($filepath);
		$filepath = realpath($filepath);
		if (!is_writable($filepath)) {
			trigger_localised_error('HIPO0074', E_USER_WARNING, $filepath);
		}
		$finished = false;
		$iterations_per_step = 2500;
		$iterations_this_step = 0;

		if (!isset($step_data['unprocessed'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$step_data['total_links'] = $thesaurus->countTermLinks();
			$step_data['links_closed'] = 0;
			$filehandle = fopen($filepath, 'wb');

			$parent = null;
			$parentid = null;
			$depth = 0;

			$seen_terms = Array(); // the list of files that have been seen
			$open_terms[0]['term'] = null;
			$open_terms[0]['termid'] = null;
			$open_terms[0]['relation'] = null;

			//the parent of the level 0 children is the null term
			$unprocessed[-1][0]['term'] = null;
			$unprocessed[-1][0]['termid'] = null;

			fwrite($filehandle, "<thesaurus>\n");

		} else {
			$filehandle = fopen($filepath, 'ab');

			$parent = $step_data['parent'];
			$parentid = $step_data['parentid'];
			$depth = $step_data['depth'];

			$seen_terms = $step_data['seen_terms']; // the list of files that have been seen
			$open_terms = $step_data['open_terms']; // the stack of term tags that are open right now and the relations they have open
			$unprocessed = $step_data['unprocessed']; // the tree of unprocessed children

		}

		while (!$finished && $iterations_this_step <= $iterations_per_step) {
			$iterations_this_step++;

			// First try to load links for processing in case this parent needs them loaded
			if (empty($unprocessed[$depth]) && !isset($seen_terms[$parentid])) {
				$unprocessed[$depth] = $thesaurus->getChildTerms($parentid);
			}

			// there are no child term relations to output for this parent, so close it.
			if (empty($unprocessed[$depth])) {

				// if the term is already open. It will be if we have been outputting children for it earlier
				if ($open_terms[0]['term'] == $parent) {

					// before we close the term, close any open relations remaining
					if (!is_null($open_terms[0]['relation'])) {
						fwrite($filehandle, "</relation>\n");
					}

					// close the term tag unless the term is null
					if (!is_null($parent)) {
						fwrite($filehandle, "</term>\n");
					}

					$closed_term = array_shift($open_terms);

				} else {
					fwrite($filehandle, '<term name="'.$parent.'"/>'."\n");
				}
				$step_data['links_closed']++;

				//now we can remove this term from the stack
				if ($depth > 0) {
					$depth--;
					$finished_parent = array_shift($unprocessed[$depth]);
					//go back to our parent to find the next child
					$parent = $unprocessed[$depth - 1][0]['term'];
					$parentid = $unprocessed[$depth - 1][0]['termid'];
				} else {
					$finished = true;
				}
				continue;
			}

			// Put this term in the 'seen' array so that we don't output it's children again
			// Array indexed by parent name, value is the depth at which we started processing it
			$seen_terms[$parentid] = $depth;

			// open the term tag unless the term is null or the tag is open
			if (!(is_null($parent) || $parent == $open_terms[0]['term'])) {
				$new_term = Array(
								'term'		=> $parent,
								'termid'	=> $parentid,
								'relation'	=> null,
							);
				array_unshift($open_terms, $new_term);
				fwrite($filehandle, '<term name="'.$parent.'">'."\n");
			}


			// Process a term link
			$term_link = $unprocessed[$depth][0];

			$link_relation = $term_link['relation'];
			$link_child = $term_link['term'];
			$link_childid = $term_link['termid'];

			// Deal with the relation tags
			// if there is a relation open

			if ($open_terms[0]['relation'] !== $link_relation) {

				if (!is_null($open_terms[0]['relation'])) {
					// close it
					fwrite($filehandle, "</relation>\n");
				}
				$open_terms[0]['relation'] = $link_relation;
				if (!is_null($link_relation)) {
					fwrite($filehandle, '<relation name="'.$link_relation.'">'."\n");
				}
			}
			// Reiterate the loop down one depth step with a new parent
			$parent = $link_child;
			$parentid = $link_childid;
			$depth++;
		} //end while.

		$step_data['percent_done'] = ($step_data['links_closed']/$step_data['total_links']) * 100;
		$step_data['message'] = 'Processed '.$step_data['links_closed'].' of '.$step_data['total_links'].' term links';

		if ($finished) {
			fwrite($filehandle, "</thesaurus>\n");
			$step_data['complete'] = true;
		} else {
			$step_data['parent'] = $parent;
			$step_data['parentid'] = $parentid;
			$step_data['depth'] = $depth;

			$step_data['seen_terms'] = $seen_terms; // the list of files that have been seen
			$step_data['open_terms'] = $open_terms; // the stack of term tags that are open right now and the relations they have open
			$step_data['unprocessed'] = $unprocessed;  // the tree of unprocessed children

			$step_data['complete'] = false;
		}
		fclose($filehandle);
		return true;
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

		$options['auto_complete'] = true;

		$running_vars = Array(
							'assetids'			=> Array($this->_running_vars['thesaurus_id'],),
							'lock_type'			=> 'attributes',
							'dependants_only'	=> false,
							'forceably_acquire'	=> false,
						);
	}//end getLockAssetDetails()


	/**
	* Get a reference to the thesaurus asset
	*
	* @return object
	* @access public
	*/
	function &getThesaurusRef()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['thesaurus_id']);

	}//end getThesaurusRef()


}//end class
?>
