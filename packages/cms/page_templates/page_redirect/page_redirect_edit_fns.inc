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
* $Id: page_redirect_edit_fns.inc,v 1.17 2006/01/20 03:22:29 sdanis Exp $
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
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Redirect_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Redirect_Edit_Fns()
	{
		$this->Page_Edit_Fns();
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
		$redirect_asset =& $asset->getRedirectAsset();
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
		if (!isset($_POST[$prefix.'_redirect_assetid']['assetid'])) {
			return TRUE;
		}

		$redirect_assetid = $_POST[$prefix.'_redirect_assetid']['assetid'];

		if ($redirect_assetid) {
			$redirect_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($redirect_assetid);
		} else {
			$redirect_asset = NULL;
		}

		return $asset->setRedirectAsset($redirect_asset);

	}//end processRedirectAssetid()


}//end class

?>