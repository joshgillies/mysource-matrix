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
* $Id: condition_server_variable.inc,v 1.2.2.2 2005/08/08 07:16:59 dmckee Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Condition_Server_Var
*
* Purpose
*     This condition compares the user agent string against a specified
*     regular expression
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.2.2.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Server_Variable extends Condition
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Condition_Server_Variable($assetid=0)
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
					'match_options'		=> Array(
											'1'	=> translate('matches'),
											'0'	=> translate('does_not_match'),
										   ),
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
		if (!isset($condition_data['server_variable']) || !isset($condition_data['server_variable_match'])) {
			return false;
		}
		$server_var_index = $condition_data['server_variable'];
		$server_var_match = $condition_data['server_variable_match'];
		$matches = preg_match('|'.$server_var_match.'|i', $_SERVER[$server_var_index]);
		return ($matches > 0);

	}//end evaluate()


}//end class

?>