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
* $Id: soap_wsdl_schema_edit_fns.inc,v 1.3 2006/12/07 04:06:31 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';


/**
* SOAP_WSDL_Schema_Edit_Fns
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_WSDL_Schema_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function SOAP_WSDL_Schema_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Provide an interface enabling the WSDL cache to be refreshed.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReparseFile(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$wsdl = $asset->attr('wsdl_cache');

		$reparse_flag = $prefix.'_reparse';
		if ($wa && !is_null($wsdl)) {
			check_box($reparse_flag);
			label(' Refresh WSDL Cache', $reparse_flag);
		}
		return $wa;

	}//end paintReparseFile()


	/**
	* Forces the WSDL cache to be rebuilt from the source file.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processReparseFile(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if (!$wa) return FALSE;

		$reparse_flag = $prefix.'_reparse';
		if (isset($_POST[$reparse_flag])) {
			$asset->setAttrValue('wsdl_uri', $asset->attr('wsdl_uri'));
		}
		return TRUE;

	}//end processReparseFile()


}//end class

?>