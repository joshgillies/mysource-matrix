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
* $Id: bulkmail_subscribe_page_management.inc,v 1.9 2013/03/04 22:44:09 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Bulkmail_Subscribe_Page_Edit_Fns
*
* @see Page_User_Group_Manager_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Subscribe_Page_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		$this->vars = Array(
						'verify_logged_in'		=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'selection',
													'default'		=> 'never',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'never'			=> 'Do not verify',
																							'always'		=> 'Always verify',
																							'subscribe'		=> 'Verify on subscription',
																							'unsubscribe'	=> 'Verify on unsubscription',
																						   ),
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																	   ),
													'description'	=> '',
												   ),
						'verify_not_logged_in'	=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'selection',
													'default'		=> 'never',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'never'			=> 'Do not verify',
																							'always'		=> 'Always verify',
																							'subscribe'		=> 'Verify on subscription',
																							'unsubscribe'	=> 'Verify on unsubscription',
																						   ),
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																	   ),
													'description'	=> '',
												   ),
						'verify_from_address'	=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'text',
													'default'		=> SQ_CONF_DEFAULT_EMAIL,
													'description'	=> 'The email address to appear on the verification email.',
												   ),
						'verify_subject_line'	=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The configurable subject line for the email sent for verification',
												   ),
					  );


		$this->vars['require_captcha'] = Array(
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										 );
		$this->vars['captcha_length'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 4,
										);
		$this->vars['captcha_zoom'] = Array(
										'added'		=> '0.9',
										'type'		=> 'int',
										'default'	=> 1,
									  );
		$this->vars['captcha_link_text'] = Array(
											'added'		=> '0.9',
											'type'		=> 'text',
											'default'	=> 'Regenerate security key',
										   );

		$this->vars['captcha_bgcolour']	= Array(
											'added'		=> '0.9',
											'type'		=> 'colour',
											'default'	=> 'FFFFFF',
										  );

		$this->vars['captcha_textcolour']	= Array(
												'added'		=> '0.9',
												'type'		=> 'colour',
												'default'	=> '000000',
											  );

		$this->vars['captcha_bordercolour'] = Array(
												'added'		=> '0.9',
												'type'		=> 'colour',
												'default'	=> '000000',
											  );

		$this->vars['captcha_use_colours']	= Array(
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											  );

		$this->vars['captcha_usefont']	= Array(
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'=> FALSE,
															   ),
										  );

		$this->vars['captcha_ttf']	= Array(
										'added'			=> '0.9',
										'type'			=> 'assetid',
										'default'		=> '',
										'description'	=> 'The asset id of the font file.',
									  );

		$this->vars['captcha_font_size'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 20,
										   );

		$this->vars['captcha_min_angle'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> -20,
										   );

		$this->vars['captcha_max_angle'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 20,
										   );

		$this->vars['captcha_x_start'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 10,
										 );

		$this->vars['captcha_min_distance'] = Array(
												'added'		=> '0.9',
												'type'		=> 'int',
												'default'	=> 10,
											  );

		$this->vars['captcha_max_distance'] = Array(
												'added'		=> '0.9',
												'type'		=> 'int',
												'default'	=> 20,
											  );

		$this->vars['captcha_width'] = Array(
										'added'		=> '0.9',
										'type'		=> 'int',
										'default'	=> 200,
									   );

		$this->vars['captcha_height'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 36,
										);

		$this->vars['captcha_arc'] = Array(
										'type'=> 'boolean',
										'default'=> FALSE,
										'parameters'=> Array(
														'allow_empty'=> FALSE,
													   ),
									 );

		$this->vars['captcha_arc_colour'] = Array(
												'added'=> '0.9',
												'type'=> 'colour',
												'default'=> 'D20A0A',
											);

		$this->vars['captcha_include_uppercase'] = Array(
													'type'=> 'boolean',
													'default'=> FALSE,
													'parameters'=> Array(
																	'allow_empty'=> FALSE,
																   ),
												   );

		$this->vars['captcha_include_numbers'] = Array(
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												 );

		$this->vars['provide_accessible_captcha'] = Array(
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													 );

		$this->vars['captcha_invalid_error_message']		= Array(
																'added'			=> '0.9',
																'type'			=> 'text',
																'default'		=> 'Security Key Incorrect',
																'is_contextable'=> TRUE,
															  );

		$this->vars['captcha_label_text']					= Array(
																'added'			=> '0.9',
																'type'			=> 'text',
																'default'		=> 'Type the characters you see in the picture above: ',
																'is_contextable'=> TRUE,
															  );
		$this->vars['use_recaptcha'] = 				Array(
														'added'			=> '0.9',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																				),
													);
		$this->vars['recaptcha_theme'] = 			Array(
														'added'			=> '0.9',
														'type'			=> 'selection',
														'default'		=> 'red',
														'parameters'	=> Array(
																			'multiple'		=> FALSE,
																			'allow_empty'   => FALSE,
																			'options'       => Array(
																								'red'	=> 'Red',
																								'white' => 'White',
																								'blackglass' => 'Black Glass',
																								'clean'	=> 'Clean',
																								),
																				),
													);
		$this->vars['recaptcha_error_message'] = 	Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Error message when Recaptcha validation fails',
													);
		$this->vars['honeypot_name'] = 				Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Honeypot input field name',
													);
		$this->vars['honeypot_id']	= 				Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Honeypot input field id',
													);
		$this->vars['honeypot_class'] =				Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Honeypot input field class name',
													);
		$this->vars['honeypot_error_message'] = 	Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Error message when Honeypot validation fails',
													);

	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
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
