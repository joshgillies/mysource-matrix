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
* $Id: content_type_code_management.inc,v 1.1 2005/12/16 05:42:13 dmckee Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Code_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.1 $
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
					  );

	}//end constructor


}//end class

?>