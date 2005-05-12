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
* $Id: password_change_design_management.inc,v 1.1 2005/05/12 23:57:36 ndvries Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Password_Change_Design_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Password_Change_Design_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Password_Change_Design_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'id_name'	=> Array(
										'default'	=> 'System Password Change Design',
									   ),
					  );

	}//end constructor


}//end class
?>
