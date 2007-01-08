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
* $Id: report_safe_trash_management.inc,v 1.1 2007/01/08 03:08:09 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Safe_Trash_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Safe_Trash_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Report_Safe_Trash_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['types']	= Array(
									'added'		=> '0.1',
									'type'		=> 'serialise',
									'default'	=> Array(),
								  );

	}//end constructor


}//end class

?>