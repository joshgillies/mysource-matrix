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
* $Id: condition_admin_access_edit_fns.inc,v 1.2 2004/12/01 11:42:59 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_Write_Access_Edit_Fns
*
* Edit functions for the admin access conditional
*
* @see Asset_Edit_Fns
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Admin_Access_Edit_Fns extends Condition_Edit_Fns
{

	/**
	* Constructor
	* 
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Condition_Admin_Access_Edit_Fns()
	{
		$this->Condition_Edit_Fns();
	}//end constructor
	
} //end class

?>