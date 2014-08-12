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
* $Id: form_question_type_country_management.inc,v 1.3 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Type_Country_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Country_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Country_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
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
						'default'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									   ),
						'available'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									   ),
					  );

	}//end constructor


}//end class
?>
