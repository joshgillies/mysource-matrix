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
* $Id: authentication_default_management.inc,v 1.5 2006/12/06 04:23:57 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Authentication_Default_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Authentication_Default_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Authentication_Default_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'name'	=> Array(
									'default'	=> 'Default Authentication',
								   ),
					  );

	}//end constructor


}//end class

?>