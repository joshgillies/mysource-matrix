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
* $Id: form_email_management.inc,v 1.23 2006/01/18 04:31:45 sdanis Exp $
*
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Email_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Email_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Email_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'receipt_emails_format'		=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'email_format',
														'default'	=> Array(),
													   ),
						'receipt_field'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'assetid',
														'default'	=> '',
													   ),
						'recipient_emails_format'	=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'email_format',
														'default'	=> Array(),
													   ),
						'staf_field'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'assetid',
														'default'	=> '',
													   ),
						'staf_format'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'email_format',
														'default'	=> Array(),
													   ),
						'selective_emails'			=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'serialise',
														'default'	=> Array(),
													   ),
						'log'						=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
					  );

	}//end constructor


	/**
	* returns an array with information about the events that the form is listening for
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
