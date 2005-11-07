<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: bulkmail_post_office_management.inc,v 1.8 2005/11/07 00:45:37 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Bulkmail_Post_Office_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Post_Office_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_Post_Office_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'recipients'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The recipients to send this bulkmail to. All users/bulkmail_users in each group will become recipients',
											   ),
						'content_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Details about content page to use and the content generation method for users',
											   ),
						'server_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array('driver' => 'smtp'),
												'description'	=> 'Details about which email server the bulkmails should be sent to',
											   ),
						'required_header'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(
																	'From'		=> SQ_CONF_DEFAULT_EMAIL,
																	'Subject'	=> '',
																   ),
												'description'	=> 'The required header information for PEAR send() function',
											   ),
						'header_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Extra header information',
											   ),
						'threshold'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(
																	'type'		=> 'asap',
																	'value'		=> '',
																   ),
												'description'	=> 'Whether to spread out the sending over x amount of time',
											   ),
						'preview_log'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Log of the last 5 preview records',
											   ),
					  );

	}//end constructor


}//end class
?>