<?php

/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: search_page_management.inc,v 1.3 2003/11/18 15:45:00 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

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

require_once SQ_PACKAGES_PATH.'/cms/page/page_management.inc';
class Search_Page_Management extends Page_Management
{

	/**
	* Constructor
	*
	*/
	function Search_Page_Management(&$pm)
	{
		$this->Page_Management($pm);

		$this->vars = array(
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
			'fields' => Array(
				'added'			=> '0.1',
				'type'			=> 'serialise',
				'default'		=> array(),
				'description'	=> 'Search fields.',
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

}//end class

?>