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
* $Id: condition.inc,v 1.10 2006/02/23 00:13:47 lwright Exp $
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
* @version $Revision: 1.10 $
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
	function Condition($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Static function to evaluates a particular condition.
	*
	* Like evaluate() but takes the whole restriction into account.
	* This helps making the condition restrictions totally transparant for the
	* client class (i.e. they don't need to understand or manipulate its
	* content).
	*
	* @param object	&$asset					a user asset (or child of User class)
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
	function evaluateRestriction(&$asset, &$restriction, $condition_type_code)
	{
		$condition_data = empty($restriction['condition_data']) ? Array() : $restriction['condition_data'];
		$evaluation_instruction = '$ret = '.$condition_type_code.'::evaluate($asset, $condition_data);';

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
	* @param object	&$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	function evaluate(&$asset, $condition_data)
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
	function updateKeywords($logical_keywords, &$condition_data)
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
	function getRequiredKeywords($condition_data)
	{
		return Array();

	}//end getRequiredKeywords()


	/**
	* Get a list of match keywords
	*
	* Only override this in a child condition asset in very exceptional
	* circumstances (see the User IP condition for an example).
	*
	* In the future, this could be potentially expanded to offer options other
	* than TRUE or FALSE match options, but not without a fair bit of further
	* development.
	*
	* @param string	$type_code	the type code of this asset
	*
	* @return array
	* @access public
	* @static
	*/
	function getMatchKeywords($type_code)
	{
		$match_options = Array(
							'1'	=> translate($type_code.'_true'),
							'0'	=> translate($type_code.'_false'),
						 );

		return $match_options;

	}//end getMatchKeywords()


}//end class

?>