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
* $Id: page_remote_content_management.inc,v 1.10 2005/12/08 01:09:23 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Remote_Content_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.10 $
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
						'remote_url'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> 'http://matrix.squiz.net',
											   ),
						'load_links'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> true,
												'parameters'	=> Array('allow_empty'	=> false),
											   ),
						'only_local_links'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> false,
												'parameters'	=> Array('allow_empty'	=> false),
											   ),
						'allow_redirect'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> true,
												'parameters'	=> Array('allow_empty'	=> false),
											   ),
						'username'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'password'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'password',
												'default'	=> '',
											   ),
						'include_tags'		=> Array (
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> true,
												'parameters'	=> Array('allow_empty'	=> false),
											   ),
						'start_tag'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'end_tag'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'timeout'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> '30',
												'parameters'	=> Array('allow_negative' => false),
											   ),
						'error_message'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'wysiwyg',
												'default'	=> '<p>Content cannot be accessed</p>',
											   ),
						'display_error'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> true,
												'parameters'	=> Array('allow_empty'	=> false),
											   ),
						'binary_message'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'wysiwyg',
												'default'	=> "<p>
																The requested document appears to be a binary file.
																Please click the url to open it in another window.<br />
																<a href='%remote_url%' target='_blank'>%remote_url%</a>
																</p>",
											   ),
						'strip_images'		=> Array(
												'added'			=> '1.4',
												'type'			=> 'boolean',
												'default'		=> false,
												'parameters'	=> Array('allow_empty' => false),
											   ),
						'tunnel_images'		=> Array(
												'added'			=> '1.4',
												'type'			=> 'boolean',
												'default'		=> false,
												'parameters'	=> Array('allow_empty' => false),
											   ),
						'tunnel_media'		=> Array(
												'added'			=> '1.4',
												'type'			=> 'boolean',
												'default'		=> false,
												'parameters'	=> Array('allow_empty' => false),
											   ),
						'tunnel_scripts'	=> Array(
												'added'			=> '1.4',
												'type'			=> 'boolean',
												'default'		=> false,
												'parameters'	=> Array('allow_empty' => false),
											   ),
						'url_white_list'	=> Array(
												'added'		=> '1.5',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
					  );

	}//end constructor


}//end class

?>