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
* $Id: condition_in_user_group_edit_fns.inc,v 1.1 2005/04/05 18:34:21 brobertson Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_In_User_Group_Edit_Fns
*
* Edit functions for the in user group conditional
*
* @see Asset_Edit_Fns
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_In_User_Group_Edit_Fns extends Condition_Edit_Fns
{

	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_In_User_Group_Edit_Fns()
	{
		$this->Condition_Edit_Fns();
		
	}//end constructor

	
	/**
	* Paint the backend editing interface for this condition
	*
	* @return void
	* @access public
	*/
	function paintEditInterface(&$asset, &$o, $prefix, &$restriction, $write_access)
	{
		$options = $asset->getOptions();
		
		if (!$write_access) {
			$o->openField('Condition Details');
			echo 'User is '.strtolower($options[$restriction['match']]);
			$o->closeField();
		} else {
			$o->openField('Match Condition');
			combo_box($prefix.'[match]', $options, false, $restriction['match']);
			$o->closeField();
			$o->openField('User Groups');
			multiple_asset_finder($prefix.'[groupids]', array_get_index($restriction['condition_data'], 'groupids', Array()), Array('user_group' => 'D'));
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
		if (empty($_POST[$prefix])) return Array();

		$groupids = Array();
		foreach($_POST[$prefix]['groupids'] as $data) {
			if (empty($data['assetid'])) continue;
			$groupids[] = $data['assetid'];
		}

		// construct and return data required by this condition
		// as found in the post data, array required consists of
		// 'match' and 'condition_data'
		$results = Array(
			'condition_data' => Array(
				'groupids' => $groupids,
			),
			'match' => $_POST[$prefix]['match'],
		);
		
		return $results;
		
	}//end processEditInterface()
	
	
}//end class

?>