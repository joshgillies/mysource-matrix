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
* $Id: search_page_management.inc,v 1.10 2004/02/19 02:27:43 diarandine Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Page_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Search_Page_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
			'root_assetid' => Array(
				'added'			=> '0.1',
				'type'			=> 'int',
				'default'		=> '',
				'description'	=> 'The asset ID of the root asset on which searches are based on.',
			),
			'results_per_page' => Array (
				'added'			=> '0.1',
				'type'			=> 'int',
				'default'		=> 10,
				'description'	=> 'Default results per page when searching.',
			),
			'lineage_seperator' => Array (
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> ' > ',
				'description'	=> 'Seperator to use when printing the lineage to a search result asset.',
			),
			'fields' => Array(
				'added'			=> '0.1',
				'type'			=> 'serialise',
				'default'		=> array(),
				'description'	=> 'Search fields.',
			),
			'search_types' => Array(
				'added'			=> '0.1',
				'type'			=> 'serialise',
				'default'		=> array(),
				'description'	=> 'Searchable asset types',
			),
			'search_styles' => Array(
				'added'			=> '0.1',
				'type'			=> 'serialise',
				'default'		=> array(),
				'description'	=> 'Styles of the search fields',
			),
			'logic' => Array(
				'added'			=> '0.1',
				'type'			=> 'selection',
				'default'		=> 'AND',
				'description'	=> 'Whether to include all or any fields.',
				'parameters'	=> Array(
					'options' => Array(
						'AND'	=> 'Match on ALL fields', 
						'OR'	=> 'Match on at least ONE field',
					),
				),
			),
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