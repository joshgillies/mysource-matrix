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
* $Id: form_question_type_option_list_management.inc,v 1.4 2006/12/06 00:35:31 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Question_Type_Option_List_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Option_List_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Option_List_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'options'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'option_list',
										'default'	=> '',
									   ),
						'default'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									   ),
						'extra'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'required'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'boolean',
										'default'		=> FALSE,
										'parameters'	=> Array(
															'allow_empty'	=> FALSE,
														   ),
										'description'	=> 'DEPRECATED AS OF v0.3',
									   ),
					  );

	}//end constructor


}//end class
?>
