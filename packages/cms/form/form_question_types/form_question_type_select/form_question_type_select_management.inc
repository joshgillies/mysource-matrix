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
* $Id: form_question_type_select_management.inc,v 1.13.6.3 2005/12/06 05:18:46 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_Select_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.13.6.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
						'height'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '1',
									   ),
						'default'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									   ),
						'multiple'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'boolean',
										'default'		=> false,
										'parameters'	=> Array(
															'allow_empty'	=> false,
														   ),
									   ),
						'extra'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'options'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'option_list',
										'default'	=> '',
									   ),
						'required'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'boolean',
										'default'		=> false,
									   ),
					  );

	}//end constructor


}//end class
?>
