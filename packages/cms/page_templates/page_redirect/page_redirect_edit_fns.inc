<?php

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
		if ($asset->writeAccess()) {
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
		if (!isset($_POST[$prefix.'_redirect_assetid'])) return true;

		$redirect_assetid = $_POST[$prefix.'_redirect_assetid'];

		if ($redirect_assetid) {
			$redirect_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($redirect_assetid);
		} else {
			$redirect_asset = null;
		}

		return $asset->setRedirectAsset($redirect_asset);

	}// end processRedirectAssetid()
	

}//end class

?>