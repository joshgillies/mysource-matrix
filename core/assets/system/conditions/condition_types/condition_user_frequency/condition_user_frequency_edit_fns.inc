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
* $Id: condition_user_frequency_edit_fns.inc,v 1.1 2005/10/06 01:01:40 dheppell Exp $
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
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_Frequency_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_User_Frequency_Edit_Fns()
	{
		$this->Condition_Edit_Fns();

	}//end constructor


	/**
	* Paint the backend editing interface for this condition
	*
	* @param object		&$asset			The asset we are working with
	* @param object		&$o				The backend outputter object
	* @param string		$prefix			Contition prefix value
	* @param array		&$restriction	The assets restriction data to work with
	* @param boolean	$write_access	Whether or not we have write access
	*
	* @return void
	* @access public
	*/
	function paintEditInterface(&$asset, &$o, $prefix, &$restriction, $write_access)
	{
		return true;

	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param object		&$asset			The asset we are working with
	* @param object		&$o				The backend outputter object
	* @param string		$prefix			Contition prefix value
	* @param array		&$restriction	The assets restriction data to work with
	* @param boolean	$write_access	Whether or not we have write access
	*
	* @return array
	* @access public
	*/
	function processEditInterface(&$asset, &$o, $prefix, &$restriction, $write_access)
	{
		return true;

	}//end processEditInterface()


}//end class

?>