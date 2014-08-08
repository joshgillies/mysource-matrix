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
* $Id: page_password_reset_management.inc,v 1.7 2013/08/09 01:59:56 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Password_Reset_Management
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Password_Reset_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		$this->vars['request_email_format'] = Array(
												'added'		=> '0.1',
												'type'		=> 'email_format',
												'default'	=> Array(),
											  );
		$this->vars['invalid_user_error']	= Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'false',
													'parameters'    => Array( 'allow_empty' => FALSE,),
													'description'	=> 'Whether to error out if an invalid username is supplied for password reset request',
												   );
		$this->vars['reset_in_same_session']	= Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'true',
													'parameters'    => Array( 'allow_empty' => FALSE,),
													'description'	=> 'Whether to check if user opens the reset link in same session of request',
												   );
		$this->vars['password_autocomplete']	= Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												  );
											  

	}//end constructor


	/**
	* Return an array with information about the events that the account manager is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


}//end class

?>
