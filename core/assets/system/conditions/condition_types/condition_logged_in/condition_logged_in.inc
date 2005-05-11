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
* $Id: condition_logged_in.inc,v 1.6 2005/05/11 06:04:56 lwright Exp $
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
* @version $Revision: 1.6 $
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
	* Get list of match options
	*
	* @return Array
	* @access public
	*/
	function getOptions()
	{
		$options = Array(
					   '1' => translate('condition_logged_in'),
					   '0' => translate('condition_not_logged_in'),
					   );
		
		return $options;
		
	}//end getOptions()
	
	
	/**
	* Evaluate this condition
	*
	* @return boolean
	* @access public
	*/
	function evaluate()
	{
		return !$GLOBALS['SQ_SYSTEM']->userPublic();

	}//end evaluate()


}//end class

?>