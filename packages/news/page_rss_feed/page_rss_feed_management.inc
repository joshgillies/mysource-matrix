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
* $Id: page_rss_feed_management.inc,v 1.2 2006/01/05 04:38:14 pgannavarapu Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_RSS_Feed_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.2 $
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
					  );

	}//end constructor


}//end class

?>
