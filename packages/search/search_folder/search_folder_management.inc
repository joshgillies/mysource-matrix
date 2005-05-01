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
* $Id: search_folder_management.inc,v 1.2.6.1 2005/05/01 23:33:07 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Folder_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Folder_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Search_Folder_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('settings' => Array(
											'added'     => '0.1',
											'type'      => 'serialise',
											'default'   => Array()
											),
							);
	}//end constructor


}//end class

?>