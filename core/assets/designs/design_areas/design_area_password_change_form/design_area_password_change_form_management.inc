<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: design_area_password_change_form_management.inc,v 1.3 2006/02/27 04:28:37 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Password_Change_Form_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Password_Change_Form_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Password_Change_Form_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'password_change_contents'	=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'Holds the processed contents array for the password change section',
													   ),
						'give_password_old_focus'	=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'Whether to give the password box focus on load of the page',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'forward_www_vars'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'When true will attempt to forward all the GET and POST vars (unable to files)',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
					  );

	}//end constructor


}//end class
?>
