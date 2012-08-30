<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: physical_file_management.inc,v 1.2 2012/08/30 00:58:36 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Physical_File_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class Physical_File_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Physical_File_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
					  );

	}//end constructor


}//end class
?>
