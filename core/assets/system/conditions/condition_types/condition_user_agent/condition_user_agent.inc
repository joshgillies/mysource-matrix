<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: condition_user_agent.inc,v 1.6 2006/12/06 04:23:59 emcdonald Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Condition_User_Agent
*
* Purpose
*     This condition compares the user agent string against a specified
*     regular expression
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_Agent extends Condition
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Condition_User_Agent($assetid=0)
	{
		$this->Condition($assetid);

	}//end constructor


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
		if (!isset($condition_data['user_agent'])) {
			return FALSE;
		}
		$matches = preg_match('|'.$condition_data['user_agent'].'|i', $_SERVER['HTTP_USER_AGENT']);
		return ($matches > 0);

	}//end evaluate()


}//end class
?>