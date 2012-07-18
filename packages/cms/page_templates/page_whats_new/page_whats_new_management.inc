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
* $Id: page_whats_new_management.inc,v 1.20 2012/07/18 04:01:55 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Whats_New_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Whats_New_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		$this->vars = Array(
						'date_field'				=> Array(
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
						'range_type'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> 'default',
													   ),
						'date_range_from'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'datetime',
														'default'		=> '---------- --:--:--',
														'parameters'	=> Array(
																			'allow_null'	=> TRUE,
																		   ),
													   ),
						'date_range_to'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'datetime',
														'default'		=> '---------- --:--:--',
														'parameters'	=> Array(
																			'allow_null'	=> TRUE,
																		   ),
													   ),
						'date_ranges'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
													   ),
						'link_value_filter'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> '',
													   ),
						'link_value_filter_enabled' => Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																	  	   ),
													   ),
						'link_value_filter_logic'	=> Array(
														'added'			=> '0.16',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
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
