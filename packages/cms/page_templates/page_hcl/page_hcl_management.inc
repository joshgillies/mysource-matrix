<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_hcl_management.inc,v 1.5.6.1 2005/05/02 02:44:58 gsherwood Exp $
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
											'allow_empty' => false,
										),

										'default' => 'xml',
							),
		);

	}//end constructor

}//end class

?>