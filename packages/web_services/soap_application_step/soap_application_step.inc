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
* $Id: soap_application_step.inc,v 1.2 2006/07/23 23:22:09 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* SOAP_Application_Step
*
* TODO: Describe
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Application_Step extends Asset
{


	function SOAP_Application_Step($assetid=0)
	{
		parent::Asset($assetid);
	}


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

		$sub_assets = Array(
						Array(
							'type_code'		=> 'bodycopy',
							'name'			=> 'Step Bodycopy',
							'link_type'		=> SQ_LINK_TYPE_2,
							'dependant'		=> TRUE,
							'exclusive'		=> TRUE,
							'content'		=> Array('content' => 'Step content has not been customised: %assetid%'),
							'link_value'	=> 'bodycopy'
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

		return $success;

	}//end _createAdditional()


	function setApplication(&$asset)
	{
		$this->_app = $asset;

	}


	function getApplication()
	{
		if (empty($this->_app)) {
			$app_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'soap_application', TRUE, 'minor', 'step');

			if (!empty($app_links)) {
				$app_link = array_shift($app_links);
				$this->_app = $GLOBALS['SQ_SYSTEM']->am->getAsset($app_link['majorid']);
			} else {
				$this->_app = null;
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
		// get the SOAP processor to talk to the remote system

		return TRUE;
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
	* Get the keywords that can be replaced by this asset
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$res = parent::getAvailableKeywords();
		$res['form_contents'] = 'Form Contents';
		return $res;

	}//end getAvailableKeywords()


}//end class
?>