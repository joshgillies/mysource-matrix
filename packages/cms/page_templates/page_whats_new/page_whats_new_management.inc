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
* $Id: page_whats_new_management.inc,v 1.4 2004/09/15 01:24:33 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Whats_New_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
				'date_field' => Array(
						'added'   => '0.1',
						'type'    => 'selection',
						'default' => 'updated',
						'parameters' => Array(
											'options' => Array(
												'created'	=> 'Created Date',
												'updated'	=> 'Last Updated Date',
												'published'	=> 'Last Published Date',
												),
											),
											'multiple' => false,
											'allow_empty' => false,
										),
				'date_range_from' => Array(
						'added'		=> '0.1',
						'type'		=> 'datetime',
						'default'	=> '---------- --:--:--',
						'parameters' 	=> Array(
							'allow_null' => true,
						),
					),
				'date_range_to' => Array(
						'added'		=> '0.1',
						'type'		=> 'datetime',
						'default'	=> '---------- --:--:--',
						'parameters' 	=> Array(
							'allow_null' => true,
						),
					),
				'date_ranges' => Array(
					'added'		=> '0.1',
					'type'		=> 'serialise',
					'default'	=> Array(),
					),
				
		);
		
		
	}//end constructor
	
	/**
	* Return an array with information about the events that the what new asset is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'				 => 'requestKeywords',
					'broadcast_type_code'		 => 'content_type_wysiwyg',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'side_of_link'		=> 'major',
						'indirect'			=> true,
						'exclusive'			=> null,
						'dependant'			=> 1,
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


}//end class

?>