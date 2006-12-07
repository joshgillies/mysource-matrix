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
* $Id: content_type_code_management.inc,v 1.3 2006/12/07 00:04:16 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Code_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Code_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Code_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'edit_content'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'update_fn'		=> '',
											'description'	=> 'The raw content of the editor',
										   ),
						'line_numbers'	=> Array(
											'name'			=> 'Line Number Options',
											'type'			=> 'serialise',
											'description'	=> 'Whether to use line numbers, and what options to use',
											'default'		=> Array(),
											'protected'		=> FALSE,
										   ),
					  );

	}//end constructor


}//end class

?>