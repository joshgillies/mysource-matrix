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
* $Id: form_question_type_tickbox_list_management.inc,v 1.16.2.1 2005/12/08 00:36:29 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_Tickbox_List_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.16.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Tickbox_List_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Tickbox_List_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'options'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'option_list',
											'default'	=> '',
										   ),
						'default'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'extra'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'required'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> false,
											'parameters'	=> Array(
																'allow_empty'	=> false,
															   ),
											'description'	=> 'DEPRECATED AS OF v0.3',
										   ),
					  );
	}//end constructor


}//end class
?>
