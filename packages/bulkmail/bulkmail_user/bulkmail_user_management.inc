<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: bulkmail_user_management.inc,v 1.7 2007/12/05 03:31:41 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bulkmail_User_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_User_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		$this->vars = Array(
						'email'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'email',
									'default'		=> '',
									'uniq'			=> TRUE,
									'description'	=> 'The users email address',
									'is_admin'		=> 0,
								   ),
						'name'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'The users full name',
									'is_admin'		=> 0,
								   ),
					  );

	}//end constructor


}//end class
?>