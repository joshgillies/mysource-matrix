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
* $Id: page_hcl_management.inc,v 1.2 2004/01/13 10:40:55 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page/page_management.inc';
/**
* Page_HCL_Management
*
* @see Asset_Management
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Version$ - 0.1
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_HCL_Management extends Page_Management
{

	/**
	* Constructor
	*
	*/
	function Page_HCL_Management(&$pm)
	{
		$this->Page_Management($pm);

		$this->vars = Array('rootid' => Array (
										'added'		=> '0.0.1',
										'type'		=> 'assetid',
										'default'	=> '',
								), // end name
							'schemas'	=> Array (
										'added'	=> '0.0.1',
										'type'	=> 'serialise',
										'default' => Array(),
							),
							'type'	=> Array(
										'added'	=> '0.0.1',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												1	=> 'RDF',
												2	=> 'XML',
											),
											'multiple' => false,
										),
										'default' => 2,
							),
		);

	}//end constructor

}//end class

?>