<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: user_management.inc,v 1.32 2013/09/12 06:42:54 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.32 $
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
		    			'history_passwords'	=> Array(
											'added'		=> '0.2',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
		    			'login_date'	=> Array(
											'added'		=> '0.2',
											'type'		=> 'datetime',
											'default'	=> '---------- --:--:--',
											'is_admin'	=> 0,
										   ),
		    			'login_ip'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
											'is_admin'	=> 0,
										   ),
		    			'disallow_password_login'	=> Array(
										'added'		=> '0.3',
										'type'		=> 'boolean',
										'default'		=> FALSE,
										'parameters'    => Array(
												'allow_empty'   => FALSE,
										    ),
									    ),
					  );

	}//end constructor


}//end class

?>
