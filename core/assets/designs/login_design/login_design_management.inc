<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: login_design_management.inc,v 1.4 2003/10/15 11:05:14 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Login_Design_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Login_Design_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Login_Design_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'id_name'   => Array(
								'default'   => 'System Login Design',
							),
						);

	}//end constructor


}//end class
?>
