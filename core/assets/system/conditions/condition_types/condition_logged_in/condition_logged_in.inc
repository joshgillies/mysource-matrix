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
* $Id: condition_logged_in.inc,v 1.9 2006/12/06 04:23:58 emcdonald Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Logged In Condition
*
* This conditional checks to see if the user has successfully authenticated
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Logged_In extends Condition
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Condition_Logged_In($assetid=0)
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
		return !$GLOBALS['SQ_SYSTEM']->userPublic();

	}//end evaluate()


}//end class

?>