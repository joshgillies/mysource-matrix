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
* $Id: user_management.inc,v 1.19 2004/02/10 04:13:30 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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

		$this->vars =	Array(
							'username' => Array(
								'added'     => '0.1',
								'type'      => 'text',
								'default'   => '',
								'uniq'      => true,
							),
							'password' => Array(
								'added'     => '0.1',
								'type'      => 'password',
								'default'   => '',
							),
							'first_name' => Array(
								'added'     => '0.1',
								'type'      => 'text',
								'default'   => '',
							),
							'last_name' => Array(
								'added'     => '0.1',
								'type'      => 'text',
								'default'   => '',
							),
							'email'     => Array(
								'added'     => '0.1',
								'type'      => 'email',
								'default'   => '',
							),
						);

		$this->components['name']['default_weighting'] = '1';
		$this->components['username'] = Array (
											'description'      => 'Username',
											'default_weighting' => '1',
											);
		$this->components['first_name'] = Array (
											'description'      => 'First Name',
											'default_weighting' => '1',
											);
		$this->components['last_name'] = Array (
											'description'      => 'Last Name',
											'default_weighting' => '1',
											);
		$this->components['email'] = Array (
											'description'      => 'Email Address',
											'default_weighting' => '1',
											);

	}//end constructor


}//end class

?>