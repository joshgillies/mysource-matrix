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
