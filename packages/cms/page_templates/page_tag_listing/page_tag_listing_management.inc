<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: page_tag_listing_management.inc,v 1.4 2012/08/30 00:58:09 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Tag_Listing_Management
*
* @see Asset_Management
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Tag_Listing_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);
		$this->vars = Array(
							'tag_max_size' => Array(
								'added'			=>	'0.0.1',
								'type'			=>	'int',
								'default'		=>	'10',
							),
							'first_page_only' => Array(
								'added'			=>	'0.0.1',
								'type'			=>	'boolean',
								'default'		=>	FALSE,
								'parameters'	=> Array(
									'allow_empty' => FALSE,
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