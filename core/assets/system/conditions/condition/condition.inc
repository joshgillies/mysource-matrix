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
* $Id: condition.inc,v 1.14 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Condition
*
* Parent class for conditional processing classes
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition extends Asset
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
	* Static function to evaluates a particular condition.
	*
	* Like evaluate() but takes the whole restriction into account.
	* This helps making the condition restrictions totally transparant for the
	* client class (i.e. they don't need to understand or manipulate its
	* content).
	*
	* @param Asset	$asset					Asset to evaluate the condition for
	* @param array	&$restriction			a restriction array as returned by processEditInterface()
	*										<pre>
	*										Array(
	*											'match' => 1|0,
	*											'condition_data'=> Array()
	*										)
	*										</pre>
	* @param string	$condition_type_code	condition type code to evaluate
	*
	* @return boolean
	* @access public
	*/
	public static function evaluateRestriction(Asset $asset, Array &$restriction, $condition_type_code)
	{
		$condition_data = empty($restriction['condition_data']) ? Array() : $restriction['condition_data'];
		$prefix = array_get_index($restriction, 'prefix', '');
		$evaluation_instruction = '$ret = '.$condition_type_code.'::evaluate($asset, $condition_data, $prefix);';

		$GLOBALS['SQ_SYSTEM']->am->includeAsset($condition_type_code);
		eval($evaluation_instruction);

		if (isset($restriction) && (isset($restriction['match'])) && !$restriction['match']) {
			$ret = !$ret;
		}

		return $ret;

	}//end evaluateRestriction()


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
		return FALSE;

	}//end evaluate()


	/**
	* Populate info about additional logical keywords into this condition
	*
	* Used predominantly with paint layouts in conjunction with getRequiredKeywords() -
	* not entirely generic but if a condition requires other keywords then it's
	* not likely to be able to be used in user restrictions anyway
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
		return;

	}//end updateKeywords()


	/**
	* Get additional keywords that this condition requires in order to be evaluated
	*
	* Used predominantly with paint layouts - not entirely generic but if a
	* condition requires other keywords then it's not likely to be able to
	* be used in user restrictions anyway
	*
	* @param array	$condition_data	Condition data array for this condition
	*
	* @return array
	* @access public
	* @static
	*/
	public static function getRequiredKeywords(Array $condition_data)
	{
		return Array();

	}//end getRequiredKeywords()


	/**
	* Get a list of match keywords
	*
	* Override in each class with the correct translate()'d text.
	*
	* @param string	$type_code	the type code of this asset
	*
	* @return array
	* @access public
	* @static
	*/
	public static function getMatchKeywords($type_code)
	{
		$match_options = Array();

		return $match_options;

	}//end getMatchKeywords()


}//end class

?>