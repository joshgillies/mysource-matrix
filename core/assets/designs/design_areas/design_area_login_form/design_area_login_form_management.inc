<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Login_Form_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Login_Form_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function Design_Area_Login_Form_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'login_contents' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'	=> 'Holds the processed contents array for the login section'
							),
							'logout_contents' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'	=> 'Holds the processed contents array for the logout section'
							),
							'give_username_focus' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'Whether to give the username input box focus on load of the page',
							),
							'forward_www_vars' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'When true will attempt to forward all the GET and POST vars (unable to files)',
							),
						);

	}//end constructor


}//end class
?>
