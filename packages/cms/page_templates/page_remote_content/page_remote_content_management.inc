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
* $Id: page_remote_content_management.inc,v 1.16 2006/05/16 05:56:49 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Remote_Content_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.16 $
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
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty'	=> FALSE),
											   ),
						'only_local_links'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty'	=> FALSE),
											   ),
						'allow_redirect'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty'	=> FALSE),
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
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty'	=> FALSE),
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
						'multi_occurence'	=> Array (
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty'	=> FALSE),
											   ),
						'no_tag_clear'		=> Array(
												'added'			=> '1.6',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty'	=> FALSE),
											   ),
						'timeout'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> '30',
												'parameters'	=> Array('allow_negative' => FALSE),
											   ),
						'error_message'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'wysiwyg',
												'default'	=> '<p>Content cannot be accessed</p>',
											   ),
						'display_error'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty'	=> FALSE),
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
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty' => FALSE),
											   ),
						'tunnel_images'		=> Array(
												'added'			=> '1.4',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty' => FALSE),
											   ),
						'tunnel_media'		=> Array(
												'added'			=> '1.4',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty' => FALSE),
											   ),
						'tunnel_scripts'	=> Array(
												'added'			=> '1.4',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty' => FALSE),
											   ),
						'url_white_list'	=> Array(
												'added'		=> '1.5',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'post_variables'	=> Array(
												'added'		=> '1.5',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'tag_replacements'	=> Array(
												'added'		=> '1.6',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
					  );

	}//end constructor


}//end class

?>