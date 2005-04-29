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
* $Id: form_question_rule_management.inc,v 1.10 2005/04/29 05:40:29 gsherwood Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Rule_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Rule_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'custom_error'	=> Array(
											'added'		=> '0.2',
											'type'		=> 'text',
											'default'	=> '',
										   ),
					  );

	}//end constructor


}//end class
?>
