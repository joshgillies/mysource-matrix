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
* $Id: soap_application.inc,v 1.16 2006/08/14 05:13:38 arailean Exp $
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
* @version $Revision: 1.16 $
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

		// init the session, to avoid extra checks in other places
		if (!isset($_SESSION['asset'][$assetid])) {
			$_SESSION['asset'][$assetid] = Array();
		}

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


	/**
	* Get the details of steps that should be created for this application automatically
	*
	* @return array
	* @access protected
	*/
	function _getInitialStepDefinitions()
	{
		return Array(
				Array(
					'type_code'		=> 'soap_application_step',
					'name'			=> 'First Step',
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

		$current_step =& $this->getCurrentStep();

		if (is_null($current_step)) {
			$this->setFirstStep();
		} else {
			if ($current_step->validateInput() && $current_step->performOperation()) {
				$this->setNextStep();
			}
		}

	}//end _preProcessInterface()


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
		return $this->attr(($short_name?'short_':'').'name');

	}//end _getName()


	/**
	* Get the assetid of the first step in this application
	*
	* @return int
	* @access public
	*/
	function getFirstStepId()
	{
		$first_step = array_get_index($this->_steps, SQ_SOAP_APP_STEP_1);

		return $first_step;

	}//end getFirstStepId()


	/**
	* Set the first step as the current step
	*
	* @return void
	* @access public
	*/
	function setFirstStep()
	{
		if (isset($this->_steps[SQ_SOAP_APP_STEP_1])) {
			$_SESSION['asset'][$this->id]['current_step_no'] = SQ_SOAP_APP_STEP_1;
		}

	}//end setFirstStep()


	/**
	* Go to the next step
	*
	* @return void
	* @access public
	*/
	function setNextStep()
	{
		$current_step = $this->getCurrentStepNo();
		$next_step = $current_step + 1;
		if (isset($this->_steps[$next_step])) {
			$_SESSION['asset'][$this->id]['current_step_no'] = $next_step;
		} else {
			// if we're past the last step, first step is the next step 8)
			$_SESSION['asset'][$this->id]['current_step_no'] = SQ_SOAP_APP_STEP_1;
		}


	}//end setNextStep()


	/**
	* Get the number of the current step
	*
	* @return int
	* @access public
	*/
	function getCurrentStepNo()
	{
		$current_step_no = array_get_index($_SESSION['asset'][$this->id], 'current_step_no');
		return $current_step_no;

	}//end getCurrentStepNo()


	/**
	* Get the current step asset
	*
	* @return object
	* @access public
	*/
	function &getCurrentStep()
	{
		$current_step = NULL;

		$current_step_no = $this->getCurrentStepNo();

		$current_step_id = array_get_index($this->_steps, $current_step_no);

		if (!is_null($current_step_id)) {
			$current_step =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_step_id);
		}

		return $current_step;

	}//end getCurrentStep()


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

	}//end getStepDataKeywordReplacement()


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


	function getStepIdByNo($step_no=NULL)
	{
		$step_id = array_get_index($this->_steps, $step_no);

		return $step_id;

	}


	function getPreviousStepId()
	{
		$previous_step_assetid = NULL;

		$current_step_no = $this->getCurrentStepNo();

		$prev_step_no = $current_step_no - 1;

		if ($prev_step_no < SQ_SOAP_APP_STEP_1) {
			return NULL;
		}

		$previous_step_assetid = $this->getStepIdByNo($prev_step_no);

		return $previous_step_assetid;

	}


}//end class

?>
