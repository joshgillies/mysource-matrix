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
* $Id: paint_layout_bodycopy.inc,v 1.1 2005/04/06 22:03:23 gnoel Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH . '/bodycopy/bodycopy/bodycopy.inc';
require_once 'include/conditions_list.inc';

/**
* Paint_Layout_Bodycopy
* 
* A Text/HTML Template for Assets
* Represents an independant and customizable presentation layers for a type of
* asset.
* 
* The content of the bodycopy can hold 2 types of keywords:
* 
*	Substitution keywords:
*		. %asset_attribute_XXX% 
*		. %asset_metadata_XXX%
*		. %asset_content%
*		. other custom keywords
*
*	Conditional keywords:
*		. %begin_KEYWORD%
*		. %else_KEYWORD%
*		. %end_KEYWORD%
*		(where KEYWORD is a substitution keyword)
*
* If an attribute is not recognised, the condition will be evaluated as false.
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Bodycopy extends Bodycopy
{
	
	var $asset_to_paint = null;
	
	var $conditions_list = null;
	
	var $keyword_replacements = Array();
	
	
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Paint_Layout_Bodycopy($assetid=0) 
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);
				
	}
	
	
	/**
	 * Returns the body of the painted asset
	 * 
	 * @param object|Asset $asset 	the asset to paint
	 * 
	 * @return string
	 * @access public
	 */
	function paintAsset(&$asset)
	{
		/* 
		 * Possibles keywords:
		 * 
		 * Substitution
		 * 
		 * 1. asset keywords 
		 * 2. %asset_content%
		 *  
		 * Condition
		 * 
		 * 3. %begin_SUBSTITUTION_KEYWORD% ... %end_SUBSTITUTION_KEYWORD%
		 *  
		 */

		$ret = '';
		
		if (!$this->attr('validated')) {
			trigger_error('Unable to paint asset "'.$asset->name.'" (#'.$asset->id.') the structure in this Paint Layout failed to validate', E_USER_WARNING);
			ob_start();
			$asset->printBody();
			return ob_get_clean(); 
		}
		
		$this->asset_to_paint = &$asset;
		
		$content = $this->getRawBodycopyContent();
		
		$required_keywords = retrieve_keywords_replacements($content);
		
		// 1. replace conditional keywords		 
		$php_begin = ' <' . '?' . 'php ';
		$php_end = ' ?' . '> ';
		$keyword_replacements = Array();
		// all the keywords required by the    
		$additional_keywords = Array();
		
		$required_substitution_keywords = Array();
		
		foreach ($required_keywords as $keyword) {
		 	if (!isset($keyword_replacements[$keyword])) {
			 	$value = null;
			 	
				if (preg_match('/^(' . SQ_KEYWORD_CONDITION_BEGIN . '|' . SQ_KEYWORD_CONDITION_END . '|' . SQ_KEYWORD_CONDITION_ELSE . ')_(.*)$/', $keyword, $condition_parts)) {
					if (count($condition_parts) == 3) {
						$test_keyword = $condition_parts[2];
				 		switch ($condition_parts[1]) {
				 			case SQ_KEYWORD_CONDITION_BEGIN :
				 				$this->_addRequiredKeywordsFromEvaluatedKeyword($test_keyword, $required_substitution_keywords);
						 		$value = 'if ($this->_evaluateKeyword(\'' . $test_keyword . '\')) {';
						 		break;
				 			case SQ_KEYWORD_CONDITION_ELSE :
						 		$value = '} else {';
						 		break;
				 			case SQ_KEYWORD_CONDITION_END :
						 		$value = '} //' . $test_keyword;
						 		break;
				 		}
	
				 		if (!empty($value)) {
				 			$value = $php_begin . $value . $php_end;
				 		}
					}
			 	}
			 	
			 	if (isset($value)) {
			 		$keyword_replacements[$keyword] = $value;
			 	} else {
			 		$required_substitution_keywords[$keyword] = 1;
			 	}
			}
		}
		
		//$content = replace_keywords($content, $keyword_replacements);

		// 2. replace substitution keywords		
		// add keywords required by conditions	
		//$required_keywords = array_merge(retrieve_keywords_replacements($content), $additional_keywords);
		//$keyword_replacements = &$this->_getAssetKeywordReplacements($asset, $required_keywords);
		
		$keyword_replacements = array_merge($keyword_replacements, $this->_getAssetKeywordReplacements($asset, array_keys($required_substitution_keywords)));			
		
		$content = replace_keywords($content, $keyword_replacements);
		
		// 3. evaluate the whole content
		$content = $php_end . $content . $php_begin;
		
		unset($this->keyword_replacements);
		$this->keyword_replacements = &$keyword_replacements;
		
		ob_start();
			eval($content);
			$ret = ob_get_contents();
		ob_end_clean();
		
		//error_log($content);
		//error_log($ret);
		
		return $ret;
		
	}
	
	
	/**
	 * Add to an array all the keywords which value is needed to evaluate a
	 * particular keyword
	 * 
	 * @param string	$test_keyword		the keyword that will be evaluated 
	 * @param Array 	$required_keywords	the Array to which the needed
	 * keywords will be added
	 * 
	 * @return void
	 * @access private
	 */
	function _addRequiredKeywordsFromEvaluatedKeyword($evaluated_keyword, &$required_keywords)
	{
		// if the evaluated keyword is a logical condition, then gets the logical keywords it is relying on		
		$condition_list = new Conditions_List($this, 'conditional_keywords');
		foreach ($condition_list->getRequiredKeywords($evaluated_keyword) as $required_keyword) {
			$required_keywords[$required_keyword] = 1; 
		}
		
		// add at least the keyword that will be evaluated
		$required_keywords[$evaluated_keyword] = 1;
		
	}
	

	/**
	* Returns true if the value of the keyword is not blank.
	* 0 will be evaluated as true.
	*
	* @param mixed $keyword_name	a keyword name 
	*
	* @return boolean
	* @access public
	*/
	function _evaluateKeyword($keyword_name)
	{				
		if (!isset($this->keyword_replacements[$keyword_name])) return false;
		
		$value = trim($this->keyword_replacements[$keyword_name]);
		
		return ($value != '');	
		
	}
	
	
	/**
	 * Returns the value of a single keyword
	 * 
	 * @param string $keyword	keyword name that must be replaced 
	 * 
	 * @return mixed 
	 * @access private
	 */
	function _getKeywordValue($keyword)
	{
		// TODO: must optimise getAssetKeywordReplacements() with a cache		
		$ret = $this->_getAssetKeywordReplacements($this->asset_to_paint, Array($keyword));
		
		$ret = isset($ret[$keyword]) ? $ret[$keyword] : '';
		
		return $ret;
		
	}
	

	/**
	 * Returns the values of a list of keywords.
	 * The returned array may contain more keywords than needed.
	 * 
	 * @param object 		$asset				the asset which will provide the
	 * values for the keywords
	 * @param Array(string) $required_keywords	keyword names that must be
	 * replaced
	 * 
	 * @return Array() 
	 * @access private
	 */
	function &_getAssetKeywordReplacements(&$asset, $required_keywords) 
	{
		$ret = Array();		

		// 1. asset content
		if (in_array('asset_contents', $required_keywords)) {
			ob_start();
				$asset->printBody();
				$ret['asset_contents'] = ob_get_contents();
			ob_end_clean();
		}

		// 2. asset keywords (includes attributes)
		// TODO: getAssetKeywords should accept the list of required keywords as a new argument  
		$asset_keywords = $asset->getAssetKeywords(false);
		foreach($asset_keywords as $keyword_name => $keyword) {
			$ret[$keyword_name] = $keyword['value'];
		}	
		
		// 3. asset metadata
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();		
		$metadata_keywords = $mm->getMetadataFieldValues($asset->id);

		foreach ($metadata_keywords as $field => $value) {
			$ret['asset_metadata_' . $field] = $value;
		}
		
		// 4. Conditions
		// TODO:  	
		// This will make this harder once the $required_keywords will be really applied by getAssetKeywords()
		// function, because the logical conditions may need other keywords which are not in $required_keywords
		$condition_list = new Conditions_List($this, 'conditional_keywords');
		$ret = array_merge($ret, $condition_list->evaluate($required_keywords, $ret, Array(false, true)));		

		return $ret;
		
	}	
		
			
	/**
	 * Sets the associated type code
	 * 
	 * @param string $asset_type	asset type
	 * 
	 * @return void
	 * @access public
	 */
	function setAssociatedAssetType($asset_type='')
	{
		$asset->setAttrValue('associated_asset_type', $asset_type);
		
	}
	
			
	/**
	 * Gets the associated type code
	 * 
	 * @return string
	 * @access public
	 */
	function getAssociatedAssetType()
	{
		return $this->attr('associated_asset_type');
		
	}
	
	
	/**
	 * Fill an array with the the list of all available keywords 
	 * 
	 * @param Array 	$keywords			an array to fill
	 * @param string 	$asset_type_code	the type of asset which should be
	 * represented by this bodycopy.
	 * 
	 * @return void
	 * @access public
	 */
	function getKeywordsDescription(&$keywords, $asset_type_code='')
	{		
		if (is_array($keywords)) {
			$show_descrition = false;
			
			// 1. asset content
			$keywords['asset_contents'] = 'Asset Contents';

			if (!empty($asset_type_code)) {
				// 2. asset keywords (includes attributes)
				// TODO: getAssetKeywords should accept the list of required keywords as a new argument  
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type_code);
				$asset = new $asset_type_code();
			} else {
				$asset = new Asset();
			}
			
			$asset_keywords = $asset->getAssetKeywords(true);
			foreach($asset_keywords as $keyword_name => $keyword) {
				$keywords[$keyword_name] = (empty($keyword['description']) || !$show_descrition) ? ucwords(str_replace('_', ' ', $keyword_name)) : $keyword['description'];
			}
			
			unset($asset);
			
			// 3. asset metadata
			$keywords['asset_metadata_'] = 'Asset Metadata X';
			
			// 4. conditions 
			$condition_list = new Conditions_List($this, 'conditional_keywords');
			$condition_names = $condition_list->getConditionsNames();
			foreach($condition_names as $condition_name) {
				$keywords[$condition_name] = ucwords(str_replace('_', ' ', $condition_name));
			}
						
			ksort($keywords);	
			
		}
		
	}
	
	
}

?>
