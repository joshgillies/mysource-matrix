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
* $Id: soap_wsdl_schema_edit_fns.inc,v 1.2 2006/10/03 23:39:32 bcaldwell Exp $
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
* @version $Revision: 1.2 $
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