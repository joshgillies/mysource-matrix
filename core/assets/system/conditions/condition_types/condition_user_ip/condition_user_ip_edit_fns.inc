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
* $Id: condition_user_ip_edit_fns.inc,v 1.1 2005/02/22 22:03:02 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_User_IP_Edit_Fns
*
* Edit functions for the user IP condition
*
* @see Asset_Edit_Fns
*
* @author  Luke Wright <lwright@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_IP_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_User_IP_Edit_Fns()
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
			echo 'User IP '.strtolower($options[$restriction['match']]).' pattern "'.$restriction['condition_data']['user_ip'].'"';
			$o->closeField();
		} else {
			$o->openField('Match Condition');
			combo_box($prefix.'[match]', $options, false, $restriction['match']);
			$o->closeField();
		
			$o->openField('Compare to IP');
			text_box($prefix.'[compare]', $restriction['condition_data']['user_ip'], 30);
			$o->closeField();
		}
		$o->sectionNote('Insert an IP address in the Compare box. You can use an asterisk ("*") in place of a number to match all possible numbers within that octet.');
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
		// construct and return data required by this condition
		// as found in the post data, array required consists of
		// 'match' and 'condition_data'
		$results = Array(
			'condition_data' => Array(
				'user_ip' => $_POST[$prefix]['compare'],
			),
				'match' => $_POST[$prefix]['match'],
		);
		
		return $results;
		
	}//end processEditInterface()


}//end class

?>