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
* $Id: data_record_management.inc,v 1.2 2006/12/05 05:53:08 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Record_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Record_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Record_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'	=> Array(
									'added'			=> '0.0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'The full name of the record',
									'update_fn'		=> '',
									'is_admin'		=> FALSE,
								   ),
					  );

	}//end constructor


}//end class
?>
