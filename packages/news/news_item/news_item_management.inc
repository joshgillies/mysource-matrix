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
* $Id: news_item_management.inc,v 1.1.1.1 2004/06/24 05:37:11 amiller Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* News_Item_Management
*
* @see Asset_Management
*
* @author  Avi Miller <amiller@squiz.net>
* @version $Version$ - 0.1
* @package MySource_Matrix_Packages
* @subpackage news
*/
class News_Item_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function News_Item_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['name']			= Array(
										'added'     => '0.1',
										'type'      => 'text',
										'default'   => '',
										);
		$this->vars['link_url']		= Array(
										'added'     => '0.1',
										'type'      => 'text',
										'default'   => '',
										);
		$this->vars['url_suffix']  = Array(
										'added'       => '0.1',
										'type'        => 'text',
										'default'     => '',
										'description' => 'Extra text to send as part of the URL query string',
										);
		$this->vars['summary'] 		= Array(
										'added'     => '0.1',
										'type'      => 'wysiwyg',
										'default'   => '',
										'description' => 'Short summary of the item',
										);
		$this->vars['body']			= Array(
										'added'     => '0.1',
										'type'      => 'wysiwyg',
										'default'   => '',
										);
	}//end constructor


}//end class

?>