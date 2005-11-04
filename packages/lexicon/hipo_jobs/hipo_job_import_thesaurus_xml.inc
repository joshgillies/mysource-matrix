<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_import_thesaurus_xml.inc,v 1.9 2005/11/04 06:04:40 arailean Exp $
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
* @version $Revision: 1.9 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Import_Thesaurus_XML extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name	a unique codename the HIPO
	*/
	function HIPO_Job_Import_Thesaurus_XML($code_name='')
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
		return translate('lexicon_hipo_name_import_thesaurus_xml');

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
					'name'			=> translate('lexicon_hipo_starting_process'),
					'function_call'	=> Array(
										'process_function'	=> 'processStart',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> false,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> translate('lexicon_hipo_setting_up_env'),
				),
				Array(
					'name'			=> translate('lexicon_hipo_acquiring_locks'),
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
					'name'			=> translate('lexicon_hipo_erasing_thesaurus'),
					'function_call'	=> Array(
										'paint_function'	=> 'paintEraseContents',
										'process_function'	=> 'processEraseContents',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> false,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('lexicon_hipo_parsing_xml'),
					'function_call'	=> Array(
										'process_function'	=> 'processParseXML',
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
			trigger_localised_error('LEX0002', E_USER_WARNING);
			return false;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$this->getThesaurusRef();
		if (is_null($asset)) {
			trigger_localised_error('LEX0003', E_USER_WARNING, $this->_running_vars['thesaurus_id']);
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
	* Paints the interface for the 'Erase Thesaurus Contents' step
	*
	* @param array	&$step_data	step data
	* @param object	&$o			outputter
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function paintEraseContents(&$step_data, &$o, $prefix)
	{
		$o->openSection(translate('lexicon_hipo_erasing_thesaurus'));
		$o->openField('');

		echo translate('lexicon_hipo_confirm_erase');

		$o->closeField();
		$o->closeSection();

		return true;

	}//end paintEraseContents()


	/**
	* Erase Thesaurus Contents
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processEraseContents(&$step_data, $prefix)
	{

		$th_asset = &$this->getThesaurusRef();

		if (!$th_asset->erase()) {
			trigger_localised_error('LEX0004', E_USER_WARNING);
			return false;
		}

		$step_data['percent_done'] = 100;
		$step_data['complete'] = true;

		return true;

	}//end processEraseContents()


	/**
	* Process a start element from the XML parser
	*
	* @param object	$parser	the calling xml parser
	* @param string	$name	tag name
	* @param array	$attrs	tag attributes
	*
	* @return boolean
	* @access public
	*/
	function startElement($parser, $name, $attrs)
	{

		$stack =& $this->_running_vars['stack'];

		$type = $name; //reduce confusion with the name attribute.

		$element = htmlentities('<'.$type.' name="'.array_get_index($attrs,'name').'">');
		//echo($element);
		if (! ($type == 'thesaurus' ||$type == TERM || $type == RELATION)) {
			trigger_localised_error('LEX0005', E_USER_WARNING, $type);
		}
		//whip the top element off and have a squiz
		$top_of_stack = array_pop($stack);
		array_push($stack,$top_of_stack);

		//check new terms to make sure they're either a term or a relation

		if ($type == RELATION) {
			if ($top_of_stack['type']==TERM) {
				$new_relation  = Array(
									'type'	=> 'relation',
									'name'	=> array_get_index($attrs,'name'),
								 );
				array_push($stack, $new_relation);

			} else if ($top_of_stack['type']==RELATION) {
			// Error You cant relate a relation to a term.
				trigger_localised_error('LEX0006', E_USER_WARNING, $element);
			}
		} else if ($type == TERM) {
			$term_name = array_get_index($attrs,'name');
			if (empty($term_name)) {
				trigger_localised_error('LEX0007', E_USER_WARNING);
			}
			if ($top_of_stack['type']!=RELATION) {
			// if the top of stack is not a relation, throw a null relation in to extend the shorthand form.
				$null_relation  = Array(
									'type'	=> 'relation',
									'name'	=> '',
								  );
				array_push($stack, $null_relation);
				$top_of_stack = $null_relation;
			}
			// We've got a term link. Send this to thesaurus.
			//this term link is composed of the current term, the top of stack(relation, and the one under that (parent term)
			$relation = array_pop($stack);
			$parent_term = array_pop($stack);
			array_push($stack,$parent_term);
			array_push($stack,$relation);

			$th_asset = &$this->getThesaurusRef();

			$th_asset->addTerm($attrs['name'], $parent_term['name'], $relation['name']);

			// add the term to the stack
			$new_term  = Array(
							'type'	=> 'term',
							'name'	=> $attrs['name'],
						 );
				array_push($stack, $new_term);

		} else if (!$type == 'thesaurus') {
			trigger_localised_error('LEX0008', E_USER_WARNING, $type);
		}
	}//end startElement()


	/**
	* Process an end element from the XML parser
	*
	* @param object	$parser	the calling xml parser
	* @param string	$name	tag name
	*
	* @return boolean
	* @access public
	*/
	function endElement($parser, $name)
	{
		$type = $name; //less confusion with the name attribute

		$stack =& $this->_running_vars['stack'];
		$element = htmlentities('</'.$type.'>');
		//echo $element;
		if (! ($type == 'thesaurus' ||$type == TERM || $type == RELATION)) {
			trigger_localised_error('LEX0005', E_USER_WARNING, $type);
		}
		//whip the top element off and have a squiz
		$top_of_stack = array_pop($stack);
		array_push($stack,$top_of_stack);

		// we want to pop a tag off the stack, so hopefully the last tag on the stack is a match, or a null relation
		if ($type == TERM) {
		// if this tag is a term
			if ($top_of_stack['type'] == TERM) {
			// if the top of stack is a term
				array_pop($stack); // pop the term
			} else if ($top_of_stack['type'] == RELATION && empty($top_of_stack['name'])) {
			// if the top of the stack is a null relation
				// We could check these and make sure we popped what we expected...
				array_pop($stack);// pop the null relation
				array_pop($stack);// then pop the term
			} else if ($top_of_stack['type'] == RELATION &&  ! empty($top_of_stack['name'])) {
			// if the top of the stack is a non-null relation
				trigger_localised_error('LEX0009', E_USER_WARNING, $top_of_stack['name']);
			}
		} else if ($type == RELATION) {
		// elseif this tag is a relation
			if ($top_of_stack['type'] == TERM) {
			// if the top of the stack is a term
				trigger_localised_error('LEX0010', E_USER_WARNING);
			} else if ($top_of_stack['type'] == RELATION) {
			// if the top of the stack is a relation
				array_pop($stack); // pop the relation
			}
		} else if ($type == 'thesaurus') {
			//we're at the end. do nothing for now. Could do a bunch of checking to make sure our stack is empty when we get to the end of the file
		}

	}//end endElement()


	/**
	* Parse the supplied XML file
	*
	* @param array	&$step_data	step data
	* @param string	$prefix		unique prefix
	*
	* @return boolean
	* @access public
	*/
	function processParseXml(&$step_data, $prefix)
	{
		define('TERM', 'term');
		define('RELATION','relation');

		$block_size = 4096;
		$blocks_per_step = 10;


		if (!isset($step_data['total_read'])) {
			//assume this is the first step because we must read something on each step
			$step_data['total_parsed']= 0;
			$step_data['total_read']= 0;
			$step_data['remainder'] ='';
		}
		if (!isset($this->_running_vars['stack'])) {
			//Create and initialise the parser
			$stack = Array();
			$this->_running_vars['stack'] = $stack;
		}


		$xml_parser = xml_parser_create();
		//stop the parser ucasing everything
		xml_parser_set_option ($xml_parser, XML_OPTION_CASE_FOLDING, 0);


		//initialise the file pointer
		$th_asset = &$this->getThesaurusRef();
		$import_path = $th_asset->getXmlFilePath();

		if (!($fp = fopen($import_path, 'r'))) {
			trigger_localised_error('LEX0011', E_USER_WARNING, $import_path);
			return false;
			die();
		}

		$fstat = fstat($fp);
		$file_size = $fstat['size'];

		if ($step_data['total_read'] > 0) {
			// We have to parse the seen portion of the file with the tag handlers unset to recreate the parser state. Sad but true.
			$data = fread($fp, $step_data['total_read']);
			if (!xml_parse($xml_parser, $data, feof($fp))) {
				trigger_localised_error('LEX0012', E_USER_WARNING,
										xml_error_string(xml_get_error_code($xml_parser)),
										htmlentities($data)
										);
				return false;
			}
		}

		$blocks =1;
		while (!feof($fp) && $blocks<=$blocks_per_step) {

			// Parse blocks from the file
			$data = fread($fp, $block_size);
			xml_set_element_handler($xml_parser, Array(&$this,'startElement'),Array(&$this,'endElement'));
			if (!xml_parse($xml_parser, $data, feof($fp))) {
				trigger_localised_error('LEX0012', E_USER_WARNING,
										xml_error_string(xml_get_error_code($xml_parser)),
										htmlentities($data)
										);
				return false;
			}

			$parsed = xml_get_current_byte_index($xml_parser);
			$step_data['remainder'] = substr($data,$parsed);
			$step_data['total_read'] = $step_data['total_read'] + $block_size;
			$step_data['total_parsed'] = $step_data['total_parsed'] + $parsed;
			$blocks++;
		}


		$step_data['percent_done'] = ($step_data['total_read']/$file_size) * 100;

		if (feof($fp)) {
			//unset all running vars we are now finished with
			$step_data['complete'] = true;
			xml_parser_free($xml_parser);
		} else {
			$step_data['complete'] = false;
		}
		$step_data['message']  = 'Parsed '.sprintf('%01.2f',$step_data['total_read']/1024).' Kb of '.sprintf('%01.2f',$file_size/1024).' Kb';
		fclose($fp);
		return true;
	}//end processParseXml()


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
