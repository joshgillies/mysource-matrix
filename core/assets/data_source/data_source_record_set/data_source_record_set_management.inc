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
* $Id: data_source_record_set_management.inc,v 1.2 2006/12/05 05:53:09 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Record_Set_Management
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage core
*/
class Data_Source_Record_Set_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Record_Set_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
