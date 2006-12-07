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
* $Id: authentication_ipb_management.inc,v 1.3 2006/12/07 03:18:13 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Authentication_IPB_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Authentication_IPB_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Authentication_IPB_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'name'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'IPB Authentication',
											'is_admin'	=> FALSE,
										   ),
						'allow_banned'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
											'is_admin'		=> FALSE,
										   ),
					  );

	}//end constructor


}//end class

?>