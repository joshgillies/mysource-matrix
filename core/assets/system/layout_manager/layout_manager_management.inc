<?php

/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |

* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: layout_manager_management.inc,v 1.3 2004/02/18 18:19:16 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Layout_Manager_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.3 $ - 0.1
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
				'layouts' => Array(
						'added'		=> '0.0.1',
						'type'		=> 'serialise',
						'default'	=> Array(),
					),
		);

	}//end constructor

}//end class
?>
