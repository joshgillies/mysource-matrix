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
* $Id: condition_simple_edit_mode.inc,v 1.1 2004/11/10 02:41:55 amiller Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Simple Edit Mode Condition
*
* This conditional checks to see if the session is in Simple Edit Mode
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.1 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Condition_Simple_Edit_Mode extends Condition
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
	* @return boolean
	* @access public
	*/
	function Evaluate()
	{
		if (!SQ_IN_LIMBO) { 
			return false;
		}
		
		return true;
		
	}//end Evaluate()

	
}//end class

?>