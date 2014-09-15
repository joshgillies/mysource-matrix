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
* $Id: page_redirect_edit_fns.inc,v 1.22 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Redirect_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Redirect_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['lock_type'] = 'redirect';

	}//end constructor


	/**
	* Paint the interface for selecting the asset to redirect to
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRedirectAssetid(&$asset, &$o, $prefix)
	{
		$redirect_asset = $asset->getRedirectAsset();
		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_redirect_assetid', (is_null($redirect_asset) ? 0 : $redirect_asset->id));
		} else {
			if (is_null($redirect_asset)) {
				echo translate('cms_redirect_none_set');
			} else {
				echo get_asset_tag_line($redirect_asset->id);
			}
		}
		return TRUE;

	}//end paintRedirectAssetid()


	/**
	* Process the interface for selecting the asset to redirect to
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRedirectAssetid(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_redirect_assetid']['assetid']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version'])) {
			return TRUE;
		}

		$redirect_assetid = $_POST[$prefix.'_redirect_assetid']['assetid'];

		if ($redirect_assetid) {
			$redirect_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($redirect_assetid);
		} else {
			$redirect_asset = NULL;
		}

		return $asset->setRedirectAsset($redirect_asset);

	}//end processRedirectAssetid()


	/**
	* Paint the interface for selecting the session variables
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSessionVariables(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$pmap = $asset->getAttribute('session_variables');
		$current_params = $pmap->_params;
		// Add in the variable names
		$fields = $asset->attr('session_vars');
		if (!empty($fields)) {
			$vars = explode("\n", $fields);
			foreach ($vars as $value) {
				$index = str_replace(' ', '_', $value);
				$index = rtrim($index, "\r");
				$current_params['parameters'][$index] = 'Session Variable: '.$value;
			}//end foreach
		}//end if
		$pmap->_params = $current_params;
		if ($wa) {
			$pmap->paint($prefix);
		} else {
			$pmap->paint($prefix, TRUE);
		}
		return TRUE;

	}//end paintSessionVariables()


	/**
	* Process the interface for selecting the session variables
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSessionVariables(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$pmap = $asset->getAttribute('session_variables');
		if ($wa) {
			$pmap->process($prefix);
			$asset->setAttrValue('session_variables', $pmap->value);
			$asset->saveAttributes();
		}//end if

		return TRUE;

	}//end processSessionVariables()


}//end class

?>
