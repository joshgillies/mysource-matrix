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
* $Id: condition_server_variable.inc,v 1.12 2013/07/25 23:23:48 lwright Exp $
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
* @version $Revision: 1.12 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


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
		if (!isset($condition_data['server_variable']) || !isset($condition_data['server_variable_match'])) {
			return FALSE;
		}
		$server_var_index = $condition_data['server_variable'];
		$server_var_match = str_replace('/', '\/', $condition_data['server_variable_match']);
		$matches = preg_match('/'.$server_var_match.'/i', array_get_index($_SERVER, $server_var_index, ''));
		return ($matches > 0);

	}//end evaluate()


}//end class

?>
