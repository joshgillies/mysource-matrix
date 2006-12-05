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
* $Id: design_area_password_change_form_management.inc,v 1.4 2006/12/05 22:42:14 emcdonald Exp $
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
* @version $Revision: 1.4 $
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
