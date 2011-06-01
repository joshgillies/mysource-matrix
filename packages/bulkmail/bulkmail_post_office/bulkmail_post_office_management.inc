<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: bulkmail_post_office_management.inc,v 1.23 2008/11/24 22:28:20 mbrydon Exp $
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
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Post_Office_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		if (!strlen(SQ_CONF_DEFAULT_EMAIL)) {
			trigger_localised_error('SYS0315', E_USER_WARNING);
		}

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
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
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
												'default'		=> Array('driver' => 'sendmail'),
												'description'	=> 'Details about which email server the bulkmails should be sent to',
											   ),
						'from'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'email',
												'default'		=> SQ_CONF_DEFAULT_EMAIL,
												'description'	=> 'Sending address as per RFC822',
											   ),
						'subject'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Subject field of the email',
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
						'threshold_types'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(
																	// bulkmail_threshold_rule_excluded_time
																	0	=> Array(
																			'multiple'	=> TRUE,
																			'params'	=> Array(
																							'from'	=> 0,
																							'to'	=> 0,
																							'days'	=> Array(),
																						   ),
																		   ),
																	// bulkmail_threshold_rule_send_rate
																	1	=> Array(
																			'multiple'	=> FALSE,
																			'params'	=> Array(
																							'value'	=> 60,
																							'unit'	=> 60,
																						   ),
																		   ),
																   ),
												'description'	=> 'Threshold rule types and their default values',
											   ),
						'bulkmail_mode'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'live',
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'live'	=> 'Live Mode',
																						'test'	=> 'Test Mode',
																					   ),
																   ),
											   ),
					  );

	}//end constructor


}//end class
?>
