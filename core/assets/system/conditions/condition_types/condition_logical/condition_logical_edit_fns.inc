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
* $Id: condition_logical_edit_fns.inc,v 1.1 2005/04/06 21:40:00 gnoel Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_Logical_Edit_Fns
*
* Edit functions for the logged in conditional
*
* @see Asset_Edit_Fns
*
* @author  Geoffroy Noel <gnoel@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Logical_Edit_Fns extends Condition_Edit_Fns
{

	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_Logical_Edit_Fns()
	{
		$this->Condition_Edit_Fns();
		
	}//end constructor

	
	/**
	* Paint the backend editing interface for this condition
	*
	* @return void
	* @access public
	*/
	function paintEditInterface(&$asset, &$o, $prefix, &$restriction, $write_access, $allowed_keywords=Array())
	{
		$options = $asset->getOptions();
		
		if (empty($restriction['condition_data'])) {
			$condition_data =  Array('logical_keywords' => Array('logical_keyword_1' => ''), 'operator' => 'ALL');
		} else {
			$condition_data = $restriction['condition_data'];
		}  

		if (!$write_access) {
			$o->openField('Logical Keywords');
			echo join(array_keys($condition_data['logical_keywords']), ', ');
			$o->closeField();
			
			$o->openField('Operator');
			echo $condition_data['operator'];
			$o->closeField();
								
			$o->openField('Condition Details');
			echo 'Expression must be '.strtolower($options[$restriction['match']]);
			$o->closeField();
		} else {
			/*					
			$o->openField('Logical Keywords');
			text_box($prefix.'[logical_keywords]', join(array_keys($condition_data['logical_keywords']), ', '), 30);				
			$o->closeField();
			*/
			
			if (!empty($allowed_keywords)) {
				$selected_keywords = array_keys($condition_data['logical_keywords']);
				$selected_keywords['-- none --'] = '-- none --';
								
				$keyword_options = Array();
				foreach ($allowed_keywords as $keyword) {
					$keyword_options[$keyword] = $keyword;
				}
					
				$i = 0;
				foreach ($selected_keywords as $keyword) {				
					$special_option = Array();
					if ($i == (count($selected_keywords) - 1)) {
						$special_option = Array('none' => '-- none --');
					} elseif ($i > 0) {
						$special_option = Array('none' => '-- remove --');
					}
					
					$metadata = Array();
					if (preg_match('/^asset_metadata_(.*)$/', $keyword, $metadata)) {
						$keyword = 'asset_metadata_';						
						$metadata = $metadata[1];
					}

					$o->openField('Logical Keyword ' . $i);
					combo_box($prefix.'[keyword' . $i . ']', array_merge($special_option, $keyword_options), false, $keyword);
					$o->closeField();
					
					if (!is_array($metadata)) {
						$o->openField('Metadata');
						text_box($prefix.'[metadata' . $i . ']', $metadata, 30);
						$o->closeField();
					}
					
					$i++;					
				}
			} else {
				$o->openField('Logical Keywords');
				echo('The keyword list is empty.');
				$o->closeField();
			}			

			$o->openField('Operator');
			combo_box($prefix.'[operator]', Array('ALL' => 'ALL', 'ANY' => 'ANY'), false, $condition_data['operator']);
			$o->closeField();

			$o->openField('Match Condition');
			combo_box($prefix.'[match]', $options, false, $restriction['match']);
			$o->closeField();
		}
		return true;
		
	}//end paintEditInterface()

	
	/**
	* Process the backend editing interface for this condition
	*
	* @return Array 
	* @access public
	*/
	function processEditInterface(&$asset, &$o, $prefix, &$restriction, $write_access)
	{
		//$keywords = $_POST[$prefix]['logical_keywords'];
		//$keywords = trim($keywords);		
		//$keywords = preg_replace('/ *, */', ',', $keywords);
		/*  
		$keywords = preg_replace('/ +/', '_', $keywords);
		$keywords = explode(',', $keywords);
		//error_log($keywords);
		$logical_keywords = Array();
		foreach ($keywords as $keyword_name) {
			if (!empty($keyword_name)) $logical_keywords[$keyword_name] = '';
		}
		*/
		
		$logical_keywords = Array();
		
		for ($i = 0; $i < 100; $i++) {
			if (isset($_POST[$prefix]['keyword' . $i])) {
				$keyword_name = $_POST[$prefix]['keyword' . $i];
				if ($keyword_name != 'none') {
					if (preg_match('/^asset_metadata/', $keyword_name)) {
						if (!empty($_POST[$prefix]['metadata' . $i])) {
							$metadata_name = trim($_POST[$prefix]['metadata' . $i]);
							if (!empty($metadata_name)) $keyword_name = 'asset_metadata_' . $metadata_name;
						}
					}
					$logical_keywords[$keyword_name] = '';
				}
			}
		}
		
		if (empty($logical_keywords)) $logical_keywords = Array('logical_keyword_1' => '');
		
		$results = Array(
			'match' => $_POST[$prefix]['match'],
			'condition_data' => Array(
				'logical_keywords' => $logical_keywords,
				'operator' => $_POST[$prefix]['operator'], 
			),
		);
		
		return $results;
		
	}//end processEditInterface()
	
	
}//end class

?>