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
* $Id: page_redirect_edit_fns.inc,v 1.11 2003/12/17 04:46:15 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';

/**
* Page_Redirect_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
	* @param object Bodycopy			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRedirectAssetid(&$asset, &$o, $prefix)
	{
		$redirect_asset = &$asset->getRedirectAsset();
		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_redirect_assetid', (is_null($redirect_asset) ? 0 : $redirect_asset->id));
		} else {
			if (is_null($redirect_asset)) {
				echo 'None Set';
			} else {
				echo $redirect_asset->name.' (#'.$redirect_asset->id.')';
			}
		}
		return true;

	}// end paintRedirectAssetid()
	

	/**
	* Process the interface for selecting the asset to redirect to 
	*
	* @param object Bodycopy			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRedirectAssetid(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_redirect_assetid']['assetid'])) return true;

		$redirect_assetid = $_POST[$prefix.'_redirect_assetid']['assetid'];

		if ($redirect_assetid) {
			$redirect_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($redirect_assetid);
		} else {
			$redirect_asset = null;
		}

		return $asset->setRedirectAsset($redirect_asset);

	}// end processRedirectAssetid()
	

}//end class

?>