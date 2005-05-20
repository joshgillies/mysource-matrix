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
* $Id: form_question_type_numeric_management.inc,v 1.7 2005/05/20 05:27:37 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_Text_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Numeric_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Numeric_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'default'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'width'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '30',
									   ),
						'height'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '1',
									   ),
						'max'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '300',
									   ),
						'extra'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'required'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'boolean',
										'default'		=> false,
										'parameters'	=> Array(
															'allow_empty'	=> false,
														   ),
										'description'	=> 'DEPRECATED AS OF v0.3',
									   ),
						'is_int'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'boolean',
										'default'		=> false,
										'parameters'	=> Array(
															'allow_empty'	=> false,
														   ),
									   ),
					  );
	}//end constructor


}//end class
?>
