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
* $Id: form_question_type_tickbox_table_management.inc,v 1.8 2005/05/20 05:27:37 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_Tickbox_Table_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Tickbox_Table_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Tickbox_Table_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'question_contents'	=> Array(
												'added'		=> '0.0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'required'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> false,
												'parameters'	=> Array(
																	'allow_empty'	=> false,
																   ),
												'description'	=> 'DEPRECATED AS OF v0.3',
											   ),
						'default'			=> Array(
												'added'		=> '1.3',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
					  );

	}//end constructor


	/**
	* Gets a list of all files
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList()
	{
		return Array(
				'files/cross.png',
				'files/cross2.png',
				'files/InputTable.js',
				'files/style.css',
			   );

	}//end _getFileList()


}//end class
?>
