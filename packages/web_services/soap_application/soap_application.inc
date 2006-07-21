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
* $Id: soap_application.inc,v 1.1 2006/07/21 06:50:15 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

define('SQ_SOAP_APP_STEP_1', 0);

/**
* SOAP Application
*
* TODO: Describe
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Application extends Asset
{


	function SOAP_Application($assetid=0)
	{
		parent::Asset($assetid);

		// init the session, to avoid extra checks in other places
		if (!isset($_SESSION['asset'][$assetid])) {
			$_SESSION['asset'][$assetid] = Array();
		}

	}


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
		if (!parent::_createAdditional($link)) return false;

$default_bodycopy = <<<HEREDOC
Hello, this is a SOAP application. You're on <strong>%step_name%</strong> (%current_step_number%/%step_count%)

<div style="background: #faf; padding: 2em">
%step_data%
</div>
HEREDOC;

		$sub_assets = Array(
						Array(
							'type_code'		=> 'bodycopy',
							'name'			=> 'Main Bodycopy',
							'link_type'		=> SQ_LINK_TYPE_2,
							'dependant'		=> TRUE,
							'exclusive'		=> TRUE,
							'content'		=> Array('content' => $default_bodycopy),
							'link_value'	=> 'bodycopy',
						),

						Array(
							'type_code'		=> 'soap_application_step',
							'name'			=> 'Step 1',
							'link_type'		=> SQ_LINK_TYPE_2,
							'exclusive'		=> TRUE,
							'dependant'		=> TRUE,
							'link_value'	=> 'step'
						),
						Array(
							'type_code'		=> 'soap_application_step',
							'name'			=> 'Second Step',
							'link_type'		=> SQ_LINK_TYPE_2,
							'exclusive'		=> TRUE,
							'dependant'		=> TRUE,
							'link_value'	=> 'step'
						),
					  );

		$success = true;

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

		if ($success) {
			// set an initial web path
			$initial_path = strtolower($this->attr('name'));
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_paths = make_valid_web_paths(Array($initial_path));
			$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
			$success = $this->saveWebPaths($good_paths);
		}

		return $success;

	}//end _createAdditional()


	/**
	* Return an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$_links = Array();

		return $_links;

	}//end _getAllowedLinks()


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
		if ($this->attr('short_name') == '') {
			$this->setAttrValue('short_name', $this->attr('name'));
		}
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
		return $this->attr(($short_name?'short_':'').'name');

	}//end _getName()


	function getCurrentStepId()
	{

		if (!isset($this->_tmp['current_step_id'])) {
			$current_step_id = array_get_index($_SESSION['asset'][$this->id], 'current_step_id');

			if (!is_null($current_step_id)) {
				// work out the first step here

				$this->_tmp['current_step_id'] = $current_step_id;
			} else {
				$this->_tmp['current_step_id'] = NULL;
			}
		}

		return $this->_tmp['current_step_id'];

	}


	function setCurrentStep($step_asset)
	{
		// move current step to previous step
		if (isset($_SESSION['asset'][$this->id]['current_step_id'])) {
			$_SESSION['asset'][$this->id]['previous_step_id'] = $_SESSION['asset'][$this->id]['current_step_id'];
		}
		$_SESSION['asset'][$this->id]['current_step_id'] = $step_asset->id;
		unset($this->_tmp['current_step_id']);
	}


	function setCurrentStepId($step_id)
	{
		// move current step to previous step
		if (isset($_SESSION['asset'][$this->id]['current_step_id'])) {
			$_SESSION['asset'][$this->id]['previous_step_id'] = $_SESSION['asset'][$this->id]['current_step_id'];
		}
		$_SESSION['asset'][$this->id]['current_step_id'] = $step_id;
		unset($this->_tmp['current_step_id']);
	}

	function getFirstStepId()
	{
		// get first step (first child in asset map order)
		$first_step = NULL;

		return $this->_steps[SQ_SOAP_APP_STEP_1];
	}


	function setFirstStep()
	{
		if (isset($this->_steps[SQ_SOAP_APP_STEP_1])) {
			$_SESSION['asset'][$this->id]['current_step_no'] = SQ_SOAP_APP_STEP_1;
		}
	}


	function setNextStep()
	{
		$current_step = $this->getCurrentStepNo();
		$next_step = $current_step + 1;
		if (isset($this->_steps[$next_step])) {
			$_SESSION['asset'][$this->id]['current_step_no'] = $next_step;
		} else {
			$_SESSION['asset'][$this->id]['current_step_no'] = null;
		}
	}


	function getCurrentStepNo()
	{
		$current_step_no = array_get_index($_SESSION['asset'][$this->id], 'current_step_no');

		return $current_step_no;
	}


	function getCurrentStep()
	{
		$current_step = NULL;

		$current_step_no = $this->getCurrentStepNo();

		$current_step_id = array_get_index($this->_steps, $current_step_no);

		if (!is_null($current_step_id)) {
			$current_step =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_step_id);
		}

		return $current_step;
	}


	function _loadSteps()
	{

		$step_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'soap_application_step', TRUE, 'major', 'step');

		$steps = Array();

		foreach($step_links as $link) {
			$steps[$link['sort_order']] = $link['minorid'];
		}

		ksort($steps);
		$steps = array_values($steps);

		$this->_steps = $steps;

		return TRUE;

	}//end _loadSteps()


	function printBody()
	{
		$this->_loadSteps();

		$this->_preProcessInterface();

		$this->_printBodycopy();
	}


	function _preProcessInterface()
	{

		$success = true;

		$current_step = $this->getCurrentStep();

		if (is_null($current_step)) {
			$this->setFirstStep();
		} else {
			if (!$current_step->validateInput()) {
				$success = false;
				$error = $current_step->getError();
			} else {
				if (!$current_step->performOperation()) {
					$success = false;
					$error = $current_step->getError();
				} else {
					$this->setNextStep();
				}
			}
		}

		return $success;
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
		$content_bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		if (is_null($content_bodycopy)) {
			trigger_error('main bodycopy not found', E_USER_WARNING);
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


	function getStepDataKeywordReplacement()
	{
		ob_start();

		$current_step = $this->getCurrentStep();

		if (is_null($current_step)) {
			trigger_error('Could not determine the step this application is on.', E_USER_ERROR);
		} else {
			$current_step->paintInterface();
		}

		return ob_get_clean();
	}


	function getStepNameKeywordReplacement()
	{
		$current_step = $this->getCurrentStep();
		if (!is_null($current_step)) {
			return $current_step->_getName();
		}
	}


	function getStepCountKeywordReplacement()
	{
		return count($this->_steps);
	}


	function getCurrentStepNumberKeywordReplacement()
	{
		return $this->getCurrentStepNo() + 1;
	}


}//end class

?>