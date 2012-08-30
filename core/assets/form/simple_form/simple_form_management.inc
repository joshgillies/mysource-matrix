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
* $Id: simple_form_management.inc,v 1.9 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Simple_Form_Management
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Simple_Form_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object Package_Manager	&$pm	the relevant package manager
	*
	* @return void
	* @access public
	*/
	function Simple_Form_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> FALSE,
											'is_contextable'=> TRUE,
										   ),
						'questions'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'next_question'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'int',
											'default'	=> 1,
										   ),
						'submit_button'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Submit',
											'is_contextable'	=> TRUE,
										   ),
						'reset_button'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Clear Form',
											'is_contextable'	=> TRUE,
										   ),
						'client_side'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'disable_keyword_replacements'		=> Array(
															'added'			=> '0.1',
															'type'			=> 'boolean',
															'default'		=> TRUE,
															'description'	=> 'If true, will not replace any keywords from user input',
															'parameters'	=> Array(
																				'allow_empty'	=> FALSE,
													   						   ),
													  	),
					  );

	}//end constructor


	/**
	* Gets a list of all files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'js/sort_order_manip.js',
			   );

	}//end _getFileList()


	/**
	* returns an array with information about the events that the form is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


}//end class


?>
