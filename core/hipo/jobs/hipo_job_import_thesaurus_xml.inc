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
* $Id: hipo_job_import_thesaurus_xml.inc,v 1.17 2012/08/30 01:09:18 ewang Exp $
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
* @version $Revision: 1.17 $
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
		return translate('Import Thesaurus XML');


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
					'name'			=> translate('Erasing Thesaurus Contents'),

					'function_call'	=> Array(
										'paint_function'	=> 'paintEraseContents',
										'process_function'	=> 'processEraseContents',
									   ),
					'running_mode'	=> 'web',
					'auto_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> translate('Parsing XML file'),

					'function_call'	=> Array(
										'process_function'	=> 'processParseXML',
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
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		$step_data = Array();
		if (!$this->processEraseContents($step_data, '')) {
			return FALSE;
		}

		$step_data['complete'] = FALSE;
		while (!$step_data['complete']) {
			if (!$this->processParseXml($step_data, '')) {
				return FALSE;
			}
		}
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
		// to start regenerating metadata, we need to have a root assetid - default to root folder if none is supplied
		if (is_null($this->_running_vars['thesaurus_id'])) {
			trigger_localised_error('HIPO0061', translate('Unable to initialise "Import Thesaurus XML" HIPO, thesaurus asset ID not specified'), E_USER_WARNING)
			return FALSE;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = $this->getThesaurusRef();
		if (is_null($asset)) {
			trigger_localised_error('HIPO0062', sprintf(translate('Unable to initialise "Import Thesaurus XML" HIPO, invalid thesaurus asset ID specified (%s)'), $this->_running_vars['thesaurus_id']), E_USER_WARNING)
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
		$o->openSection(translate('Erasing Thesaurus Contents'));

		$o->openField('');
			echo translate('Please confirm that you want to erase the existing thesaurus. Your XML file is ready to be imported. Its contents will replace the existing thesaurus data.');

			$o->addHiddenField('form_submitted', 1);
		$o->closeField();
		$o->closeSection();

		return TRUE;

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
		if (isset($_REQUEST['form_submitted'])) {
			$th_asset = $this->getThesaurusRef();

			if (!$th_asset->erase()) {
				trigger_localised_error('HIPO0063', translate('Unable to Erase Contents of the Thesaurus'), E_USER_WARNING)
				return FALSE;
			}

			$step_data['percent_done'] = 100;
			$step_data['complete'] = TRUE;
		}

		return TRUE;

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
		$tagstack =& $this->_running_vars['tagstack'];
		$relations =& $this->_running_vars['relations'];

		$thesaurus = $this->getThesaurusRef();

		if ($tag != 'thesaurus' && $tag != 'term' && $tag != 'relation' && $tag != 'note') {
			trigger_localised_error('HIPO0064', sprintf(translate('Invalid tag type "%1$s"'), $tag), E_USER_WARNING)
		}

		if (empty($stack) && $tag != 'thesaurus') {
			trigger_error('EMPTY STACK');
			return FALSE;
		}

		$current_tag = array_get_index($this->_running_vars, 'current_tag');
		switch ($tag) {
			case 'thesaurus':
				if (!empty($stack)) {
					trigger_localised_error('HIPO0067', sprintf(translate('could not handle tag of type "%1$s"'), $tag), E_USER_WARNING)
					return FALSE;
				}
				array_unshift($stack, Array('term' => NULL));

			break;

			case 'relation':
				if (isset($stack[0]['relation'])) {
					trigger_localised_error('HIPO0065', sprintf(translate('Relations may not have relations as child nodes. Offending child: %1$s'), $tag), E_USER_WARNING)
					return FALSE;
				}

				$rel_name = array_get_index($attrs, 'name');
				$rel_id = array_get_index($relations, $rel_name);
				if (is_null($rel_id)) {
					// add relation to the thesaurus
					$rel_id = $thesaurus->addRelation($rel_name);
					$relations[$rel_name] = $rel_id;
				}

				$stack[0]['relation'] = $rel_id;

			break;

			case 'term':
				if (empty($attrs['name'])) {
					trigger_localised_error('HIPO0066', translate('Terms must have a name attribute'), E_USER_WARNING)
					return FALSE;
				}

				$rel_id = NULL;

				// add the term
				if (is_null($stack[0]['term'])) {
					$parent_asset =& $thesaurus;
				} else {
					$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($stack[0]['term']);
					$rel_id = array_get_index($stack[0], 'relation');
					if (is_null($rel_id)) {
						$NULL_rel_id = array_get_index($relations, NULL);
						// null relation, which hasn't been added yet
						if (is_null($NULL_rel_id)) {
							$rel_id = $thesaurus->addRelation(NULL);
							$relations[NULL] = $rel_id;
						} else {
							$rel_id = $NULL_rel_id;
						}
					}
				}

				$create_link['asset'] =& $parent_asset;
				$create_link['value'] = $rel_id;

				$term_asset = new Thesaurus_Term();
				$term_asset->setAttrValue('name', $attrs['name']);
				$term_asset->create($create_link);

				// put it onto the stack
				array_unshift($stack, Array('term' => $term_asset->id));

			break;

			case 'note':
				if ($tagstack[0] != 'term') {
					trigger_error('NOTE tag can only be a child of TERM tag', E_USER_WARNING);
					return FALSE;
				}

				if (empty($attrs['name'])) {
					trigger_error('Note must have a Name', E_USER_WARNING);
					return FALSE;
				}

				$this->_running_vars['current_note']['name'] = $attrs['name'];

			break;

			default:
				trigger_localised_error('HIPO0067', sprintf(translate('could not handle tag of type "%1$s"'), $tag), E_USER_WARNING)
				return FALSE;

		}//end switch $tag

		array_unshift($tagstack, $tag);

	}//end startElement()


	/**
	* Handler of Character Data blocks
	*
	* Currently being used for notes only
	*
	* @param object	$parser	the calling xml parser
	* @param string	$data	text
	*
	* @return boolean
	* @access public
	*/
	function cDataHandler($parser, $data)
	{
		$stack =& $this->_running_vars['stack'];
		$tagstack =& $this->_running_vars['tagstack'];

		if (isset($tagstack[0]) && $tagstack[0] == 'note') {
			// we do this because for some reason if there are newlines in the character data block,
			// it will be reported as multiple elements
			if (isset($this->_running_vars['current_note']['text'])) {
				$this->_running_vars['current_note']['text'] .= $data;
			} else {
				$this->_running_vars['current_note']['text'] = $data;
			}
			// add note to the current Term
		}

	}//end cDataHandler()


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
		$tagstack =& $this->_running_vars['tagstack'];
		$relations =& $this->_running_vars['relations'];

		if ($tag != 'thesaurus' && $tag != 'term' && $tag != 'relation' && $tag != 'note') {
			trigger_localised_error('HIPO0064', sprintf(translate('Invalid tag type "%1$s"'), $tag), E_USER_WARNING)
		}

		if (empty($stack)) {
			trigger_error('EMPTY STACK');
			return FALSE;
		}

		if ($tagstack[0] != $tag) {
			trigger_error('Malformed XML. Cannot close tag <'.$tag.'> while being in <'.$tagstack[0].'>');
			return FALSE;
		}

		switch ($tag) {
			case 'thesaurus':
				if (is_null($stack[0]['term'])) {
					array_shift($stack);
				} else {
					trigger_error('Malformed XML. Unexpected tag: <'.$tag.'>.');
					return FALSE;
				}
			break;

			case 'relation':
				if (!is_null($stack[0]['relation'])) {
					unset($stack[0]['relation']);
				} else {
					trigger_error('Malformed XML. Unexpected tag: <'.$tag.'>.');
					return FALSE;
				}
			break;

			case 'term':
				if (!is_null($stack[0]['term'])) {
					array_shift($stack);
				} else {
					trigger_error('Malformed XML. Unexpected tag: <'.$tag.'>.');
					return FALSE;
				}
			break;

			case 'note':
				$note_name = $this->_running_vars['current_note']['name'];
				$note_value = $this->_running_vars['current_note']['text'];
				$term = $GLOBALS['SQ_SYSTEM']->am->getAsset($stack[0]['term']);
				$term->addTermNote($note_name, $note_value);
				unset($this->_running_vars['current_note']);

			break;

			default:
				trigger_localised_error('HIPO0067', sprintf(translate('could not handle tag of type "%1$s"'), $tag), E_USER_WARNING)
				return FALSE;
		}//end switch

		array_shift($tagstack);

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
			// assume this is the first step because we must read something on each step
			$step_data['total_parsed'] = 0;
			$step_data['total_read'] = 0;
			$step_data['remainder'] = '';
		}

		if (!isset($this->_running_vars['stack'])) {
			$this->_running_vars['stack'] = Array();
		}

		if (!isset($this->_running_vars['tagstack'])) {
			$this->_running_vars['tagstack'] = Array();
		}

		if (!isset($this->_running_vars['relations'])) {
			$this->_running_vars['relations'] = Array();
		}


		$th_asset = $this->getThesaurusRef();

		// use  DOMDocument::load() to  see if there  is any entity  reference loop
		// which may exhaust us of the memory limit, if not then continue as normal
		$xml = new DOMDocument();
		if (!$xml->load($th_asset->getXmlFilePath(), LIBXML_NOENT)) return FALSE;

		$xml_parser = xml_parser_create();
		// stop the parser ucasing everything
		xml_parser_set_option ($xml_parser, XML_OPTION_CASE_FOLDING, 0);


		// initialise the file pointer
		$import_path = $th_asset->getXmlFilePath();

		if (!($fp = fopen($import_path, 'r'))) {
			trigger_localised_error('CORE0257', sprintf(translate('Could not open XML input file "%1$s"'), $import_path), E_USER_WARNING)
			return FALSE;
			die();
		}

		$fstat = fstat($fp);
		$file_size = $fstat['size'];

		if ($step_data['total_read'] > 0) {
			// we have to parse the seen portion of the file with the tag handlers unset to recreate the parser state. Sad but true.
			$data = fread($fp, $step_data['total_read']);
			if (!xml_parse($xml_parser, $data, feof($fp))) {
				trigger_localised_error('CORE0258', sprintf(translate('XML error: "%1$s" for chunk "%2$s"'), xml_error_string(xml_get_error_code($xml_parser)), htmlentities($data, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)), E_USER_WARNING)
				return FALSE;
			}
		}

		xml_set_element_handler($xml_parser, Array(&$this,'startElement'), Array(&$this,'endElement'));
		xml_set_character_data_handler($xml_parser, Array(&$this,'cDataHandler'));

		$blocks = 1;
		while (!feof($fp) && $blocks <= $blocks_per_step) {

			// parse blocks from the file
			$data = fread($fp, $block_size);
			if (!xml_parse($xml_parser, $data, feof($fp))) {
				trigger_localised_error('CORE0258', sprintf(translate('XML error: "%1$s" for chunk "%2$s"'), xml_error_string(xml_get_error_code($xml_parser)), htmlentities($data, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)), E_USER_WARNING)
				return FALSE;
			}

			$parsed = xml_get_current_byte_index($xml_parser);

			$step_data['remainder'] = substr($data, $parsed);
			$step_data['total_read'] = $step_data['total_read'] + strlen($data);
			$step_data['total_parsed'] = $step_data['total_parsed'] + $parsed;
			$blocks++;
		}


		$step_data['percent_done'] = ($step_data['total_read']/$file_size) * 100;

		if (feof($fp)) {
			// unset all running vars we are now finished with
			$step_data['complete'] = TRUE;
			xml_parser_free($xml_parser);
		} else {
			$step_data['complete'] = FALSE;
		}
		$step_data['message']  = 'Parsed '.sprintf('%01.2f',$step_data['total_read']/1024).' Kb of '.sprintf('%01.2f',$file_size/1024).' Kb';
		fclose($fp);
		return TRUE;

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

		$options['auto_complete'] = TRUE;

		$running_vars = Array(
							'assetids'			=> Array($this->_running_vars['thesaurus_id'],),
							'lock_type'			=> 'attributes',
							'dependants_only'	=> FALSE,
							'forceably_acquire'	=> FALSE,
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
		$thes = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['thesaurus_id']);
		return $thes;

	}//end getThesaurusRef()


}//end class
?>
