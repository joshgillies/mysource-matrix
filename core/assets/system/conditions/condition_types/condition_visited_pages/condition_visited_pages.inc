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
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';
require_once dirname(__FILE__).'/condition_visited_pages_edit_fns.inc';


/**
* Condition_Visited_Pages
*
* Purpose
*	This condition compares the URLs/assetids against the pages visited in the session
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Visited_Pages extends Condition
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Evaluate this condition
	*
	* @param Asset	$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate(Asset $asset, Array $condition_data)
	{
		// Get the previously visited urls from the asset
		$visted_urls = isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['visited_urls']) ? $_SESSION[SQ_SESSION_SANDBOX_INDEX]['visited_urls'] : Array();
		$visted_assetids = isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['visited_assets']) ? $_SESSION[SQ_SESSION_SANDBOX_INDEX]['visited_assets'] : Array();

		// Compare assetids first
		$assetids = isset($condition_data['assetids']) ? $condition_data['assetids'] : Array();
		$matched = array_intersect($visted_assetids, $assetids) ? TRUE : FALSE;

		// If assetids didn't matched, try comparing URLs
		if (!$matched) {
			$urls = isset($condition_data['url_patterns']) ? $condition_data['url_patterns'] : Array();
			if (!empty($urls)) {
				foreach($visted_urls as $visited_url) {
					foreach($urls as $url_regex) {
						$url_regex = str_replace('/', '\/', $url_regex);
						if (@preg_match('/'.$url_regex.'/i', $visited_url)) {
							$matched = TRUE;
							break;
						}
					}//end foreach
				}//end foreach
			}//end if
		}//end if

		return $matched;

	}//end evaluate()


	/**
	* Get a list of match keywords
	*
	* For User IP condition, we are not using the true/false condition - we are
	* using our own Grant/Deny logic. Therefore we will limit the 'match'
	* condition to just the TRUE condition
	*
	* @param string	$type_code	the type code of this asset
	*
	* @return array
	* @access public
	* @static
	*/
	public static function getMatchKeywords($type_code)
	{
		$match_options = Array(
							'1'	=> translate($type_code.'_true'),
							'0' => translate($type_code.'_false'),
						 );

		return $match_options;

	}//end getMatchKeywords()


}//end class

?>
