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
* $Id: condition_keyword_regexp.inc,v 1.13 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Condition_Keyword_Regexp
*
* Purpose
*     This condition compares a keyword against a specified regular expression
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Keyword_Regexp extends Condition
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
		if (!isset($condition_data['keyword']) || !isset($condition_data['keyword_match'])) {
			return FALSE;
		}

		if (isset($condition_data['keyword_value'])) {

			if (!preg_match('/^%globals_/', $condition_data['keyword_value'])) {
				$keyword_replacement = $condition_data['keyword_value'];
			}
			else {
				// If keyword is a global then its value is set as %global_%, which is evaluate only after bodycopy 
				// is created. But for processing global keyword here, its value needs to be evaluated right here. 
				// See bug #3716

				$keyword_replacement = replace_global_keywords($condition_data['keyword_value']);
			}
		} else {
			$keyword_replacement = self::getRegExpKeywordReplacement($asset, $condition_data);
		}
		
		$keyword_match = $condition_data['keyword_match'];
		
		if (array_get_index($condition_data, 'replace_keyword_in_match', '0') == '1') {
			//replace global keywords
			replace_global_keywords($keyword_match);
	
			//replace asset keywords
			$keywords = retrieve_keywords_replacements($keyword_match);
			$replacements = Array();
			foreach ($keywords as $keyword) {
				$replacements[$keyword] = $asset->getKeywordReplacement($keyword);
			}
			replace_keywords($keyword_match, $replacements);
		}
	
		$matches = preg_match('/'.str_replace('/', '\\'.'/', $keyword_match).'/i', $keyword_replacement);
		return ($matches > 0);

	}//end evaluate()


	/**
	* Evaluate keywords (local or global)
	*
	* @param Asset	$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	public static function getRegExpKeywordReplacement(Asset $asset, Array $condition_data)
	{
		$ret = '';

		if (preg_match('/^globals_/', $condition_data['keyword'])) {
			$ret = '%'.$condition_data['keyword'].'%';
			replace_global_keywords($ret);
		} else {
			$ret = $asset->getKeywordReplacement($condition_data['keyword']);
		}

		return $ret;

	}//end getRegExpKeywordReplacement()


	/**
	* Populate info about additional logical keywords into this condition
	*
	* Used predominantly with paint layouts - not entirely generic but if a
	* condition requires other keywords (as Condition_Keyword_Regexp does) then
	* it's not likely to be able to be used in user restrictions anyway
	*
	* @param array	$logical_keywords	Logical keywords already evaluated
	* @param array	&$condition_data	Condition data array for this condition
	*
	* @return void
	* @access public
	* @static
	*/
	public static function updateKeywords(Array $logical_keywords, Array &$condition_data)
	{
		$keyword_name = parse_keyword($condition_data['keyword'], $modifiers);

		if (count($modifiers) >= 1) {
			$condition_data['keyword_value'] = '%'.$condition_data['keyword'].'%';
		} else {
			$condition_data['keyword_value'] = isset($logical_keywords[$keyword_name]) ? $logical_keywords[$keyword_name] : '';
		}//end if

	}//end updateKeywords()


	/**
	* Get additional keywords that this condition requires in order to be evaluated
	*
	* Used predominantly with paint layouts - not entirely generic but if a
	* condition requires other keywords (as Condition_Keyword_Regexp does) then
	* it's not likely to be able to be used in user restrictions anyway
	*
	* @param array	$condition_data	Condition data array for this condition
	*
	* @return array
	* @access public
	* @static
	*/
	public static function getRequiredKeywords(Array $condition_data)
	{
		return Array($condition_data['keyword']);

	}//end getRequiredKeywords()


}//end class

?>
