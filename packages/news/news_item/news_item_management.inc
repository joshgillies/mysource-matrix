<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: news_item_management.inc,v 1.13 2012/08/30 00:59:01 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* News_Item_Management
*
* @see Asset_Management
*
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class News_Item_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars['name'] = Array(
								'added'		=> '0.1',
								'type'		=> 'text',
								'default'	=> '',
								'is_admin'      => false,
								'is_contextable'	=> TRUE,
							  );
		$this->vars['link_url'] = Array(
									'added'		=> '0.1',
									'type'		=> 'text',
									'default'	=> '',
									'is_admin'      => false,
								  );
		$this->vars['url_suffix'] = Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'Extra text to send as part of the URL query string',
										'is_admin'      => false,
									);
		$this->vars['summary'] = Array(
									'added'			=> '0.1',
									'type'			=> 'wysiwyg',
									'default'		=> '',
									'description'	=> 'Short summary of the item',
									'is_admin'      => false,
									'is_contextable'	=> TRUE,
								 );
		$this->vars['body'] = Array(
								'added'		=> '0.1',
								'type'		=> 'wysiwyg',
								'default'	=> '',
								'is_admin'      => false,
								'is_contextable'	=> TRUE,
							  );

		$this->vars['contact_name'] = Array(
										'added'		=> '0.2',
										'type'		=> 'text',
										'default'	=> '',
										'is_admin'      => false,
									  );

		$this->vars['contact_phone'] = Array(
										'added'		=> '0.2',
										'type'		=> 'text',
										'default'	=> '',
										'is_admin'      => false,
									   );

		$this->vars['show_headline'] = Array(
										'added'			=> '0.3',
										'type'			=> 'boolean',
										'default'		=> true,
										'parameters'	=> Array(
															'allow_empty'	=> false,
														   ),
										'is_admin'      => false,
									   );

		$this->vars['show_summary'] = Array(
										'added'			=> '0.3',
										'type'			=> 'boolean',
										'default'		=> true,
										'parameters'	=> Array(
															'allow_empty'	=> false,
														   ),
										'is_admin'      => false,
									  );

		$this->vars['show_body'] = Array(
									'added'			=> '0.3',
									'type'			=> 'boolean',
									'default'		=> true,
									'parameters'	=> Array(
														'allow_empty'	=> false,
													   ),
									'is_admin'      => false,
								   );

		$this->vars['show_contact_name'] = Array(
											'added'			=> '0.3',
											'type'			=> 'boolean',
											'default'		=> true,
											'parameters'	=> Array(
																'allow_empty'	=> false,
															   ),
											'is_admin'      => false,
										   );

		$this->vars['show_contact_phone'] = Array(
												'added'			=> '0.3',
												'type'			=> 'boolean',
												'default'		=> true,
												'parameters'	=> Array(
																	'allow_empty'	=> false,
																   ),
												'is_admin'      => false,
											);

		$this->components['summary'] = Array (
										'description'		=> 'Summary',
										'default_weighting'	=> '1',
									   );
		$this->components['body'] = Array (
										'description'		=> 'Body',
										'default_weighting'	=> '1',
									);


	}//end constructor


}//end class

?>
