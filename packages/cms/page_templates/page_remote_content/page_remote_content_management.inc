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
* $Id: page_remote_content_management.inc,v 1.2 2004/09/01 03:39:18 arailean Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Remote_Content_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Remote_Content_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Remote_Content_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'remote_url' => Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => 'http://matrix.squiz.net',
											),

						'load_links' => Array(
											'added'       => '0.1',
											'type'      => 'boolean',
											'default'   => true,
											),

						'allow_redirect' => Array(
											'added'       => '0.1',
											'type'      => 'boolean',
											'default'   => true,
											),

						'start_tag' => Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => '',
											),

						'end_tag' => Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => '',
											),

						'timeout' => Array(
											'added'     => '0.1',
											'type'      => 'int',
											'default'   => '30',
											'parameters'=> Array(
																'allow_negative' => false
																),
											),

						'error_message' => Array(
											'added'     => '0.1',
											'type'      => 'wysiwyg',
											'default'   => '<p>Content cannot be accessed</p>',
											),

						'display_error' => Array(
											'added'       => '0.1',
											'type'      => 'boolean',
											'default'   => true,
											),

						'binary_message' => Array(
											'added'     => '0.1',
											'type'      => 'wysiwyg',
											'default'   => "<p>
															The requested document appears to be a binary file. 
															Please click the url to open it in another window.<br />
															<a href='%remote_url%' target='_blank'>%remote_url%</a>
															</p>",

											),


					);

	}//end constructor


}//end class

?>