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
* $Id: layout_manager_management.inc,v 1.6 2006/12/06 04:40:08 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Layout_Manager_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.6 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*/
	function Layout_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'layouts'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									   ),
					  );

	}//end constructor


}//end class
?>
