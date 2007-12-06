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
* $Id: design_area_login_form_management.inc,v 1.10 2007/12/06 04:28:49 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Login_Form_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Login_Form_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'login_contents'		=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
													'description'	=> 'Holds the processed contents array for the login section',
												   ),
						'logout_contents'		=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
													'description'	=> 'Holds the processed contents array for the logout section',
												   ),
						'give_username_focus'	=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'Whether to give the username input box focus on load of the page',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'forward_www_vars'		=> Array(
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
