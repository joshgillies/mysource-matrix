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
* $Id: bulkmail_user_management.inc,v 1.5 2006/01/05 22:39:05 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bulkmail_User_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_User_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_User_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'email'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'email',
									'default'		=> '',
									'uniq'			=> TRUE,
									'description'	=> 'The users email address',
									'is_admin'		=> 0,
								   ),
						'name'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'The users full name',
									'is_admin'		=> 0,
								   ),
					  );

	}//end constructor


}//end class
?>