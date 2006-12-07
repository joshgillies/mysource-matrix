<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_user_management.inc,v 1.2 2006/12/07 03:13:05 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ipb_User_Management
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_User_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ipb_User_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'mgroup'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'email'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'email',
												'default'	=> '',
											   ),
						'title'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'member_login_key'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'date_of_birth'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'photo_type'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'photo_location'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'aim_name'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'icq_number'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'website'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'yahoo'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'interests'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'msnname'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'vdirs'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'location'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'signature'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'avatar_location'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'avatar_size'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'avatar_type'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
					  );

	}//end constructor


}//end class

?>