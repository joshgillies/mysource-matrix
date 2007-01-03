<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: soap_application.inc,v 1.21 2007/01/03 03:15:57 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* SOAP Application
*
* TODO: Describe
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Application extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return object
	* @access public
	*/
	function SOAP_Application($assetid=0)
	{
		parent::Asset($assetid);

	}//end constructor


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
	* Perform any additional processing required during the creation of this asset
	*
	* This will create the following asset hierarchy upon creation (asset types
	* in square brackets):
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
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

		$default_bodycopy = translate('soap_app_default_contents');

		$main_bc_data = Array(
							'type_code'		=> 'bodycopy',
							'name'			=> 'Main Bodycopy',
							'link_type'		=> SQ_LINK_TYPE_2,
							'dependant'		=> TRUE,
							'exclusive'		=> TRUE,
							'content'		=> Array('content' => $default_bodycopy),
							'link_value'	=> 'bodycopy',
						);
		$sub_assets = array_merge(Array($main_bc_data), $this->_getInitialStepDefinitions());

		$success = TRUE;

		foreach ($sub_assets as $sub_asset) {
			$asset_type = $sub_asset['type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);

			$asset =& new $asset_type();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> array_get_index($sub_asset, 'link_value'),
							'link_type'		=> array_get_index($sub_asset, 'link_type', SQ_LINK_TYPE_2),
							'is_dependant'	=> array_get_index($sub_asset, 'dependant', FALSE),
							'is_exclusive'	=> array_get_index($sub_asset, 'exclusive', FALSE),
						 );

			$asset->setAttrValue('name', $sub_asset['name']);

			$success = $asset->create($copy_link, array_get_index($sub_asset, 'content'));


			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);

			if (!$success) break;
		}
		return $success;

	}//end _createAdditional()


	/**
	* Get the details of steps that should be created for this application automatically
	*
	* @return array
	* @access protected
	*/
	function _getInitialStepDefinitions()
	{
		// basic soap application has no steps
		return Array();

	}//end _getInitialStepDefinitions()


	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$this->_loadSteps();

		$this->_preProcessInterface();

		$step =& $this->getCurrentStep();
		if (empty($step)) return parent::printFrontend();

		// Find the design applied to the current step and use that
		$step_url = strip_url($step->getUrl(), TRUE);
		$design_info = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($step_url, 'design::system::frontend');
		if (!empty($design_info['designid'])) {
			$design =& $GLOBALS['SQ_SYSTEM']->am->getAsset($design_info['designid'], $design_info['type_code']);
		} else {
			$design = NULL;
		}
		$this->printFrontendAsset($this, $design);

	}//end printFrontend()


	/**
	* Process the interface
	*
	* @return boolean
	* @access public
	*/
	function _preProcessInterface()
	{
		if (!isset($_REQUEST['s'])) return TRUE;

		$current_step =& $this->getCurrentStep();
		if (!$current_step->validateInput()) return FALSE;
		if (!$this->executeStepOperation()) return FALSE;

		$this->setNextStep();

	}//end _preProcessInterface()


	/**
	* Process the interface
	*
	* @return boolean
	* @access public
	*/
	function executeStepOperation()
	{
		$current_step =& $this->getCurrentStep();
		$result = $current_step->performOperation();
		return $result;

	}//end executeStepOperation()


	/**
	* Called by the design to print the body of this asset
	*
	* Finds the correct main bodycopy, replaces its keywords, and prints it
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$current_step =& $this->getCurrentStep();
		if (!is_null($current_step)) {
			if (!$current_step->attr('use_application_layout')) {
				$current_step->paintInterface();
				return;
			}
		}

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'bodycopy');
		$content_bodycopy =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		if (is_null($content_bodycopy)) {
			// TODO: localise
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

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_bodycopy);

	}//end printBody()


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
		return $this->attr(($short_name ? 'short_' : '').'name');

	}//end _getName()


	/**
	* Go to the next step
	*
	* @return void
	* @access public
	*/
	function setNextStep()
	{
		$next_step_assetid = $this->_getNextStepAssetId();
		$_REQUEST['s'] = $next_step_assetid;

	}//end setNextStep()


	/**
	* Load the step data
	*
	* @return void
	* @access private
	*/
	function _loadSteps()
	{
		$step_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'soap_application_step', FALSE, 'major', 'step');

		$steps = Array();
		foreach ($step_links as $link) {
			$steps[$link['sort_order']] = $link['minorid'];
		}
		ksort($steps);

		$steps = array_values($steps);
		$this->_steps = $steps;

		return TRUE;

	}//end _loadSteps()


	/**
	* Get the asset id of the previous step
	*
	* @return string
	* @access private
	*/
	function _getPreviousStepAssetId()
	{
		$step_num = $this->getCurrentStepNo();
		$prev_step_num = $step_num - 1;

		$prev_step_assetid = array_get_index($this->_steps, $prev_step_num);
		if (is_null($prev_step_assetid)) {
			$prev_step_assetid = reset($this->_steps);
		}
		return $prev_step_assetid;

	}//end _getPreviousStepAssetId()


	/**
	* Get the asset id of the current step
	*
	* @return string
	* @access private
	*/
	function _getCurrentStepAssetId()
	{
		$step_assetid = array_get_index($_REQUEST, 's');
		if (is_null($step_assetid)) {
			$step_assetid = reset($this->_steps);
		}
		return $step_assetid;

	}//end _getCurrentStepAssetId()


	/**
	* Get the asset id of the next step
	*
	* @return string
	* @access private
	*/
	function _getNextStepAssetId()
	{
		$step_num = $this->getCurrentStepNo();
		$next_step_num = $step_num + 1;

		$next_step_assetid = array_get_index($this->_steps, $next_step_num);
		if (is_null($next_step_assetid)) {
			$next_step_assetid = reset($this->_steps);
		}
		return $next_step_assetid;

	}//end _getNextStepAssetId()


	/**
	* Returns the number of the previous step
	*
	* @return int
	* @access public
	*/
	function getPreviousStepNo()
	{
		$step_num = 0;
		$step_assetids = array_flip($this->_steps);

		$prev_step_assetid = $this->_getPreviousStepAssetId();
		if (in_array($prev_step_assetid, $this->_steps)) {
			$step_num = array_get_index($step_assetids, $prev_step_assetid);
		}
		return $step_num;

	}//end getPreviousStepNo()


	/**
	* Get the number of the current step
	*
	* @return int
	* @access public
	*/
	function getCurrentStepNo()
	{
		$step_num = 0;
		$step_assetids = array_flip($this->_steps);

		// this is the current step asset
		$step_assetid = $this->_getCurrentStepAssetId();
		if (in_array($step_assetid, $this->_steps)) {
			$step_num = array_get_index($step_assetids, $step_assetid);
		}
		return $step_num;

	}//end getCurrentStepNo()


	/**
	* Returns the number of the next step
	*
	* @return int
	* @access public
	*/
	function getNextStepNo()
	{
		$step_num = 0;
		$step_assetids = array_flip($this->_steps);

		$next_step_assetid = $this->_getNextStepAssetId();
		if (in_array($next_step_assetid, $this->_steps)) {
			$step_num = array_get_index($step_assetids, $next_step_assetid);
		}
		return $step_num;

	}//end getNextStepNo()


	/**
	* Returns a reference to the previous step asset
	*
	* @return object
	* @access public
	*/
	function &getPreviousStep()
	{
		$previous_step_id = $this->_getPreviousStepAssetId();

		$previous_step =& $GLOBALS['SQ_SYSTEM']->am->getAsset($previous_step_id);
		return $previous_step;

	}//end getPreviousStep()


	/**
	* Get the current step asset
	*
	* @return object
	* @access public
	*/
	function &getCurrentStep()
	{
		$current_step_id = $this->_getCurrentStepAssetId();

		$current_step =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_step_id);
		return $current_step;

	}//end getCurrentStep()


	/**
	* Returns a reference to the next step asset
	*
	* @return object
	* @access public
	*/
	function &getNextStep()
	{
		$next_step_id = $this->_getNextStepAssetId();

		$next_step =& $GLOBALS['SQ_SYSTEM']->am->getAsset($next_step_id);
		return $next_step;

	}//end getNextStep()


	/**
	* Get the replacement for the step data keyword
	*
	* @return string
	* @access public
	*/
	function getStepContentKeywordReplacement()
	{
		ob_start();

		$current_step =& $this->getCurrentStep();
		if (is_null($current_step)) {
			trigger_error('Could not determine the step this application is on.', E_USER_WARNING);
		} else {
			$current_step->paintInterface();
		}

		return ob_get_clean();

	}//end getStepContentKeywordReplacement()


	/**
	* Get the replacement for the step name keyword
	*
	* @return string
	* @access public
	*/
	function getStepNameKeywordReplacement()
	{
		$current_step =& $this->getCurrentStep();
		if (!is_null($current_step)) {
			return $current_step->_getName();
		}

	}//end getStepNameKeywordReplacement()


	/**
	* Get the replacement for the step count keyword
	*
	* @return string
	* @access public
	*/
	function getStepCountKeywordReplacement()
	{
		return count($this->_steps);

	}//end getStepCountKeywordReplacement()


	/**
	* Get the replacement for the step number keyword
	*
	* @return string
	* @access public
	*/
	function getCurrentStepNumberKeywordReplacement()
	{
		return $this->getCurrentStepNo() + 1;

	}//end getCurrentStepNumberKeywordReplacement()


	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['step_content'] = "Step Content";
		$keywords['step_count'] = "Step Count";
		$keywords['current_step_number'] = "Current Step Number";
		$keywords['step_name'] = "Step Name";

		return $keywords;

	}

}//end class

?>
