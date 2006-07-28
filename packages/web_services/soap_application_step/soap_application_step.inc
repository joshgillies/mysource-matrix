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
* $Id: soap_application_step.inc,v 1.16 2006/07/28 03:33:03 tbarrett Exp $
*
*/


define('SQ_SOAP_APP_TYPE_KEYWORD_PREFIX',  'wsdl_type');
define('SQ_SOAP_APP_VALUE_KEYWORD_PREFIX', 'wsdl_value');

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* SOAP_Application_Step
*
* TODO: Describe
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Application_Step extends Asset
{


	function SOAP_Application_Step($assetid=0)
	{
		$this->Asset($assetid);

	}//end SOAP_Application_Step()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* This will create the following asset hierarchy upon creation (asset types
	* in square brackets):
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// set an initial web path
		$initial_path = strtolower($this->attr('name'));
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
		if (!$this->saveWebPaths($good_paths)) return FALSE;

		// note that any further bodycopies need to have 'exclusive' set to FALSE, and the
		// link value to the name of the bodycopy
		$sub_assets = Array(
						Array(
							'type_code'		=> 'bodycopy',
							'name'			=> 'Step Bodycopy',
							'link_type'		=> SQ_LINK_TYPE_2,
							'dependant'		=> TRUE,
							'exclusive'		=> FALSE,
							'content'		=> Array('content' => 'Step content has not been customised: %asset_assetid%'),
							'link_value'	=> 'bodycopy',
						),
						Array(
							'type_code'		=> 'bodycopy',
							'name'			=> 'Fault Bodycopy',
							'link_type'		=> SQ_LINK_TYPE_2,
							'dependant'		=> TRUE,
							'exclusive'		=> FALSE,
							'content'		=> Array('content' => 'A fault has occurred.'),
							'link_value'	=> 'fault',
						),
					  );

		$success = TRUE;

		foreach ($sub_assets as $sub_asset) {
			$asset_type = $sub_asset['type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);

			$asset =& new $asset_type();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> array_get_index($sub_asset, 'link_value'),
							'link_type'		=> $sub_asset['link_type'],
							'is_dependant'	=> array_get_index($sub_asset, 'dependant', FALSE),
							'is_exclusive'	=> $sub_asset['exclusive'],
						 );

			$asset->setAttrValue('name', $sub_asset['name']);

			$success = $asset->create($copy_link, array_get_index($sub_asset, 'content'));


			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);

			if (!$success) break;
		}

		return $success;

	}//end _createAdditional()


	function setApplication(&$asset)
	{
		$this->_app = $asset;

	}


	function &getApplication()
	{
		if (empty($this->_app)) {
			$app_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'soap_application', FALSE, 'minor', 'step');

			if (!empty($app_links)) {
				$app_link = array_shift($app_links);
				$this->_app =& $GLOBALS['SQ_SYSTEM']->am->getAsset($app_link['majorid']);
			} else {
				$this->_app = NULL;
			}
		}

		return $this->_app;

	}


	/**
	* Create this asset
	*
	* The return value will be:<br/>
	* <ul>
	*   <li>FALSE if the asset was not created</li>
	*   <li>the ID of the newly created link if the asset and intital link were created</li>
	* </ul>
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </pre>
	*
	* @return mixed int|boolean
	* @access public
	*/
	function create(&$link)
	{
		return parent::create($link);

	}//end create()


	/**
	* Returns name of the forum
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	function paintInterface()
	{
		// get the application we belong to (it will answer questions about other steps)
		$app = $this->getApplication();

		// find the previous step

		// get data from previous step

		// set data to own body, provide keyword replacements, etc

		// print
		$this->_printBodycopy();

	}


	function validateInput()
	{
		// ask the form to process itself and prepare the data
		$formid = $this->getFormId();
		if (!empty($formid)) {
			$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($formid, 'simple_form');
			if (!empty($form)) {
				return $form->process();
			}
		}
		return TRUE;

	}


	function performOperation()
	{
		// construct input message
		$input_message =& $this->getInputMessage();
		$output_message = NULL;

		// get the SOAP processor to talk to the remote system
		$op_assetid = $this->attr('soap_operation_asset');
		if (!empty($op_assetid)) {
			$op =& $GLOBALS['SQ_SYSTEM']->am->getAsset($op_assetid);

			$error_status = $op->process($input_message, $output_message);

			if (!is_null($error_status)) {
				// transport failure, could not get valid
				$this->_tmp['transport_error'] = $error_status;
				return FALSE;
			}

			if ($output_message->isFault()) {
				// mark our state as faulty, so we know to print the fault bodycopy
				// TODO: Replace this with the error message grabbed from the fault
				$this->_tmp['op_fault'] =& $output_message;
				return FALSE;
			}
			$this->_tmp['op_result'] = $output_message;

			return TRUE;
		}
		// if no operation, processing is successful

		return TRUE;

	}//end performOperation()


	function &getOutputMessage()
	{
		$output_message = array_get_index($this->_tmp, 'op_result');
		return $output_message;
	}


	function &getPreviousStepOutputMessage()
	{
		$output_message = NULL;

		$app = $this->getApplication();
//		log_dump($app);
		$prev_step_assetid = $app->getPreviousStepId();
		if (!is_null($prev_step_assetid)) {
			log_dump('step found');
			$prev_step_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($prev_step_assetid);
			$output_message = $prev_step_asset->getOutputMessage();
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($prev_step_asset);
		} else {
			log_dump('step not found');
		}

		log_dump($output_message);

		return $output_message;

	}


	/**
	* Print the main bodycopy
	*
	* Provided that the context has already been determined, this function simply find an appropriate
	* bodycopy and prints it, providing it with keywords
	*
	* @return void
	* @access public
	*/
	function _printBodycopy()
	{
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'bodycopy');
		if (empty($bodycopy_link)) {
			trigger_error('Main bodycopy not found', E_USER_WARNING);
			return;
		}
		$content_bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		if (is_null($content_bodycopy)) {
			trigger_error('Main bodycopy not found', E_USER_WARNING);
			return;
		}

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = $content_bodycopy->getKeywords();
		$replacements = Array();
		foreach ($keywords as $keyword) {
			$replacements[$keyword] = $this->getKeywordReplacement($keyword);
		}

		$content_bodycopy->setKeywordReplacements($replacements);
		$content_bodycopy->printBody();

	}//end _printBodycopy()



	/**
	* Get the ID of the simple form linked to this asset, if any
	*
	* @return int
	* @access private
	*/
	function getFormId()
	{
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'simple_form', TRUE, 'form');
		if (empty($form_link)) {
			return 0;
		} else {
			return $form_link['minorid'];
		}

	}//end getFormId()


	/**
	* Add a simple form linked under this asset
	*
	* @return void
	* @access public
	*/
	function addForm()
	{
		// TODO: Only allow one form to be added. Hide the 'Add' interface if form exists (in edit fns)
		$formid = $this->getFormId();
		if (empty($formid)) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('simple_form');
			$form =& new Simple_Form();
			$form->setAttrValue('name', 'Form');
			$link = Array(
						'asset'			=> &$this,
						'link_type'		=> SQ_LINK_TYPE_2,
						'sort_order'	=> -1,
						'value'			=> 'form',
					);
			$form->create($link);
		}

	}//end addForm()


	/**
	* Get the replacement for the form_contents keyword
	*
	* @return string
	* @access public
	*/
	function getFormContentsKeywordReplacement()
	{
		$formid = $this->getFormId();
		if (empty($formid)) return '';
		$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($formid, 'simple_form');
		ob_start();
			$form->printBody();
			$res = ob_get_contents();
		ob_end_clean();
		return $res;

	}//end getFormContentsKeywordReplacement()


	/**
	* Get the replacement for the form_contents keyword
	*
	* @return string
	* @access public
	*/
	function getFaultDetailsKeywordReplacement()
	{
		if (empty($this->_tmp['op_fault'])) {
			return '';
		}

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'fault');
		if (empty($bodycopy_link)) {
			return '';
		}

		$bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], 'bodycopy');
		if (is_null($bodycopy)) {
			return '';
		}

		ob_start();
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords = $bodycopy->getKeywords();
			bam($keywords);
			$replacements = Array();
			foreach ($keywords as $keyword) {
				$replacements[$keyword] = $this->getKeywordReplacement($keyword);
			}
			$bodycopy->setKeywordReplacements($replacements);
			$bodycopy->printBody();
			$res = ob_get_contents();
		ob_end_clean();
		return $res;

	}//end getFaultDetailsKeywordReplacement()


	/**
	* Get the replacement for the transport error keyword
	*
	* @return string
	* @access public
	*/
	function getTransportErrorKeywordReplacement()
	{
		$err = array_get_index($this->_tmp, 'transport_error', '');
		return empty($err) ? '' : 'Error when calling SOAP service: '.$err;

	}//end getFaultDetailsKeywordReplacement()


	/**
	* Get the replacement for the fault message (used within the fault bodycopy)
	*
	* @return string
	* @access public
	*/
	function getFaultMessageKeywordReplacement()
	{
		
		if (empty($this->_tmp['op_fault'])) {
			return 'No msg';
		}
		// TODO:  represent this message properly
		return '<pre>'.print_r($this->_tmp['op_fault'], 1).'</pre>';

	}//end getFaultMessageKeywordReplacement()


	/**
	* Print the body of this asset on the frontend
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		trigger_error('SOAP Application Steps must be accessed via their application parent rather than directly', E_USER_WARNING);

	}//end printBody()


	/**
	* Get the data fields for this app
	*
	* @return void
	* @access public
	*/
	function getDataFields()
	{
		$op_assetid = $this->attr('soap_operation_asset');
		if (empty($op_assetid)) {
			return Array();
		}
		$op_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($op_assetid);
		if (is_null($op_asset)) {
			return Array();
		}
		$input_message =& $op_asset->getInputMessageAsset();
		if (is_null($input_message)) {
			return Array();
		}
		$res = $input_message->getArgs();
		foreach ($res as $i => $v) {
			$res[$i]['question_type'] = 'form_question_type_text'; // HACK
		}
		return $res;

	}//end getDataFields()


	function getDataFieldQuestionValue($code_name)
	{
		$formid = $this->getFormId();
		$questionid = $this->getFormQuestionMapping($code_name);
		if ($formid) {
			$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($formid, 'simple_form');
			$answers = $form->getAnswers();
			if (isset($answers[$questionid])) {
				return $answers[$questionid]['value'];
			}
		}
		return NULL;

	}//end getDataFieldQuestionValue()


	/**
	* Get input message
	*
	* @return void
	* @access public
	*/
	function &getInputMessage()
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_message');
		$empty_message =& new SOAP_WSDL_Message();
		$op_assetid = $this->attr('soap_operation_asset');
		if (empty($op_assetid)) {
			return $empty_message;
		}
		$op_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($op_assetid);
		if (is_null($op_asset)) {
			return $empty_message;
		}
		$input_message =& $op_asset->getInputMessageAsset();
		if (is_null($input_message)) {
			return $empty_message;
		}
		foreach ($input_message->getArgs() as $name => $details) {
			$input_message->setArgValue($name, $this->getDataFieldQuestionValue($name));
		}
		return $input_message;

	}//end getInputMessage()


	/**
	* Set the form question to be mapped to a particular data field
	*
	* @param string	$code_name		The code name of the data field
	* @param string	$question_id	The ID of the question asset
	*
	* @return void
	* @access public
	*/
	function setFormQuestionMapping($code_name, $question_id)
	{
		$existing_link = $GLOBALS['SQ_SYSTEM']->am->getShadowLinkByAsset($this->id, $question_id, SQ_LINK_NOTICE, NULL, 'major', FALSE);
		// note:  the above call would ideally have an additional condition that the link value be 'data_mapping_'.$code_name, but we need to work with the fact that there can't be more than one shadow link between any pair of assets.
		if (!empty($existing_link)) {
			if ($existing_link['minorid'] == $question_id) {
				// link already exists
				return;
			} else {
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($existing_link['linkid']);
			}
		}
		$question =& $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id);
		if (!is_null($question)) {
			$GLOBALS['SQ_SYSTEM']->am->createAssetLink($this, $question, SQ_LINK_NOTICE, 'data_mapping_'.$code_name);
		}


	}//end setFormQuestionMapping()


	/**
	* Get all the form question mappings
	*
	* @return array
	* @access public
	*/
	function getFormQuestionMappings()
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getShadowLinkByAsset($this->id, NULL, SQ_LINK_NOTICE, NULL, 'major', TRUE);
		$mappings = Array();
		foreach ($links as $link) {
			if (0 === strpos($link['value'], 'data_mapping_')) {
				$mappings[substr($link['value'], 13)] = $link['minorid'];
			}
		}
		return $mappings;

	}//end getFormQuestionMappings()


	/**
	* Get the form question mapping for the supplied code name
	*
	* @return array
	* @access public
	*/
	function getFormQuestionMapping($code_name)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getShadowLinkByAsset($this->id, NULL, SQ_LINK_NOTICE, 'data_mapping_'.$code_name, 'major', FALSE);
		if (empty($link)) {
			return NULL;
		} else {
			return $link['minorid'];
		}

	}//end getFormQuestionMappings()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		/*
			Add an initial check:
			if (preg_match('/^'.SQ_SOAP_APP_TYPE_KEYWORD_PREFIX.'_step1-a-b-c')
			etc.
		*/

		// Bodycopies have link values that denote what level they are on, on who their parents are.
		// Relations are denoted with '-', so if, for example, Bodycopy A has a bodycopy 'underneath'
		// it called B. Thus, the link value for A will be 'A', and B 'A-B'.
		// Note that the second bodycopy isn't represented by the asset map, only by the link values
		// (and, presumably, name).
		//
		// Bodycopies have keywords in the format '%wsdl_type-A-B%' (an absolute reference to the A-B bodycopy)

		$res_failed = "%$keyword%";

		$res = parent::getKeywordReplacement($keyword);
		if ($res != $res_failed) return $res;

		// check if $this->_tmp already contains a cached-out copy of the bodycopy links
		if (isset($this->_tmp['bodycopy_links'])) {
			$bodycopy_links =& $this->_tmp['bodycopy_links'];
		} else {
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE);
			foreach ($links as $link) {
				// ignore the main bodycopy, it's always presumed to exist
				if ($link['value'] != 'bodycopy') {
					$bodycopy_links[$link['value']] = $link['minorid'];
				}
			}
		}

		if (!isset($this->_tmp['input_data'])) {
			$this->_tmp['input_data'] = $this->getPreviousStepOutputMessage();
		}
		$input_data = get_object_vars($this->_tmp['input_data']);


		// work out what the type id is
		$type_matches = Array();
		preg_match('/^'.SQ_SOAP_APP_TYPE_KEYWORD_PREFIX.'-(.+)/', $keyword, $type_matches);
		$wsdl_type = array_get_index($type_matches, 1);

		$value_matches = Array();
		preg_match('/^'.SQ_SOAP_APP_VALUE_KEYWORD_PREFIX.'-(.+)/', $keyword, $value_matches);
		$wsdl_value = array_get_index($value_matches, 1);

		// TODO: ========================================
		// TODO: === Unable to handle arrays of types ===
		// TODO: ========================================

		// grab immediately if primitive, otherwise interrogate the bodycopies
		if (!empty($wsdl_value)) {
			// TODO: Remove - Dummy Data
			/*
			$next_step_input = Array(
				'response' =>
				Object (
					'resulttype' = string,
					'searchtype' = string,
					'foliotypein' = string,
					'foliotypeout' = string,
					'nooflines' = int,
					'pages' = Array(
						Object (
							line = Array(string)
						),
					),
				),
			);
			*/

			// does the value exist?
			if (!isset($input_data[$wsdl_value])) {
				return $res_failed;
			}

			// check the type
			if (is_object($input_data[$wsdl_value])) {
				// can't handle objects
				// TODO: localise, write better error message
				trigger_error('Soap Application unable to display object', E_USER_WARNING);
				return $res_failed;
			}

			if (is_array($input_data[$wsdl_value])) {
				// TODO: ===================================================
				// TODO: ===== Horrible, nasty hack. Fix it with fire. =====
				// TODO: ===================================================
				if ($wsdl_value == 'pages') {
					$res = $this->_getPagesKeywordReplace($keyword, $input_data[$wsdl_value], $bodycopy_links['lines']);
					if (!$res) {
						return $res_failed;
					}
					return $res;
				}

			} else {
				// just return the value
				return $input_data[$wsdl_value];
			}

		} else if (!empty($wsdl_type)) {

			// not there, look up bodycopy based on keyword, get bodycopy to print itself
			$content_bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_links[$wsdl_type]);
			if (is_null($content_bodycopy)) {
				// TODO: Localise
				trigger_error('Step bodycopy "'.$wsdl_type.'" not found', E_USER_WARNING);
				return;
			}

			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords = $content_bodycopy->getKeywords();
			$replacements = Array();
			foreach ($keywords as $keyword) {
				$replacements[$keyword] = $this->getKeywordReplacement($keyword);
			}

			$content_bodycopy->setKeywordReplacements($replacements);
			ob_start();
				$content_bodycopy->printBody();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_bodycopy);
			return ob_get_clean();

		} else {
			// no match, bug out
			return $res_failed;
		}


		return $res_failed;

	}//end getKeywordReplacement()


	function _getPagesKeywordReplace($keyword, $lines_array, $bodycopy_assetid)
	{
		$content_bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_assetid);
		if (is_null($content_bodycopy)) {
			// TODO: Localise
			trigger_error('Step bodycopy not found', E_USER_WARNING);
			return FALSE;
		}

		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$output = '';
		foreach ($lines_array as $current_line) {
			$replacements = Array(
								$keyword	=> $current_line,
							);
			$content_bodycopy->setKeywordReplacements($replacements);
			ob_start();
				$content_bodycopy->printBody();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_bodycopy);
			$output .= ob_get_clean();
		}

		return $output;

	}//end getPagesKeywordReplace()



	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$vars['keywords'] = isset($vars['keywords']) ? $vars['keywords'] : Array();
		$requester_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$bc_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE);
		$requester_name = '';
		foreach ($bc_links as $bc_link) {
			if (isset($requester_parents[$bc_link['minorid']])) {
				$requester_name = $bc_link['value'];
				break;
			}
		}
		$vars['keywords'] += $this->getAvailableKeywords($requester_name);

	}//end onRequestKeywords()


	/**
	* 
	*
	* @param boolean	$varname
	*
	* @return void
	* @access public
	*/
	function getAvailableKeywords($requester_name='')
	{
		if (empty($requester_name)) return parent::getAvailableKeywords();
		$res = Array();
		switch ($requester_name) {
			case 'bodycopy':
				// main BC
				$res['fault_details'] = 'Fault Details';
				if ($this->getFormId()) {
					$res['form_contents'] = 'Form Contents'; // only if form present
				}
				$res['transport_error'] = 'Details of any error that occurred while calling the soap service';
			break;

			case 'fault':
				// fault BC
				$res['fault_message'] = 'Fault message';
			break;
		}
		return $res;

	}//end getAvailableKeywords()


}//end class
?>