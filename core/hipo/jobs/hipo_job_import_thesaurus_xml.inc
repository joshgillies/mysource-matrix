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
* $Id: hipo_job_import_thesaurus_xml.inc,v 1.5.2.1 2006/09/04 23:38:35 emcdonald Exp $
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
* @version $Revision: 1.5.2.1 $
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
		return translate('thesaurus_hipo_name_import_thesaurus_xml');

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
					'name'			=> translate('thesaurus_hipo_erasing_thesaurus'),
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
					'name'			=> translate('thesaurus_hipo_parsing_xml'),
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
			trigger_localised_error('HIPO0061', E_USER_WARNING);
			return false;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$this->getThesaurusRef();
		if (is_null($asset)) {
			trigger_localised_error('HIPO0062', E_USER_WARNING, $this->_running_vars['thesaurus_id']);
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
		$o->openSection(translate('thesaurus_hipo_erasing_thesaurus'));
		$o->openField('');

		echo translate('thesaurus_hipo_confirm_erase');

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
			trigger_localised_error('HIPO0063', E_USER_WARNING);
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
	* @param string	$tag	tag name
	* @param array	$attrs	tag attributes
	*
	* @return boolean
	* @access public
	*/
	function startElement($parser, $tag, $attrs)
	{

		$stack =& $this->_running_vars['stack'];

		if ($tag != 'thesaurus' && $tag != 'term' && $tag != 'relation') {
			trigger_localised_error('HIPO0064', E_USER_WARNING, $tag);
		}

		if (empty($stack) && $tag != 'thesaurus') {
			trigger_error('EMPTY STACK');
			return false;
		}

		switch ($tag) {
			case 'thesaurus':
				if (empty($stack)) {
					array_unshift($stack, Array('term' => null, 'relation' => null));
				} else {
					trigger_localised_error('HIPO0067', E_USER_WARNING, $tag);
					return false;
				}
			break;

			case 'relation':
				if (!is_null($stack[0]['relation'])) {
					trigger_localised_error('HIPO0065', E_USER_WARNING, $tag);
					return false;
				} else {
					$stack[0]['relation'] = array_get_index($attrs, 'name');
				}
			break;

			case 'term':
				if (empty($attrs['name'])) {
					trigger_localised_error('HIPO0066', E_USER_WARNING);
					return false;

				} else {
					// add the term
					if (is_null($stack[0]['term'])) {
						$parent_asset =& $this->getThesaurusRef();
					} else {
						$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($stack[0]['term']);
					}

					$create_link['asset'] = &$parent_asset;
					$create_link['value'] = $stack[0]['relation'];

					$term_asset = new Thesaurus_Term();
					$term_asset->setAttrValue('name', $attrs['name']);
					$term_asset->create($create_link);

					// put it onto the stack
					array_unshift($stack, Array('term' => $term_asset->id, 'relation' => null));
				}
			break;

			default:
				trigger_localised_error('HIPO0067', E_USER_WARNING, $tag);
		}

	}//end startElement()


	/**
	* Process an end element from the XML parser
	*
	* @param object	$parser	the calling xml parser
	* @param string	$tag	tag name
	*
	* @return boolean
	* @access public
	*/
	function endElement($parser, $tag)
	{
		$stack =& $this->_running_vars['stack'];

		if ($tag != 'thesaurus' && $tag != 'term' && $tag != 'relation') {
			trigger_localised_error('HIPO0064', E_USER_WARNING, $tag);
		}

		if (empty($stack)) {
			trigger_error('EMPTY STACK');
			return false;
		}

		switch ($tag) {
			case 'thesaurus':
				if (is_null($stack[0]['term'])) {
					array_shift($stack);
				} else {
					trigger_error('Malformed XML. Unexpected tag: <'.$tag.'>.');
					return false;
				}
			break;

			case 'relation':
				if (!is_null($stack[0]['relation'])) {
					$stack[0]['relation'] = null;
				} else {
					trigger_error('Malformed XML. Unexpected tag: <'.$tag.'>.');
					return false;
				}
			break;

			case 'term':
				if (!is_null($stack[0]['term'])) {
					array_shift($stack);
				} else {
					trigger_error('Malformed XML. Unexpected tag: <'.$tag.'>.');
					return false;
				}
			break;

			default:
				trigger_localised_error('HIPO0067', E_USER_WARNING, $tag);
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

		$block_size = 4096;
		$blocks_per_step = 10;


		$GLOBALS['SQ_SYSTEM']->am->includeAsset('thesaurus_term');

		if (!isset($step_data['total_read'])) {
			//assume this is the first step because we must read something on each step
			$step_data['total_parsed'] = 0;
			$step_data['total_read'] = 0;
			$step_data['remainder'] = '';
		}

		if (!isset($this->_running_vars['stack'])) {
			$this->_running_vars['stack'] = Array();
		}


		$xml_parser = xml_parser_create();
		//stop the parser ucasing everything
		xml_parser_set_option ($xml_parser, XML_OPTION_CASE_FOLDING, 0);


		//initialise the file pointer
		$th_asset = &$this->getThesaurusRef();
		$import_path = $th_asset->getXmlFilePath();

		if (!($fp = fopen($import_path, 'r'))) {
			trigger_localised_error('CORE0257', E_USER_WARNING, $import_path);
			return false;
			die();
		}

		$fstat = fstat($fp);
		$file_size = $fstat['size'];

		if ($step_data['total_read'] > 0) {
			// We have to parse the seen portion of the file with the tag handlers unset to recreate the parser state. Sad but true.
			$data = fread($fp, $step_data['total_read']);
			if (!xml_parse($xml_parser, $data, feof($fp))) {
				trigger_localised_error('CORE0258', E_USER_WARNING, xml_error_string(xml_get_error_code($xml_parser)), htmlentities($data));
				return false;
			}
		}

		$blocks = 1;
		while (!feof($fp) && $blocks <= $blocks_per_step) {

			// Parse blocks from the file
			$data = fread($fp, $block_size);
			xml_set_element_handler($xml_parser, Array(&$this,'startElement'), Array(&$this,'endElement'));
			if (!xml_parse($xml_parser, $data, feof($fp))) {
				trigger_localised_error('CORE0258', E_USER_WARNING, xml_error_string(xml_get_error_code($xml_parser)), htmlentities($data));
				return false;
			}

			$parsed = xml_get_current_byte_index($xml_parser);

			$step_data['remainder'] = substr($data, $parsed);
			$step_data['total_read'] = $step_data['total_read'] + strlen($data);
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
	* Get a reference to the thesaurus assets
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
