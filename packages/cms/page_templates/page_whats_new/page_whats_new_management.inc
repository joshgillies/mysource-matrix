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
* $Id: page_whats_new_management.inc,v 1.15 2006/01/20 04:21:01 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Whats_New_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Whats_New_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Whats_New_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'date_field'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'updated',
												'parameters'	=> Array(
																	'options'	=> Array(
																					'created'	=> 'Created Date',
																					'updated'	=> 'Last Updated Date',
																					'published'	=> 'Last Published Date',
																				   ),
																   ),
												'multiple'		=> FALSE,
												'allow_empty'	=> FALSE,
											   ),
						'date_range_from'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'datetime',
												'default'		=> '---------- --:--:--',
												'parameters'	=> Array(
																	'allow_null'	=> TRUE,
																   ),
											   ),
						'date_range_to'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'datetime',
												'default'		=> '---------- --:--:--',
												'parameters'	=> Array(
																	'allow_null'	=> TRUE,
																   ),
											   ),
						'date_ranges'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
					  );

	}//end constructor


	/**
	* Return an array with information about the events that the what new asset is listening for
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
