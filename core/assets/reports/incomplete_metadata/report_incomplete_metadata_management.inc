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
* $Id: report_incomplete_metadata_management.inc,v 1.6 2006/12/06 02:59:55 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Incomplete_Metadata_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Incomplete_Metadata_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Report_Incomplete_Metadata_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['types']	= Array(
									'added'		=> '0.2',
									'type'		=> 'serialise',
									'default'	=> Array(),
								  );

	}//end constructor


}//end class

?>