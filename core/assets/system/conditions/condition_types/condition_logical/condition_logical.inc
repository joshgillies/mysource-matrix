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
* $Id: condition_logical.inc,v 1.2 2005/05/11 06:04:56 lwright Exp $
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
* @version $Revision: 1.2 $
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
	* Get list of match options
	*
	* @return Array
	* @access public
	*/
	function getOptions()
	{
		$options = Array(
					   '1' => translate('true'),
					   '0' => translate('false'),
					   );
		
		return $options;
		
	}//end getOptions()
	
	
	/**
	 * Returns the list of logical keywords contianed in a condition_data
	 * structure
	 * 
	 * @param Array $condition_data 	a condition logical condition_data
	 * structure
	 *
	 * @return Array(keyword_name => value);
	 * @acces public
	 */
	function getLogicalKeywords(&$condition_data)
	{
		return $condition_data['logical_keywords'];
		
	}
	
	
	/**
	 * Sets the logical keywords in a condition_data structure
	 *
	 * @param Array $condition_data		a condition logical condition_data
	 * structure
	 * @param Array $logical_keywords	an array of logical keywords
	 * 
	 * @return void
	 * @acces public
	 */
	function setLogicalKeywords(&$condition_data, $logical_keywords)
	{
		$condition_data['logical_keywords'] = $logical_keywords;
		
	}


	/**
	* Evaluate this condition
	*
	* @return boolean
	* @access public
	*/
	function evaluate(&$asset, $condition_data)
	{
		$ret = false;
		
		if (!empty($condition_data['operator']) && !empty($condition_data['logical_keywords'])) {			
			switch ($condition_data['operator']) {
				case 'ALL':
					$ret = true;
					foreach ($condition_data['logical_keywords'] as $logical_value) {
						if (!$logical_value) { 
							$ret = false;
							break;
						}
					}
					break;
				case 'ANY':
					$ret = false;
					foreach ($condition_data['logical_keywords'] as $logical_value) {
						if ($logical_value) { 
							$ret = true;
							break;
						}
					}
					break;
			}
		}
		
		return $ret;

	}//end evaluate()


}//end class

?>