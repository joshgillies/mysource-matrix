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
* $Id: page_rss_feed_management.inc,v 1.5 2012/08/30 00:59:01 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_RSS_Feed_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Page_RSS_Feed_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_RSS_Feed_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'rss_types'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'rss_1.0',
													'parameters'	=> Array(
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																		'options'		=> Array(
																							'rss_1.0'	=> 'RSS 1.0',
																							'rss_2.0'	=> 'RSS 2.0',
																							'atom_1.0'	=> 'Atom 1.0',
																						   ),
																	   ),
												   ),
						'num_assets_as_items'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'parameters'	=> Array(
																		'allow_negative'	=> FALSE,
																		'allow_empty'		=> FALSE,
																	   ),
												   ),
						'items'					=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'channel'				=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'image'					=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'http_auth'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'		=> FALSE,
																	   ),
												   ),
					  );

	}//end constructor


}//end class

?>
