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
* $Id: condition_logical.inc,v 1.9 2012/08/30 01:09:10 ewang Exp $
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
* @version $Revision: 1.9 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

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
	public static function getLogicalKeywords(Array &$condition_data)
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
	public static function setLogicalKeywords(Array &$condition_data, Array $logical_keywords)
	{
		$condition_data['logical_keywords'] = $logical_keywords;

	}//end setLogicalKeywords()


	/**
	* Evaluate this condition
	*
	* @param Asset	$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate(Asset $asset, $condition_data)
	{
		$ret = FALSE;
		if (!empty($condition_data['operator']) && !empty($condition_data['logical_keywords'])) {
			// replace global and non-global keywords
			foreach ($condition_data['logical_keywords'] as $keyword => $value) {
				// Replace only if the conditional keyword is not already replaced
				if (empty($value) || $value === '%'.$keyword.'%') {
					if (strpos($keyword, 'globals_') === 0) {
						$value = '%'.$keyword.'%';
						replace_global_keywords($value);
					} else {
						$value = $asset->getKeywordReplacement($keyword);
					}
					// Blank out the non-replaced keyword, otherwise it will be interpreted as TRUE
					$condition_data['logical_keywords'][$keyword] = $value == '%'.$keyword.'%' ? '' : $value;
				}
			}

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
	public static function updateKeywords(Array $logical_keywords, Array &$condition_data)
	{
		$condition_logical_keywords = self::getLogicalKeywords($condition_data);
		foreach (array_keys($condition_logical_keywords) as $condition_logical_keyword_name) {
			$condition_logical_keywords[$condition_logical_keyword_name] = isset($logical_keywords[$condition_logical_keyword_name]) ? $logical_keywords[$condition_logical_keyword_name] : NULL;
		}

		self::setLogicalKeywords($condition_data, $condition_logical_keywords);

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
	public static function getRequiredKeywords($condition_data)
	{
		return array_keys(self::getLogicalKeywords($condition_data));

	}//end getRequiredKeywords()


}//end class

?>
