<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: condition_keyword_regexp.inc,v 1.3 2006/12/06 04:23:58 emcdonald Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Condition_Keyword_Regexp
*
* Purpose
*     This condition compares a keyword against a specified regular expression
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.3 $
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
	function Condition_Keyword_Regexp($assetid=0)
	{
		$this->Condition($assetid);

	}//end constructor


	/**
	* Evaluate this condition
	*
	* @param object	&$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	function evaluate(&$asset, $condition_data)
	{
		if (!isset($condition_data['keyword_value']) || !isset($condition_data['keyword_match'])) {
			return FALSE;
		}
		$keyword_replacement = $condition_data['keyword_value'];
		$keyword_match = $condition_data['keyword_match'];
		$matches = preg_match('|'.$keyword_match.'|i', $keyword_replacement);
		return ($matches > 0);

	}//end evaluate()


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
	function updateKeywords($logical_keywords, &$condition_data)
	{
		$keyword_name = $condition_data['keyword'];
		$condition_data['keyword_value'] = $logical_keywords[$keyword_name] ? $logical_keywords[$keyword_name] : '';

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
	function getRequiredKeywords($condition_data)
	{
		return Array($condition_data['keyword']);

	}//end getRequiredKeywords()


}//end class

?>