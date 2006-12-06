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
* $Id: login_design_management.inc,v 1.6 2006/12/06 04:40:08 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Login_Design_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.6 $
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
						'id_name'	=> Array(
										'default'	=> 'System Login Design',
									   ),
					  );

	}//end constructor


}//end class
?>
