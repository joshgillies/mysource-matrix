<?php

/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: search_page_management.inc,v 1.2 2003/10/21 06:26:02 dwong Exp $
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