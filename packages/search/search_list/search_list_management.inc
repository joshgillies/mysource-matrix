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
* $Id: search_list_management.inc,v 1.1.6.1 2005/05/01 23:33:07 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_List_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_List_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Search_List_Management(&$pm)
	{
		$this->Asset_Management($pm);
		
		$this->vars['default_format']	= Array(
											'added'     => '0.1',
											'type'      => 'wysiwyg',
											'default'   => '%search_term% ( %number_of_occurrences% )',
										);
										
		$this->vars['search_field']	= Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => '',
										);
		
	}//end constructor


}//end class

?>