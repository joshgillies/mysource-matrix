<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_asset_listing_management.inc,v 1.25 2005/03/23 03:56:35 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Asset_Listing_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Asset_Listing_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['prev_page_text']	= Array(
										'added'   => '0.1',
										'type'    => 'text',
										'default' => 'Previous'
										);

		$this->vars['next_page_text']	= Array(
										'added'   => '0.1',
										'type'    => 'text',
										'default' => 'Next'
										);

		$this->vars['parameter_map'] = Array(
										'added'	=> '0.4',
										'type'  => 'parameter_map',
										'default' => Array(),
										'parameters' => Array(
											'parameters'=> Array(
												'root_node' => 'Replacement Root node for the listing (must be a child of the static root node)',
												)
											)
										);

	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'                 => 'requestKeywords',
					'broadcast_type_code'        => 'content_type',
					'broadcast_strict_type_code' => false,
					'options'	=> Array(
						'side_of_link' => 'major',
						'indirect'     => true,
						'is_exclusive' => null,
						'is_dependant' => 1,
						'value'        => '',
						'link_type'    => SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


}//end class

?>
