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
* $Id: data_source_rss_edit_fns.inc,v 1.3 2006/12/08 00:52:00 emcdonald Exp $
*
*/
require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';
/**
* Data_Source_RSS_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_RSS_Edit_Fns extends Data_Source_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Data_Source_RSS_Edit_Fns()
	{
		$this->Data_Source_Edit_Fns();

	}//end constructor


	/**
	* Paints the interface for entering Data Source details
	*
	* @param object	&$asset	the Data_Source_DB asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintRSS(&$asset, &$o, $prefix)
	{
		$url = $asset->attr('url');
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_box('rss_link', $url, 30);
			return true;
		} else {
			echo $url;
		}
		return false;

	}//end paintRSS()


	/**
	* Paints the interface for entering Data Source details
	*
	* @param object	&$asset	the Data_Source_DB asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processRSS(&$asset, &$o, $prefix)
	{
		$url = $asset->attr('url');
		if (isset($_POST['rss_link'])) {
			$link = $_POST['rss_link'];
			if (strcasecmp($url, $link) != 0) {
				$asset->setAttrValue('url', $link);
				// overwrite the cache with this new rss feed url
				$asset->setResultSet(Array(), $link);
				$asset->getResultSet($link);
				return true;
			}
		}
		return false;

	}//end processRSS()


}//end class
?>
