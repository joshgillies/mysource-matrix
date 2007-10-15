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
* $Id: rtf_file_management.inc,v 1.1 2007/10/15 05:51:27 bshkara Exp $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Rtf_File_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Rtf_File_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Rtf_File_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars	= Array (
					  );

	}//end constructor


}//end class
?>
