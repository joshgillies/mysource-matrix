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
* $Id: form_question_type_datetime_management.inc,v 1.12 2005/04/29 05:40:29 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_Datetime_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Datetime_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Datetime_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'default'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'datetime',
										'default'		=> '---------- --:--:--',
										'parameters'	=> Array(
															'allow_null'	=> true,
														   ),
									   ),
						'extra'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'circa'		=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'boolean',
										'default'		=> false,
										'parameters'	=> Array(
															'allow_empty'	=> false,
														   ),
									   ),
						'show'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(
														'd'	=> 1,
														'm'	=> 1,
														'y'	=> 1,
														'h'	=> 1,
														'i'	=> 1,
														's'	=> 1,
														'c'	=> 0,
													   ),
									   ),
						'null'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(
														'd'	=> 0,
														'm'	=> 0,
														'y'	=> 0,
														'h'	=> 0,
														'i'	=> 0,
														's'	=> 0,
													   ),
									   ),
						'text'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(
														'd'	=> 0,
														'm'	=> 0,
														'y'	=> 0,
														'h'	=> 0,
														'i'	=> 0,
														's'	=> 0,
													   ),
									   ),
						'minimum'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'datetime',
										'default'		=> '---------- --:--:--',
										'parameters'	=> Array(
															'allow_null'	=> true,
															'show'			=> Array('d', 'y'),
														   ),
									   ),
						'maximum'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'datetime',
										'default'		=> '---------- --:--:--',
										'parameters'	=> Array(
															'allow_null'	=> true,
														   ),
									   ),
					  );

	}//end constructor


}//end class
?>
