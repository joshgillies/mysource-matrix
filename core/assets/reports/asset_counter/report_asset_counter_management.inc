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
* $Id: report_asset_counter_management.inc,v 1.2 2004/10/26 01:31:01 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Asset_Counter_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Asset_Counter_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Report_Asset_Counter_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['types']        = Array(
			'added'   => '1.1',
			'type'    => 'serialise',
			'default' => Array(),
		);

		$this->vars['sort_by']		= Array(
			'added'	=> '0.0.1',
			'type'	=> 'selection',
			'parameters' => Array(
				'options' => Array(
					'count'		=> 'Count',
					'name'		=> 'Name',
					'type'	    => 'Code',
				),

				'multiple' => false,
				'allow_empty' => false,

			),
			'default' => 'type'
		);

		$this->vars['reverse_sort']	= Array(
			'added'     => '0.1',
			'type'      => 'boolean',
			'default'   => false,
			'parameters' => Array(
				'allow_empty' => false
			),
		);

	}//end constructor


}//end class

?>