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
* $Id: form_question_type_select_management.inc,v 1.4 2006/12/06 00:35:31 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Question_Type_Select_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Select_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Select_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'height'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '1',
										   ),
						'default'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'multiple'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'extra'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'options'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'option_list',
											'default'	=> '',
										   ),
						'allow_empty'	=> Array(
											'added'			=> '0.3',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'empty_text'	=> Array(
											'added'		=> '0.3',
											'type'		=> 'text',
											'default'	=> '-- Leave Empty --',
										   ),
					  );

	}//end constructor


}//end class
?>
