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
* $Id: condition_user_ip.inc,v 1.1 2005/02/22 22:03:02 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Condition_User_IP
*
* Purpose
*     This condition compares the IP address of the current visitor against a
*     specified comparison address
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_IP extends Condition
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Condition_User_IP($assetid=0)
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
					   '1' => 'Matches',
					   '0' => 'Does Not Match',
					   );
		
		return $options;
		
	}//end getOptions()


	/**
	* Evaluate this condition
	*
	* @return boolean
	* @access public
	*/
	function evaluate($asset, $condition_data)
	{
		if (!isset($condition_data['user_ip'])) return false;
		
		// replace '*' with 'match between 1 and 3 digits'
		//$condition_data['user_ip'] = '/'.preg_replace(array('/\*/', '/\./'), array('\d{1,3}', '\\\.'), $condition_data['user_ip']).'/';
		$condition_data['user_ip'] = preg_replace(array('/\*/', '/\./'), array('\d{1,3}', '\\\.'), $condition_data['user_ip']);
		$matches = preg_match('|'.$condition_data['user_ip'].'|i', $_SERVER['REMOTE_ADDR']);
		
		return ($matches > 0);

	}//end evaluate()


}//end class

?>