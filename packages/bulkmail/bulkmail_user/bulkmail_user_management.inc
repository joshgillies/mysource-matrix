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
* $Id: bulkmail_user_management.inc,v 1.9 2012/08/30 00:57:13 ewang Exp $
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
* @version $Revision: 1.9 $
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