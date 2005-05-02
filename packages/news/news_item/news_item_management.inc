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
* $Id: news_item_management.inc,v 1.2.2.2 2005/05/02 04:38:44 gsherwood Exp $
*
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

		$this->vars['contact_name']         = Array(
										'added'     => '0.2',
										'type'      => 'text',
										'default'   => '',
										);

		$this->vars['contact_phone']         = Array(
										'added'     => '0.2',
										'type'      => 'text',
										'default'   => '',
										);
										
		$this->vars['show_headline']	= Array(
										'added'     => '0.3',
										'type'      => 'boolean',
										'default'   => true,
										'parameters' => Array(
													 'allow_empty' => false
													 ),
										);
										
		$this->vars['show_summary']	= Array(
										'added'     => '0.3',
										'type'      => 'boolean',
										'default'   => true,
										'parameters' => Array(
													 'allow_empty' => false
													 ),
										);

		$this->vars['show_body']	= Array(
										'added'     => '0.3',
										'type'      => 'boolean',
										'default'   => true,
										'parameters' => Array(
													 'allow_empty' => false
													 ),
										);

		$this->vars['show_contact_name']	= Array(
										'added'     => '0.3',
										'type'      => 'boolean',
										'default'   => true,
										'parameters' => Array(
													 'allow_empty' => false
													 ),
										);

		$this->vars['show_contact_phone']	= Array(
										'added'     => '0.3',
										'type'      => 'boolean',
										'default'   => true,
										'parameters' => Array(
													 'allow_empty' => false
													 ),
										);


	}//end constructor


}//end class

?>
