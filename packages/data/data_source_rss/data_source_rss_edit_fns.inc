<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_rss_edit_fns.inc,v 1.8 2012/08/30 00:58:16 ewang Exp $
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
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_RSS_Edit_Fns extends Data_Source_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

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
			text_box('rss_link', $url, 100);
			return TRUE;
		} else {
			echo $url;
		}
		return FALSE;

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
				return TRUE;
			}
		}
		return FALSE;

	}//end processRSS()


	/**
	* Paints the interface for selecting a date attribute to convert to timestamp
	*
	* @param object	&$asset	the Data_Source_DB asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintShadowDate(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$date_f = $asset->attr('date_field');

		if ($wa) {
			$shadow_asset = $asset->getAsset($asset->id.':0');
			$keywords = $shadow_asset->getAvailableKeywords();
			$options = Array('' => '-- Leave Empty --');
			// only get the ds__ fields with date keyword:
			// pubdate or updated or published
			foreach ($keywords as $key => $desc) {
			if (strstr($key, 'ds__') && (stristr($key, 'date') || stristr($key, 'publish'))) {
					$options[$key] = $key;
				}
			}
			combo_box($prefix.'_date_field', $options, FALSE, $date_f);
			return TRUE;
		} else {
			echo $date_f;
		}
		return FALSE;

	}//end paintShadowDate()


	/**
	* Process the interface for selecting a date attribute to convert to timestamp
	*
	* @param object	&$asset	the Data_Source_DB asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processShadowDate(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			if (isset($_POST[$prefix.'_date_field'])) {
				$date_f = $_POST[$prefix.'_date_field'];
				$asset->setAttrValue('date_field', $date_f);
				return TRUE;
			}
		}
		return FALSE;

	}//end processShadowDate()


}//end class
?>
