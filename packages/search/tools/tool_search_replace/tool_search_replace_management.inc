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
* $Id: tool_search_replace_management.inc,v 1.2 2005/04/29 05:38:41 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Search_Replace_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Tool_Search_Replace_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Search_Replace_Management(&$pm)
	{
		$this->Asset_Management($pm);


	}//end constructor


}//end class

?>
