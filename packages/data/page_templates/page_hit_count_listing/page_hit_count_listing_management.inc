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
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Asset_Listing_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @author  Christophe Olivar <colivar@squiz.net>
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Page_Hit_Count_Listing_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Hit_Count_Listing_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['display_all_assets'] = Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array( 'allow_empty' => FALSE,),
											);

		$this->vars['min_hitcount'] = Array(
										'added'		=> '0.1',
										'type'		=> 'int',
										'default'	=> '-1',
									  );

		$this->vars['max_hitcount'] = Array(
										'added'		=> '0.1',
										'type'		=> 'int',
										'default'	=> '-1',
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
