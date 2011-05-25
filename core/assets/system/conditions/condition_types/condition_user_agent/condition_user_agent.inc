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
* $Id: condition_user_agent.inc,v 1.8.14.1 2011/05/25 22:52:14 akarelia Exp $
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
* @version $Revision: 1.8.14.1 $
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
	public static function evaluate(Asset $asset, Array $condition_data)
	{
		if (!isset($condition_data['user_agent'])) {
			return FALSE;
		}
		if (!isset($_SERVER['HTTP_USER_AGENT'])) {
			return FALSE;
		}

		// escape any '/' that are thrown our way
		$condition_data['user_agent'] = str_replace('/', '\/', $condition_data['user_agent']);
		$matches = preg_match('/'.$condition_data['user_agent'].'/i', $_SERVER['HTTP_USER_AGENT']);
		return ($matches > 0);

	}//end evaluate()


}//end class

?>
