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
* $Id: page_remote_content_edit_fns.inc,v 1.5.2.1 2005/12/08 04:08:30 arailean Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Remote_Content_Edit_Fns
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Remote_Content_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Remote_Content_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paint URL white list interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintWhiteList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$urls = $asset->attr('url_white_list');

		$urls = (is_array($urls))? implode("\n", $urls):'';

		if ($write_access) {
			text_area($prefix.'_url_white_list', $urls, 60, 10);
		} else {
			echo nl2br($urls);
		}
		return $write_access;

	}//end paintWhiteList()


	/**
	* Process URL white list interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processWhiteList(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$urls = array_get_index($_REQUEST, $prefix.'_url_white_list');

		$matches	= Array();
		$new_urls	= Array();

		if (preg_match_all('/\S+/i', $urls, $matches, PREG_SET_ORDER)) {
			foreach ($matches as $match){
				$url = array_pop($match);
				$new_urls[$url] = $url;
			}
		}

		$asset->setAttrValue('url_white_list', $new_urls);
		return true;

	}//end processWhiteList()


}//end class

?>