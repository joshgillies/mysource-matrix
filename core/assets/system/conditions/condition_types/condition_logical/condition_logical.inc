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
* $Id: condition_logical.inc,v 1.5 2006/01/26 22:34:05 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Logical Condition
*
* This conditional evaluates a ALL or ANY logical expression between any number
* of logical keywords.
*
* @author  Geoffroy Noel <gnoel@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Logical extends Condition
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Condition_Logical($assetid=0)
	{
		$this->Condition($assetid);

	}//end constructor


	/**
	* Returns the list of logical keywords contianed in a condition_data
	* structure
	*
	* @param array	&$condition_data	a condition logical condition_data structure
	*
	* @return array
	* @access public
	*/
	function getLogicalKeywords(&$condition_data)
	{
		return $condition_data['logical_keywords'];

	}//end getLogicalKeywords()


	/**
	* Sets the logical keywords in a condition_data structure
	*
	* @param array	&$condition_data	a condition logical condition_data structure
	* @param array	$logical_keywords	an array of logical keywords
	*
	* @return void
	* @access public
	*/
	function setLogicalKeywords(&$condition_data, $logical_keywords)
	{
		$condition_data['logical_keywords'] = $logical_keywords;

	}//end setLogicalKeywords()


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
		$ret = FALSE;

		if (!empty($condition_data['operator']) && !empty($condition_data['logical_keywords'])) {
			switch ($condition_data['operator']) {
				case 'ALL':
					$ret = TRUE;
					foreach ($condition_data['logical_keywords'] as $logical_value) {
						if (!$logical_value) {
							$ret = FALSE;
							break;
						}
					}
				break;

				case 'ANY':
					$ret = FALSE;
					foreach ($condition_data['logical_keywords'] as $logical_value) {
						if ($logical_value) {
							$ret = TRUE;
							break;
						}
					}
				break;
			}
		}

		return $ret;

	}//end evaluate()


	/**
	* Populate info about additional logical keywords into this condition
	*
	* Used predominantly with paint layouts - not entirely generic but if a
	* condition requires other keywords (as Condition_Logical does) then it's
	* not likely to be able to be used in user restrictions anyway
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
		$condition_logical_keywords = Condition_Logical::getLogicalKeywords($condition_data);
		foreach (array_keys($condition_logical_keywords) as $condition_logical_keyword_name) {
			$condition_logical_keywords[$condition_logical_keyword_name] = isset($logical_keywords[$condition_logical_keyword_name]) ? $logical_keywords[$condition_logical_keyword_name] : '';
		}

		Condition_Logical::setLogicalKeywords($condition_data, $condition_logical_keywords);

	}//end updateKeywords()


	/**
	* Get additional keywords that this condition requires in order to be evaluated
	*
	* Used predominantly with paint layouts - not entirely generic but if a
	* condition requires other keywords (as Condition_Logical does) then it's
	* not likely to be able to be used in user restrictions anyway
	*
	* @param array	$condition_data	Condition data array for this condition
	*
	* @return array
	* @access public
	* @static
	*/
	function getRequiredKeywords($condition_data)
	{
		return array_keys(Condition_Logical::getLogicalKeywords($condition_data));

	}//end getRequiredKeywords()


}//end class

?>