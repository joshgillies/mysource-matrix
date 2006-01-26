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
* $Id: user_management.inc,v 1.29 2006/01/26 22:34:07 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function User_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'username'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
											'uniq'		=> TRUE,
											'is_admin'	=> 0,
										   ),
						'password'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'password',
											'default'	=> '',
										   ),
						'first_name'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
											'is_admin'	=> 0,
										   ),
						'last_name'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
											'is_admin'	=> 0,
										   ),
						'email'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'email',
											'default'	=> '',
											'is_admin'	=> 0,
										   ),
						'locale'		=> Array(
											'added'		=> '0.3',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'restrictions'	=> Array(
											'added'		=> '0.2',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
					  );

	}//end constructor


}//end class

?>