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
* $Id: page_hcl_management.inc,v 1.4 2004/02/10 16:32:41 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

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
class Page_HCL_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function Page_HCL_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('type'	=> Array(
										'added'	=> '0.0.1',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												'rdf'	=> 'RDF',
												'xml'	=> 'XML',
											),
											'multiple' => false,
										),

										'default' => 'xml',
							),
		);

	}//end constructor

}//end class

?>