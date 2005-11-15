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
* $Id: bulkmail_post_office_management.inc,v 1.12 2005/11/15 00:05:48 rong Exp $
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
* @version $Revision: 1.12 $
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
						'content_id'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The asset we will be using as content in the email',
											   ),
						'content_gen'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'one_user',
												'description'	=> 'The method of content generation',
												'parameters'	=> Array(
																	'multiple'		=> false,
																	'allow_empty'	=> false,
																	'options'		=> Array(
																						'each_user'	=> 'Generate as recipient',
																						'one_user'	=> 'Generate as set user',
																					   ),
																   ),
											   ),
						'content_gen_as'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The user to generate the content with',
											   ),
						'content_design'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The design to use for the content asset',
											   ),
						'content_layout'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The layout to use for the content asset',
											   ),
						'server_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array('driver' => 'smtp'),
												'description'	=> 'Details about which email server the bulkmails should be sent to',
											   ),
						'from'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> SQ_CONF_DEFAULT_EMAIL,
												'description'	=> 'The "From" header field required by PEAR send() function',
											   ),
						'subject'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The "Subject" header field required by PEAR send() function',
											   ),
						'header_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Extra header detail fields',
											   ),
						'threshold'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Threshold rules that will be enforced by the Bulk Mailer when it runs',
											   ),
					  );

	}//end constructor


}//end class
?>