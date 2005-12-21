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
* $Id: data_source_rss_edit_fns.inc,v 1.2 2005/12/21 21:58:35 rong Exp $
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
* @version $Revision: 1.2 $
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
