<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: page_multiple_page_page_management.inc,v 1.3 2006/12/07 00:04:21 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Multiple_Page_Page_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage CMS
*/
class Page_Multiple_Page_Page_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Multiple_Page_Page_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['name']['default'] = '';

	}//end constructor


}//end class

?>