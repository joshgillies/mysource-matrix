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
* $Id: form_question_type_tickbox_table_management.inc,v 1.3 2006/12/06 00:35:32 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Question_Type_Tickbox_Table_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'DEPRECATED AS OF v0.3',
											   ),
						'default'			=> Array(
												'added'		=> '0.3',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'extra'				=> Array(
												'added'		=> '0.4',
												'type'		=> 'text',
												'default'	=> '',
											   ),
					  );

	}//end constructor


	/**
	* Gets a list of all files
	*
	* @return array
	* @access private
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
