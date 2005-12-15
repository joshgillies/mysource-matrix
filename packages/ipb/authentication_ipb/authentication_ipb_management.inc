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
* $Id: authentication_ipb_management.inc,v 1.1.1.1 2005/12/15 03:02:07 skim Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Authentication_IPB_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1.1.1 $
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
											'is_admin'	=> false,
										   ),
						'allow_banned'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> false,
											'parameters'	=> Array(
																'allow_empty'	=> false,
															   ),
											'is_admin'		=> false,
										   ),
					  );

	}//end constructor


}//end class

?>