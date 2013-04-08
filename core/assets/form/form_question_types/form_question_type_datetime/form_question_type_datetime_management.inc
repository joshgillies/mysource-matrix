<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_datetime_management.inc,v 1.8 2013/04/08 07:52:53 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Question_Type_Datetime_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
						'default'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array(
																'allow_null'	=> TRUE,
															   ),
										   ),
						'extra'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'circa'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'show'			=> Array(
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
										   					'a' => 0,
														   ),
										   ),
						'null'			=> Array(
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
						'text'			=> Array(
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
						'minimum'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array(
																'allow_null'	=> TRUE,
																'show'			=> Array('d', 'y'),
															   ),
										   ),
						'maximum'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'datetime',
											'default'		=> '---------- --:--:--',
											'parameters'	=> Array(
																'allow_null'	=> TRUE,
															   ),
										   ),
						'print_format'	=> Array(
											'added'		=> '0.3',
											'type'		=> 'text',
											'default'	=> 'Y-m-d H:i:s',
										   ),
						'extras'		=> Array(
											'added'		=> '0.3',
											'type'		=> 'serialise',
											'default'	=> Array(
															'd'	=> '',
															'm'	=> '',
															'y'	=> '',
															'h'	=> '',
															'i'	=> '',
															's'	=> '',
										   					'a' => '',
														   ),
										   ),
						'use_js'			=> Array(
											'added'			=> '0.4',
											'type'			=> 'boolean',
											'default'		=> TRUE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
					  );

	}//end constructor


}//end class
?>
